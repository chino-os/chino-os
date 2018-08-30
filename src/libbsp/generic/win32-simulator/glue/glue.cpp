#include <kernel/kernel_iface.h>

extern "C"
{
	void __libc_init_array()
	{

	}

	void __libc_fini_array()
	{

	}

	void main()
	{
		Kernel_Main(nullptr);
		while (1);
	}
}
