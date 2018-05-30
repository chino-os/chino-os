//
// Kernel Device
//
#include "Fdt.hpp"
#include "../io/Usart.hpp"
#include "../controller/Rcc.hpp"

using namespace Chino::Device;

#define REF_FDT_DRIVER_DESC(Type) &Type::Descriptor

const FDTDriverDescriptor* Chino::Device::g_FDTDrivers[] =
{
	REF_FDT_DRIVER_DESC(UsartDriver),
	REF_FDT_DRIVER_DESC(RccDriver),
	nullptr
};
