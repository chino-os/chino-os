#include <kernel/kernel_iface.h>
#include <stdint.h>
#include <string.h>

#define ASSERT_IF_DPORT_REG(_r, OP)

//clear bit or clear bits of register
#define REG_CLR_BIT(_r, _b)  ({                                                                                        \
            ASSERT_IF_DPORT_REG((_r), REG_CLR_BIT);                                                                    \
            (*(volatile uint32_t*)(_r) &= ~(_b));                                                                      \
})

#define BIT(x) (1 << (x))
#define DR_REG_TIMERGROUP0_BASE 0x3ff5F000
#define DR_REG_RTCCNTL_BASE 0x3ff48000

#define REG_TIMG_BASE(i) (DR_REG_TIMERGROUP0_BASE + i*0x1000)
#define RTC_CNTL_WDTCONFIG0_REG (DR_REG_RTCCNTL_BASE + 0x8c)

#define TIMG_WDTCONFIG0_REG(i) (REG_TIMG_BASE(i) + 0x0048)

/* TIMG_WDT_FLASHBOOT_MOD_EN : R/W ;bitpos:[14] ;default: 1'h1 ; */
/*description: When set  flash boot protection is enabled*/
#define TIMG_WDT_FLASHBOOT_MOD_EN  (BIT(14))
#define TIMG_WDT_FLASHBOOT_MOD_EN_M  (BIT(14))
#define TIMG_WDT_FLASHBOOT_MOD_EN_V  0x1
#define TIMG_WDT_FLASHBOOT_MOD_EN_S 14

/* RTC_CNTL_WDT_FLASHBOOT_MOD_EN : R/W ;bitpos:[10] ;default: 1'h1 ; */
/*description: enable WDT in flash boot*/
#define RTC_CNTL_WDT_FLASHBOOT_MOD_EN  (BIT(10))
#define RTC_CNTL_WDT_FLASHBOOT_MOD_EN_M  (BIT(10))
#define RTC_CNTL_WDT_FLASHBOOT_MOD_EN_V  0x1
#define RTC_CNTL_WDT_FLASHBOOT_MOD_EN_S 10

/* RTC_CNTL_WDT_EN : R/W ;bitpos:[31] ;default: 1'h0 ; */
/*description: enable RTC WDT*/
#define RTC_CNTL_WDT_EN  (BIT(31))
#define RTC_CNTL_WDT_EN_M  (BIT(31))
#define RTC_CNTL_WDT_EN_V  0x1
#define RTC_CNTL_WDT_EN_S 31

void uartAttach(void);
int uart_tx_one_char(uint8_t TxChar);

extern int _bss_start;
extern int _bss_end;
extern int _data_start;
extern int _data_end;

void BSPKernelEntry()
{
	//Clear bss
	memset(&_bss_start, 0, (&_bss_end - &_bss_start) * sizeof(_bss_start));

	REG_CLR_BIT(RTC_CNTL_WDTCONFIG0_REG, RTC_CNTL_WDT_FLASHBOOT_MOD_EN);
	REG_CLR_BIT(RTC_CNTL_WDTCONFIG0_REG, RTC_CNTL_WDT_EN);
	REG_CLR_BIT(TIMG_WDTCONFIG0_REG(0), TIMG_WDT_FLASHBOOT_MOD_EN);

	uartAttach();
	Kernel_Main(NULL);
}
