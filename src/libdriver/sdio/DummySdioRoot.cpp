//
// Kernel Device
//
#include <libbsp/bsp.hpp>
#include <kernel/kdebug.hpp>
#include <kernel/device/io/Sdio.hpp>

using namespace Chino;
using namespace Chino::Device;

Chino::ObjectPtr<Driver> Chino::Device::BSPInstallSdioRootDriver(ObjectPtr<SdioController> sdio)
{
	return nullptr;
}
