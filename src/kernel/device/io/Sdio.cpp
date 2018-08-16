//
// Kernel Device
//
#include "Sdio.hpp"
#include <libbsp/bsp.hpp>

using namespace Chino::Device;

Chino::ObjectPtr<Driver> SdioController::TryLoadDriver()
{
	return BSPInstallSdioRootDriver(this);
}
