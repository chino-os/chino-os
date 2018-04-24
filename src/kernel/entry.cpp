//
// Kernel Entry
//
#include "kernel_iface.hpp"

volatile int s[256];
int a[256] = { 0 };

extern "C" void kernel_entry(const BootParameters& params)
{
	s[1] = 0;
	a[1] = 1;
}