//
// Kernel Device
//
#include "Dmac.hpp"
#include <libbsp/bsp.hpp>
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <kernel/device/controller/Pic.hpp>
#include <kernel/threading/ThreadSynchronizer.hpp>
#include "Rcc.hpp"
#include <string>
#include <io_helper.hpp>
#include <kernel/memory/MemoryManager.hpp>

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Threading;

DEFINE_FDT_DRIVER_DESC_2(DmacDriver, "dmac", "st,stm32f103-dmac1", "st,stm32f103-dmac2");

struct dmac_isr
{
	uint32_t Value;

	bool Any(size_t channel) volatile
	{
		return GetBit(&Value, channel * 4);
	}

	bool IsCompleted(size_t channel) volatile
	{
		return GetBit(&Value, channel * 4 + 1);
	}

	bool IsHalfTransferred(size_t channel) volatile
	{
		return GetBit(&Value, channel * 4 + 2);
	}

	bool HasError(size_t channel) volatile
	{
		return GetBit(&Value, channel * 4 + 3);
	}
};

struct dmac_ifcr
{
	uint32_t Value;

	void ClearAll(size_t channel) volatile
	{
		SetBit(&Value, channel * 4, true);
	}

	void ClearCompleted(size_t channel) volatile
	{
		SetBit(&Value, channel * 4 + 1, true);
	}

	void ClearHalfTransferred(size_t channel) volatile
	{
		SetBit(&Value, channel * 4 + 2, true);
	}

	void ClearError(size_t channel) volatile
	{
		SetBit(&Value, channel * 4 + 3, true);
	}
};

enum dmac_dir
{
	DMAC_DIR_FromPeriph = 0,	//!< Read from peripheral
	DMAC_DIR_FromMem = 1		//!< Read from memory
};

enum dmac_width
{
	DMAC_WIDTH_8 = 0b00,		//!< 8-bits
	DMAC_WIDTH_16 = 0b01,		//!< 16-bits
	DMAC_WIDTH_32 = 0b10		//!< 32-bits
};

enum dmac_priority
{
	DMAC_PRIO_Low = 0b00,		//!< Low
	DMAC_PRIO_Medium = 0b01,	//!< Medium
	DMAC_PRIO_High = 0b10,		//!< High
	DMAC_PRIO_VeryHigh = 0b11	//!< Very high
};

union dmac_ccr
{
	struct
	{
		uint32_t EN : 1;		//!< Channel enable
		uint32_t TCIE : 1;		//!< Transfer complete interrupt enable
		uint32_t HTIE : 1;		//!< Half transfer interrupt enable
		uint32_t TEIE : 1;		//!< Transfer error interrupt enable
		dmac_dir DIR : 1;		//!< Data transfer direction
		uint32_t CIRC : 1;		//!< Circular mode
		uint32_t PINC : 1;		//!< Peripheral increment mode
		uint32_t MINC : 1;		//!< Memory increment mode
		dmac_width PSIZE : 2;	//!< Peripheral size
		dmac_width MSIZE : 2;	//!< Memory size
		dmac_priority PL : 2;	//!< Channel priority level
		uint32_t MEM2MEM : 1;	//!< Memory to memory mode
		uint32_t RESV0 : 17;
	};

	uint32_t Value;
};

union dmac_cndtr
{
	struct
	{
		uint32_t NDT : 16;		//!< Number of data to transfer
		uint32_t RESV0 : 16;
	};

	uint32_t Value;
};

typedef volatile struct
{
	dmac_isr ISR;
	dmac_ifcr IFCR;

	struct
	{
		dmac_ccr CCR;
		dmac_cndtr CNDTR;
		uint32_t CPAR;
		uint32_t CMAR;
		uint32_t RESV0;
	} Channel[7];
} DMAC_TypeDef;

struct DmaTransferOptions
{
	DmaTransmition Type;
	uintptr_t SourceAddress;
	size_t SourceByteWidth;
	bool SourceInc;
	uintptr_t DestAddress;
	size_t DestByteWidth;
	bool DestInc;
	size_t Count;
};

class Stm32DmaControllerBase : public DmaController, public FreeObjectAccess
{
public:
	Stm32DmaControllerBase(const FDTDevice& fdt)
		:fdt_(fdt)
	{
		auto regProp = fdt.GetProperty("reg");
		kassert(regProp.has_value());
		dmac_ = reinterpret_cast<decltype(dmac_)>(regProp->GetUInt32(0));
		g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
		periph_ = static_cast<RccPeriph>(size_t(RccPeriph::DMAC1) + fdt.GetName().back() - '1');
	}

	void Configure(size_t channelId, const DmaTransferOptions& options)
	{
		auto& channel = dmac_->Channel[channelId];
		kassert(!channel.CCR.EN);
		kassert(options.Count <= 65535);

		dmac_ccr ccr{ 0 };
		if (options.Type == DmaTransmition::Perip2Mem)
		{
			ccr.DIR = DMAC_DIR_FromPeriph;
			ccr.PINC = options.SourceInc ? 1 : 0;
			ccr.MINC = options.DestInc ? 1 : 0;
			ccr.PSIZE = GetWidth(options.SourceByteWidth);
			ccr.MSIZE = GetWidth(options.DestByteWidth);
			channel.CPAR = options.SourceAddress;
			channel.CMAR = options.DestAddress;
		}
		else if (options.Type == DmaTransmition::Mem2Periph)
		{
			ccr.DIR = DMAC_DIR_FromMem;
			ccr.MINC = options.SourceInc ? 1 : 0;
			ccr.PINC = options.DestInc ? 1 : 0;
			ccr.MSIZE = GetWidth(options.SourceByteWidth);
			ccr.PSIZE = GetWidth(options.DestByteWidth);
			channel.CMAR = options.SourceAddress;
			channel.CPAR = options.DestAddress;
		}
		else
		{
			ccr.DIR = DMAC_DIR_FromPeriph;
			ccr.PINC = options.SourceInc ? 1 : 0;
			ccr.MINC = options.DestInc ? 1 : 0;
			ccr.PSIZE = GetWidth(options.SourceByteWidth);
			ccr.MSIZE = GetWidth(options.DestByteWidth);
			ccr.MEM2MEM = 1;
			channel.CPAR = options.SourceAddress;
			channel.CMAR = options.DestAddress;
		}

		ccr.TCIE = 1;
		ccr.TEIE = 1;
		channel.CCR.Value = ccr.Value;
		channel.CNDTR.NDT = options.Count;

		//g_Logger->PutFormat("ccr: %x, cndtr: %z\n", channel.CCR.Value, channel.CNDTR.NDT);
	}

	void StartAsync(size_t channelId)
	{
		kassert(!dmac_->Channel[channelId].CCR.EN);
		dmac_->Channel[channelId].CCR.EN = 1;
	}

	virtual void Register3And4ChannelISR(size_t channelId, std::function<void()> handler)
	{

	}

	virtual void Unregister3And4ChannelISR(size_t channelId)
	{

	}
private:
	static dmac_width GetWidth(size_t bitwidth)
	{
		switch (bitwidth)
		{
		case 1:
			return DMAC_WIDTH_8;
		case 2:
			return DMAC_WIDTH_16;
		case 4:
			return DMAC_WIDTH_32;
		default:
			throw std::invalid_argument("Invalid bitwidth.");
		}
	}
protected:
	friend class Stm32DmaChannel;

	DMAC_TypeDef * dmac_;
	const FDTDevice& fdt_;
	RccPeriph periph_;
};

class Stm32DmaChannel : public DmaChannel
{
	struct Session
	{
		DmaTransferOptions Options;
		ObjectPtr<AsyncActionCompletionEvent> CompletionEvent;
		Chino::Device::details::BufferListSelect<const volatile uint8_t> RestSource;
		Chino::Device::details::BufferListSelect<volatile uint8_t> RestDest;
		size_t SourceByteWidth, DestByteWidth;
	};
public:
	Stm32DmaChannel(size_t dmacId, size_t channelId, size_t irq, ObjectAccessor<Stm32DmaControllerBase>&& dmac, Locker<Mutex>&& locker)
		:dmacId_(dmacId), channelId_(channelId), irq_(irq), dmac_(std::move(dmac)), locker_(std::move(locker))
	{
		if (dmacId == 2 && (channelId == 3 || channelId == 4))
		{
			dmac->Register3And4ChannelISR(channelId, std::bind(&Stm32DmaChannel::OnIRQ, this));
		}
		else
		{
			auto nvic = g_ObjectMgr->GetDirectory(WKD_Device).Open("nvic1", OA_Read | OA_Write).MoveAs<PicDevice>();
			isr_ = g_DeviceMgr->InstallIRQHandler(nvic->GetId(), irq, std::bind(&Stm32DmaChannel::OnIRQ, this));
			nvic->SetIRQEnabled(irq, true);
		}
	}

	~Stm32DmaChannel()
	{
		if (dmacId_ == 2 && (channelId_ == 3 || channelId_ == 4))
		{
			dmac_->Unregister3And4ChannelISR(channelId_);
		}
		else
		{
			auto nvic = g_ObjectMgr->GetDirectory(WKD_Device).Open("nvic1", OA_Read | OA_Write).MoveAs<PicDevice>();
			nvic->SetIRQEnabled(irq_, false);
		}
	}

	virtual void ConfigureImpl(DmaTransmition type, BufferList<const volatile uint8_t> source, BufferList<volatile uint8_t> dest, size_t sourceByteWidth, size_t destByteWidth) override
	{
		auto srcSize = source.GetTotalSize() / sourceByteWidth;
		auto destSize = dest.GetTotalSize() / destByteWidth;
		kassert(srcSize && destSize);
		if (srcSize != 1 && destSize != 1)
		{
			if (srcSize != destSize)
				throw std::invalid_argument("Transmition count must be equal or 1.");
		}

		if (currentSession_)
			throw std::runtime_error("A session is already running.");

		DmaTransferOptions options;
		options.Type = type;
		options.SourceInc = srcSize != 1;
		options.DestInc = destSize != 1;
		options.SourceByteWidth = sourceByteWidth;
		options.DestByteWidth = destByteWidth;

		auto event = MakeObject<AsyncActionCompletionEvent>();
		currentSession_ = { options, event, source.Select(), dest.Select(), sourceByteWidth, destByteWidth };
		StartNextTransmition();
	}

	virtual ObjectPtr<IAsyncAction> StartAsync()
	{
		auto event = currentSession_->CompletionEvent;
		dmac_->StartAsync(channelId_);
		return event;
	}
private:
	void OnIRQ()
	{
		auto& dmac = dmac_->dmac_;
		auto channelId = channelId_;
		bool disable = false;
		if (dmac->ISR.Any(channelId))
		{
			if (dmac->ISR.IsCompleted(channelId))
			{
				if (currentSession_->RestSource.IsEmpty() || currentSession_->RestDest.IsEmpty())
				{
					currentSession_->CompletionEvent->SetResult();
					disable = true;
				}
				else
				{
					dmac->Channel[channelId].CCR.EN = 0;
					StartNextTransmition();
					dmac_->StartAsync(channelId_);
				}
			}
			else if (dmac->ISR.HasError(channelId))
			{
				currentSession_->CompletionEvent->SetException(std::make_exception_ptr(std::runtime_error("DMA transfer error.")));
				disable = true;
			}

			dmac->IFCR.ClearAll(channelId);

			if (disable)
			{
				dmac->Channel[channelId].CCR.EN = 0;
				currentSession_.reset();
			}
		}
	}

	void StartNextTransmition()
	{
		auto& session = *currentSession_;
		auto& options = session.Options;

		auto srcInc = options.SourceInc;
		auto destInc = options.DestInc;

		gsl::span<const volatile uint8_t> src;
		gsl::span<volatile uint8_t> dest;
		if (srcInc && !destInc)
		{
			src = session.RestSource.Pop();
			dest = session.RestDest.First();
			options.Count = src.size() / session.SourceByteWidth;
		}
		else if (!srcInc && destInc)
		{
			src = session.RestSource.First();
			dest = session.RestDest.Pop();
			options.Count = dest.size() / session.DestByteWidth;
		}
		else if (srcInc && destInc)
		{
			src = session.RestSource.First();
			dest = session.RestDest.First();
			options.Count = std::min(src.size() / session.SourceByteWidth, dest.size() / session.DestByteWidth);
			src = src.subspan(0, options.Count * session.SourceByteWidth);
			dest = dest.subspan(0, options.Count * session.DestByteWidth);

			session.RestSource = session.RestSource.Skip(src.size());
			session.RestDest = session.RestDest.Skip(src.size());
		}
		else
		{
			if (session.RestSource.Count() / session.SourceByteWidth != 1 || session.RestDest.Count() / session.DestByteWidth != 1)
				throw std::runtime_error("Not supported.");

			src = session.RestSource.First();
			dest = session.RestDest.First();
			options.Count = 1;
			src = src.subspan(0, session.SourceByteWidth);
			dest = dest.subspan(0, session.DestByteWidth);

			session.RestSource = {};
			session.RestDest = {};
		}

		kassert(options.Count);
		options.SourceAddress = uintptr_t(src.data());
		options.DestAddress = uintptr_t(dest.data());

		dmac_->Configure(channelId_, session.Options);
	}
private:
	const size_t dmacId_;
	const size_t irq_;
	const size_t channelId_;
	ObjectAccessor<Stm32DmaControllerBase> dmac_;
	Locker<Mutex> locker_;
	ObjectPtr<IObject> isr_;
	std::optional<Session> currentSession_;
};

template<size_t N>
struct Stm32DmacISR
{
};

template<>
struct Stm32DmacISR<2>
{
	std::array<std::function<void()>, 2> isr_;
	ObjectPtr<IObject> dmacIsr_;
};

template<size_t N>
class Stm32DmaController : public Stm32DmaControllerBase
{
	static constexpr size_t ChannelsCount = N == 1 ? 7 : 5;
public:
	using Stm32DmaControllerBase::Stm32DmaControllerBase;

	virtual ObjectPtr<DmaChannel> OpenChannel(DmaRequestLine requestLine) override
	{
		size_t channelId;

		if constexpr (N == 1)
		{
			switch (requestLine)
			{
			case DmaRequestLine::I2C1_TX:
				channelId = 5;
				break;
			case DmaRequestLine::I2C1_RX:
				channelId = 6;
				break;
			default:
				throw std::invalid_argument("Invalid peripheral.");
			}
		}
		else
		{
			throw std::runtime_error("Not impl.");
		}

		auto access = OA_Read | OA_Write;
		auto irq = fdt_.GetProperty("irq")->GetUInt32(channelId);
		return MakeObject<Stm32DmaChannel>(N, channelId, irq, MakeAccessor<Stm32DmaControllerBase>(this, access), channelsMutex_[channelId]);
	}

	virtual void Register3And4ChannelISR(size_t channelId, std::function<void()> handler) override
	{
		if constexpr (N == 2)
		{
			channel3and4isr_.isr_[channelId == 3 ? 0 : 1] = std::move(handler);
		}
	}

	virtual void Unregister3And4ChannelISR(size_t channelId) override
	{
		if constexpr (N == 2)
		{
			channel3and4isr_.isr_[channelId == 3 ? 0 : 1] = nullptr;
		}
	}
protected:
	virtual void OnFirstOpen() override
	{
		for (auto& mutex : channelsMutex_)
			mutex = MakeObject<Mutex>();

		if constexpr (N == 2)
		{
			auto nvic = g_ObjectMgr->GetDirectory(WKD_Device).Open("nvic1", OA_Read | OA_Write).MoveAs<PicDevice>();
			auto irq = fdt_.GetProperty("irq")->GetUInt32(3);
			channel3and4isr_.dmacIsr_ = g_DeviceMgr->InstallIRQHandler(nvic->GetId(), irq, std::bind(&Stm32DmaController::OnChannel3And4ISR, this));
			nvic->SetIRQEnabled(irq, true);

		}

		RccDevice::Rcc1SetPeriphClockIsEnabled(periph_, true);
	}

	virtual void OnLastClose() override
	{
		RccDevice::Rcc1SetPeriphClockIsEnabled(periph_, false);

		for (auto& mutex : channelsMutex_)
			mutex.Reset();
	}
private:
	void OnChannel3And4ISR()
	{
		if constexpr (N == 2)
		{
			for (auto& isr : channel3and4isr_.isr_)
			{
				if (isr)
					isr();
			}
		}
	}
private:
	std::array<ObjectPtr<Mutex>, ChannelsCount> channelsMutex_;
	Stm32DmacISR<N> channel3and4isr_;
};

DmacDriver::DmacDriver(const FDTDevice& device)
	:device_(device)
{

}

void DmacDriver::Install()
{
	if (device_.HasCompatible("st,stm32f103-dmac1"))
		g_DeviceMgr->InstallDevice(MakeObject<Stm32DmaController<1>>(device_));
	else if (device_.HasCompatible("st,stm32f103-dmac2"))
		g_DeviceMgr->InstallDevice(MakeObject<Stm32DmaController<2>>(device_));
}
