//
// Kernel Audio
//
#pragma once
#include "../kernel_iface.h"
#include "../object/Object.hpp"
#include <gsl/gsl>

namespace Chino
{
	namespace Audio
	{
		enum class AudioFormatTag
		{
			PCM,
			AutoDetect,
		};

		struct AudioFormat
		{
			AudioFormatTag Tag;
			uint16_t NumOfChannels;
			uint32_t SampleRate;
			uint16_t BlockAlign;
			uint16_t BitsPerSample;
		};

		struct IAudioRenderClient : public Object
		{
			virtual void GetBuffer(gsl::span<uint8_t>& buffer) = 0;
			virtual void ReleaseBuffer() = 0;
		};

		struct IAudioClient : public Object
		{
			virtual bool IsFormatSupported(const AudioFormat& format) = 0;
			virtual void SetFormat(const AudioFormat& format) = 0;
			virtual ObjectPtr<IAudioRenderClient> GetRenderClient() = 0;
			virtual void Start() = 0;
			virtual void Stop() = 0;
		};
	}
}
