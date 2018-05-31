//
// Kernel Device
//
#pragma once

#include "../fdt/Fdt.hpp"
#include <noncopyable.hpp>

namespace Chino
{
	namespace Device
	{
		enum class PortPins
		{
			Pin0 = 0x0001,
			Pin1 = 0x0002,
			Pin2 = 0x0004,
			Pin3 = 0x0008,
			Pin4 = 0x0010,
			Pin5 = 0x0020,
			Pin6 = 0x0040,
			Pin7 = 0x0080,
			Pin8 = 0x0100,
			Pin9 = 0x0200,
			Pin10 = 0x0400,
			Pin11 = 0x0800,
			Pin12 = 0x1000,
			Pin13 = 0x2000,
			Pin14 = 0x4000,
			Pin15 = 0x8000
		};

		class PortDevice;

		class PortPin : NonCopyable
		{
		private:
			friend class PortDevice;

			PortPin(PortDevice& port, PortPins pin);
		private:
			ObjectPtr<PortDevice> port_;
			PortPins pin_;
		};

		class PortDevice : public Device
		{
		public:
			PortDevice(const FDTDevice& fdt);

			PortPin OpenPin(PortPins pin);
		private:
			friend class PortPin;

			void Close(PortPins pin);
		private:
			uintptr_t regAddr_;
		};
	}
}
