//
// Kernel Device
//
#pragma once
#include "Driver.hpp"
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
			void InstallDriver(std::unique_ptr<Driver>&& driver);
		private:
			std::vector<std::unique_ptr<Driver>> drivers_;
		};
	}
}

extern StaticHolder<Chino::Device::DeviceMananger> g_DeviceMgr;
