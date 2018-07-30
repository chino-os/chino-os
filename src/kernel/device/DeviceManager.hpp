//
// Kernel Device
//
#pragma once
#include "Driver.hpp"
#include "Device.hpp"
#include "../utils.hpp"
#include <vector>
#include <memory>
#include <unordered_map>

namespace Chino
{
	namespace Device
	{
		class PicDevice;

		class DeviceMananger
		{
			class IRQEntry : public Object
			{
			public:
				IRQEntry(size_t picId, int32_t irq, std::function<void()>&& handler);
				~IRQEntry();

				void Invoke();
			private:
				size_t picId_;
				int32_t irq_;
				std::function<void()> handler_;
			};
		public:
			DeviceMananger();

			void InstallDevices(const BootParameters& bootParams);
			void InstallDevice(ObjectPtr<Device> drive);
			void InstallDriver(ObjectPtr<Driver> driver);
			ObjectPtr<IObject> InstallIRQHandler(size_t picId, int32_t irq, std::function<void()> handler);

			void DumpDevices();

			void OnIRQ(size_t picId, int32_t irq);
		private:
			void UninstallIRQHandler(size_t picId, int32_t irq);
		private:
			std::vector<ObjectPtr<Driver>> drivers_;
			std::vector<ObjectPtr<Device>> devices_;
			std::unordered_map<size_t, std::unordered_map<int32_t, ObjectPtr<IRQEntry>>> irqHandlers_;
		};
	}
}

extern StaticHolder<Chino::Device::DeviceMananger> g_DeviceMgr;
