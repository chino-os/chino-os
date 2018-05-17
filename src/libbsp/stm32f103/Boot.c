#include <kernel/kernel_iface.h>

static void init_lma(void)
{
	extern unsigned int _data_lma;
	extern unsigned int _data;
	extern unsigned int _edata;
	unsigned int *src, *dst;

	src = &_data_lma;
	dst = &_data;
	while (dst < &_edata)
		*dst++ = *src++;
}

static void init_bss(void)
{
	extern unsigned int _bss;
	extern unsigned int _ebss;
	unsigned int *dst;

	dst = &_bss;
	while (dst < &_ebss)
		*dst++ = 0;
}

void BSPInitAndCallKerneyEntry()
{
	/* Copy lma data to memory */
	init_lma();
	/* Initialize bss data to 0 */
	init_bss();
	//Kernel_Main(NULL);
	while (1);
}
