//
// Chino Board
//
#pragma once
#include <kernel/kernel_iface.h>
#include <kernel/device/Driver.hpp>
#include <memory>

namespace Chino
{
	namespace Device
	{
		extern std::unique_ptr<Driver> InstallRootDriver(const BootParameters& bootParams);
	}
}
