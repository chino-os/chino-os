//
// Chino Board
//
#pragma once
#include <kernel/kernel_iface.h>
#include <kernel/device/Driver.hpp>
#include <memory>

namespace Chino
{
	namespace Memory
	{
		struct HeapRegionDesc
		{
			uintptr_t StartAddress;
			size_t SizeInBytes;
		};

		bool BSPEnumHeapRegion(const BootParameters& bootParams, size_t index, HeapRegionDesc& desc);
	}

	namespace Device
	{
		ObjectPtr<Driver> BSPInstallRootDriver(const BootParameters& bootParams);
	}

	namespace Diagnostic
	{
		void BSPInitializeDebug(const BootParameters& bootParams);
		void BSPDebugPutChar(wchar_t chr);
		void BSPDebugBlueScreen();
		void BSPDebugClearScreen();
	}

	namespace Thread
	{
		void BSPSetupSchedulerTimer();
		void BSPSleepMs(uint32_t ms);
	}
}
