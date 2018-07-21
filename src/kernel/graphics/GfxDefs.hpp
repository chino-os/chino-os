//
// Kernel Device
//
#pragma once
#include "../kernel_iface.h"
#include <gsl/gsl>

namespace Chino
{
	namespace Graphics
	{
		struct ColorValue
		{
			float R, G, B, A;
		};

		enum class ColorFormat
		{
			B5G6R5_UNORM,
			R32G32B32A32_FLOAT
		};

		template<ColorFormat>
		struct Color;

		template<>
		struct Color<ColorFormat::B5G6R5_UNORM>
		{
			uint16_t Value;
		};

		template<>
		struct Color<ColorFormat::R32G32B32A32_FLOAT>
		{
			float R, G, B, A;
		};

		using Rgb565 = Color<ColorFormat::B5G6R5_UNORM>;
	}
}
