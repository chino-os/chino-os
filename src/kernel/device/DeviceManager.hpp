//
// Kernel Device
//
#pragma once
#include "Driver.hpp"
#include "Device.hpp"
#include "../utils.hpp"
#include <vector>
#include <memory>

namespace Chino
{
	namespace Device
	{
		class DeviceMananger
		{
		public:
			DeviceMananger();

			void InstallDevices(const BootParameters& bootParams);
			void InstallDevice(Device& drive);
			void InstallDriver(Driver& driver);

			void DumpDevices();
		private:
		private:
			std::vector<ObjectPtr<Driver>> drivers_;
			std::vector<ObjectPtr<Device>> devices_;
		};
	}
}

extern StaticHolder<Chino::Device::DeviceMananger> g_DeviceMgr;
