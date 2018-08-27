//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>
#include <kernel/audio/AudioDefs.hpp>

namespace Chino
{
	namespace Device
	{
		class VS1053BDriver : public Driver
		{
		public:
			DECLARE_FDT_DRIVER(VS1053BDriver);

			virtual void Install() override;
		private:
			const FDTDevice& device_;
		};

		ObjectPtr<Audio::IAudioClient> CreateVS1053BAudioClient(ObjectAccessor<Device>&& device);
	}
}
