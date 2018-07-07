//
// Kernel Device
//
#pragma once

#include "../fdt/Fdt.hpp"
#include <noncopyable.hpp>
#include <kernel/object/Directory.hpp>

namespace Chino
{
	namespace Device
	{
		enum class PortPins
		{
			Pin0,
			Pin1,
			Pin2,
			Pin3,
			Pin4,
			Pin5,
			Pin6,
			Pin7,
			Pin8,
			Pin9,
			Pin10,
			Pin11,
			Pin12,
			Pin13,
			Pin14,
			Pin15
		};

		enum class PortInputMode
		{
			Analog = 0,
			Floating = 1,
			PullUpDown = 2
		};

		enum class PortOutputMode
		{
			PushPull = 0,
			OpenDrain = 1,
			AF_PushPull = 2,
			AF_OpenDrain = 3
		};

		enum class PortSpeed
		{
			PS_10MHz = 1,
			PS_2MHz = 2,
			PS_50MHz = 1
		};

		class PortDevice;

		class PortPin : public Device, public ExclusiveObjectAccess
		{
		public:
			PortPin(ObjectAccessor<PortDevice>&& port, PortPins pin);

			void SetMode(PortInputMode mode);
			void SetMode(PortOutputMode mode, PortSpeed speed);
		protected:
			virtual void OnLastClose() override;
		private:
			ObjectAccessor<PortDevice> port_;
			PortPins pin_;
		};

		class PortDevice : public Device, public FreeObjectAccess
		{
		public:
			PortDevice(const FDTDevice & fdt);

			ObjectAccessor<PortPin> OpenPin(PortPins pin);
		private:
			friend class PortPin;

			void ClosePin(PortPins pin) noexcept;
			void MarkPinUsed(PortPins pin, bool used) noexcept;
			void ValidateExclusiveUsePin(PortPins pin);
		private:
			uintptr_t regAddr_;
			uint32_t usedPins_;
		};

		class PortDriver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(PortDriver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
			uintptr_t regAddr_;
		};
	}
}
