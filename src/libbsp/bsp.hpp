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
		std::unique_ptr<Driver> BSPInstallRootDriver(const BootParameters& bootParams);
	}

	namespace Diagnostic
	{
		void BSPInitializeDebug(const BootParameters& bootParams);
		void BSPDebugPutChar(wchar_t chr);
		void BSPDebugBlueScreen();
		void BSPDebugClearScreen();
	}
}
