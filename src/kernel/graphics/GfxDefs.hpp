//
// Kernel Device
//
#pragma once
#include "../kernel_iface.h"
#include "../object/Object.hpp"
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

		struct SizeU
		{
			uint32_t Width, Height;
		};

		struct RectU
		{
			uint32_t Left, Top, Right, Bottom;
		};

		struct PointU
		{
			uint32_t X, Y;
		};

		struct SurfaceData
		{
			gsl::span<uint8_t> Data;
			size_t Stride;
			RectU Rect;
		};

		class Surface : public Object
		{
		public:
			virtual SizeU GetPixelSize() noexcept = 0;
			virtual ColorFormat GetFormat() noexcept = 0;

			virtual SurfaceData Lock(const RectU& rect) = 0;
			virtual void Unlock(SurfaceData& data) = 0;
		};
	}
}
