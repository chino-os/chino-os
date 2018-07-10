//
// Chino Thread
//
#include "../bsp_defines.hpp"
#include <libbsp/bsp.hpp>
#include <libarch/arch.h>
#include <climits>

using namespace Chino::Threading;

extern "C"
{
	extern void ArchTimerHandler();
	extern void ArchAPICSpuriousHandler();
}

#pragma pack(push, 1)

struct IDTPointer
{
	uint16_t TableLimit;
	uint64_t TableBase;                /* The address of the first entry in xInterruptDescriptorTable. */
};

struct IDTEntry
{
	uint16_t ISRLow;				/* Low 16 bits of handler address. */
	uint16_t SegmentSelector;		/* Flat model means this is not changed. */
	uint8_t Zero1;					/* Must be set to zero. */
	uint8_t Flags;					/* Flags for this entry. */
	uint16_t ISRMiddle;				/* Middle 16 bits of handler address. */
	uint32_t ISRHigh;				/* High 32 bits of handler address. */
	uint32_t Zero2;					/* Must be set to zero. */
};

struct GDTPointer
{
	uint16_t TableLimit;
	uint64_t TableBase;
};

typedef void(*ISR_Handler_t)();

#define configAPIC_BASE	0xFEE00000UL

/* The interrupt priority (for vectors 16 to 255) is determined using vector/16.
The quotient is rounded to the nearest integer with 1 being the lowest priority
and 15 is the highest.  Therefore the following two interrupts are at the lowest
priority.  *NOTE 1* If the yield vector is changed then it must also be changed
in the portYIELD_INTERRUPT definition immediately below. */
#define portAPIC_TIMER_INT_VECTOR 		( 0x21 )
#define portAPIC_YIELD_INT_VECTOR 		( 0x20 )

/* Build yield interrupt instruction. */
#define portYIELD_INTERRUPT "int $0x20"

/* APIC register addresses. */
#define portAPIC_EOI					( *( ( volatile uint32_t * ) 0xFEE000B0UL ) )

/* APIC bit definitions. */
#define portAPIC_ENABLE_BIT				( 1UL << 8UL )
#define portAPIC_TIMER_PERIODIC 		( 1UL << 17UL )
#define portAPIC_DISABLE 				( 1UL << 16UL )
#define portAPIC_NMI 					( 4 << 8)
#define portAPIC_DIV_16 				( 0x03 )

/* Define local API register addresses. */
#define portAPIC_ID_REGISTER			( *( ( volatile uint32_t * ) ( configAPIC_BASE + 0x20UL  ) ) )
#define portAPIC_SPURIOUS_INT			( *( ( volatile uint32_t * ) ( configAPIC_BASE + 0xF0UL  ) ) )
#define portAPIC_LVT_TIMER				( *( ( volatile uint32_t * ) ( configAPIC_BASE + 0x320UL ) ) )
#define portAPIC_TIMER_INITIAL_COUNT	( *( ( volatile uint32_t * ) ( configAPIC_BASE + 0x380UL ) ) )
#define portAPIC_TIMER_CURRENT_COUNT	( *( ( volatile uint32_t * ) ( configAPIC_BASE + 0x390UL ) ) )
#define portAPIC_TASK_PRIORITY			( *( ( volatile uint32_t * ) ( configAPIC_BASE + 0x80UL  ) ) )
#define portAPIC_LVT_ERROR				( *( ( volatile uint32_t * ) ( configAPIC_BASE + 0x370UL ) ) )
#define portAPIC_ERROR_STATUS			( *( ( volatile uint32_t * ) ( configAPIC_BASE + 0x280UL ) ) )
#define portAPIC_LDR	 				( *( ( volatile uint32_t * ) ( configAPIC_BASE + 0xD0UL  ) ) )
#define portAPIC_TMRDIV 				( *( ( volatile uint32_t * ) ( configAPIC_BASE + 0x3E0UL ) ) )
#define portAPIC_LVT_PERF 				( *( ( volatile uint32_t * ) ( configAPIC_BASE + 0x340UL ) ) )
#define portAPIC_LVT_LINT0 				( *( ( volatile uint32_t * ) ( configAPIC_BASE + 0x350UL ) ) )
#define portAPIC_LVT_LINT1 				( *( ( volatile uint32_t * ) ( configAPIC_BASE + 0x360UL ) ) )

/* Default flags setting for entries in the IDT. */
#define portIDT_FLAGS					( 0x8E )
#define portAPIC_SPURIOUS_INT_VECTOR 	( 0xff )

#define portNUM_VECTORS		256

#define configCPU_CLOCK_HZ 1000000000
#define configTICK_RATE_HZ 10

#pragma pack(pop)

alignas(32) static IDTEntry interruptDescriptorTable_[portNUM_VECTORS];
alignas(32) static uint64_t gdtTable_[3];

static void prvSetInterruptGate(uint8_t number, ISR_Handler_t handler, uint8_t flags);
static void SetupGDT();
static void SetupIDT();

void Chino::Threading::BSPSetupSchedulerTimer()
{
	//SetupGDT();
	SetupIDT();

	portAPIC_LVT_TIMER = portAPIC_DISABLE;

	/* Install APIC timer ISR vector. */
	prvSetInterruptGate((uint8_t)portAPIC_TIMER_INT_VECTOR, ArchTimerHandler, portIDT_FLAGS);

	/* Install spurious interrupt vector. */
	prvSetInterruptGate((uint8_t)portAPIC_SPURIOUS_INT_VECTOR, ArchAPICSpuriousHandler, portIDT_FLAGS);

	/* Set the interrupt frequency. */
	portAPIC_TMRDIV = portAPIC_DIV_16;
	portAPIC_TIMER_INITIAL_COUNT = ((configCPU_CLOCK_HZ >> 4UL) / configTICK_RATE_HZ) - 1UL;

	/* Enable LAPIC Counter.*/
	portAPIC_LVT_TIMER = portAPIC_TIMER_PERIODIC | portAPIC_TIMER_INT_VECTOR;

	/* Enable the APIC, mapping the spurious interrupt at the same time. */
	portAPIC_SPURIOUS_INT = portAPIC_SPURIOUS_INT_VECTOR | portAPIC_ENABLE_BIT;

	ArchEnableInterrupt();
}

void Chino::Threading::BSPSleepMs(uint32_t ms)
{
	auto count = configCPU_CLOCK_HZ / 1000 * ms;
	volatile int a;
	for (size_t i = 0; i < count; i++) a++;
}

void Chino::Threading::BSPYield()
{
	__asm volatile(portYIELD_INTERRUPT);
}

static void prvSetInterruptGate(uint8_t number, ISR_Handler_t handler, uint8_t flags)
{
	uint16_t codeSegment;
	auto base = uintptr_t(handler);

	interruptDescriptorTable_[number].ISRLow = (uint16_t)(base & USHRT_MAX);
	interruptDescriptorTable_[number].ISRMiddle = (uint16_t)((base >> 16ul) & USHRT_MAX);
	interruptDescriptorTable_[number].ISRHigh = (uint32_t)((base >> 32ul) & UINT_MAX);

	/* When the flat model is used the CS will never change. */
	__asm volatile("mov %%cs, %0" : "=r" (codeSegment));
	interruptDescriptorTable_[number].SegmentSelector = codeSegment;
	interruptDescriptorTable_[number].Zero1 = 0;
	interruptDescriptorTable_[number].Flags = flags;
	interruptDescriptorTable_[number].Zero2 = 0;
}

struct GDT
{
	uint32_t base;
	uint32_t limit;
	uint32_t type;
};

#include <kernel/kdebug.hpp>

void encodeGdtEntry(uint64_t *tgdt, struct GDT source)
{
	auto target = reinterpret_cast<uint8_t*>(tgdt);
	// Check the limit to make sure that it can be encoded
	if ((source.limit > 65536) && (source.limit & 0xFFF) != 0xFFF) {
		kassert(!"You can't do that!");
	}
	if (source.limit > 65536) {
		// Adjust granularity if required
		source.limit = source.limit >> 12;
		target[6] = 0xC0;
	}
	else {
		target[6] = 0x40;
	}

	// Encode the limit
	target[0] = source.limit & 0xFF;
	target[1] = (source.limit >> 8) & 0xFF;
	target[6] |= (source.limit >> 16) & 0xF;

	// Encode the base 
	target[2] = source.base & 0xFF;
	target[3] = (source.base >> 8) & 0xFF;
	target[4] = (source.base >> 16) & 0xFF;
	target[7] = (source.base >> 24) & 0xFF;

	// And... Type
	target[5] = source.type;
}

static void SetupGDT()
{
	GDTPointer gdt;
	gdt.TableBase = uintptr_t(gdtTable_);
	gdt.TableLimit = sizeof(gdtTable_) - 1;

	encodeGdtEntry(gdtTable_, { .base = 0,.limit = 0,.type = 0 });
	encodeGdtEntry(gdtTable_ + 1, { .base = 0,.limit = 0xffffffff,.type = 0x9A });
	encodeGdtEntry(gdtTable_ + 2, { .base = 0,.limit = 0xffffffff,.type = 0x92 });

	/* Set GDT in CPU. */
	__asm volatile("lgdt %0" :: "m" (gdt));
}

static void SetupIDT()
{
	IDTPointer idt;
	/* Set IDT address. */
	idt.TableBase = uintptr_t(interruptDescriptorTable_);
	idt.TableLimit = sizeof(interruptDescriptorTable_) - 1;

	/* Set IDT in CPU. */
	__asm volatile("lidt %0" :: "m" (idt));
}
