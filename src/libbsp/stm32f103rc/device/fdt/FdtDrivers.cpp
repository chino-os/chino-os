//
// Kernel Device
//
#include "Fdt.hpp"
#include "../serial/Usart.hpp"

using namespace Chino::Device;

#define REF_FDT_DRIVER_DESC(Type) &Type::Descriptor

const FDTDriverDescriptor* Chino::Device::g_FDTDrivers[] =
{
	REF_FDT_DRIVER_DESC(UsartDriver),
	nullptr
};