//
// Kernel Device
//
#pragma once
#include "Driver.hpp"
#include "storage/Drive.hpp"
#include "storage/filesystem/FileSystem.hpp"
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

			void RegisterDrive(DriveDevice& drive);
			void DumpDevices();

			DriveDevice& GetDrive(size_t index) const;
		private:
		private:
			std::vector<std::unique_ptr<Driver>> drivers_;
			std::vector<std::reference_wrapper<DriveDevice>> drives_;
		};
	}
}

extern StaticHolder<Chino::Device::DeviceMananger> g_DeviceMgr;
