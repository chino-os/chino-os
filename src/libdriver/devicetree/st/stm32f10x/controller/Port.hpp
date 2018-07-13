//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>
#include <noncopyable.hpp>

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
			Analog,
			Floating,
			PullUp,
			PullDown
		};

		enum class PortOutputMode
		{
			PushPull,
			OpenDrain,
			AF_PushPull,
			AF_OpenDrain
		};

		enum class PortOutputSpeed
		{
			PS_10MHz = 0b01,
			PS_2MHz = 0b10,
			PS_50MHz = 0b11
		};

		class PortDevice;

		class PortPin : public Device, public ExclusiveObjectAccess
		{
		public:
			PortPin(ObjectAccessor<PortDevice>&& port, PortPins pin);

			void SetMode(PortInputMode mode);
			void SetMode(PortOutputMode mode, PortOutputSpeed speed);

			uint32_t GetValue();
			void SetValue(uint32_t value);
		protected:
			virtual void OnLastClose() override;
		private:
			ObjectAccessor<PortDevice> portDevice_;
			PortPins pin_;
		};

		class PortDevice : public Device, public FreeObjectAccess
		{
		public:
			static constexpr size_t PinCount = 16;

			PortDevice(const FDTDevice & fdt);

			ObjectAccessor<PortPin> OpenPin(PortPins pin);
		protected:
			virtual void OnFirstOpen() override;
			virtual void OnLastClose() override;
		private:
			friend class PortPin;

			void ClosePin(PortPins pin) noexcept;
			void MarkPinUsed(PortPins pin, bool used) noexcept;
			void ValidateExclusiveUsePin(PortPins pin);

			void SetMode(PortPins pin, PortInputMode mode);
			void SetMode(PortPins pin, PortOutputMode mode, PortOutputSpeed speed);

			uint32_t GetValue(PortPins pin);
			void SetValue(PortPins pin, uint32_t value);
		private:
			uintptr_t regAddr_;
			uint32_t usedPins_;
			uint32_t portIdx_;
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
