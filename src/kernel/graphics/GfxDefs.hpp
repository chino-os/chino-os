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

			static Color From(ColorValue value) noexcept
			{
				auto r = value.R * 31;
				auto g = value.G * 63;
				auto b = value.B * 31;
				return { static_cast<uint16_t>((uint16_t(r) << 11) | (uint16_t(g) << 5) | uint16_t(b)) };
			}
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

		struct PointU
		{
			uint32_t X, Y;
		};

		struct RectU
		{
			uint32_t Left, Top, Right, Bottom;

			RectU() = default;
			RectU(const PointU& position, const SizeU& size) noexcept
				:Left(position.X), Top(position.Y), Right(position.X + size.Width), Bottom(position.Y + size.Height)
			{

			}

			SizeU GetSize() const noexcept { return { Right - Left,Bottom - Top }; }
		};

		size_t GetPixelBytes(ColorFormat format);

		struct SurfaceData
		{
			gsl::span<uint8_t> Data;
			size_t Stride;
			RectU Rect;
		};

		enum class SurfaceLocation
		{
			SystemMemory,
			DeviceMemory
		};

		class Surface : public Object
		{
		public:
			virtual SizeU GetPixelSize() noexcept = 0;
			virtual ColorFormat GetFormat() noexcept = 0;
			virtual SurfaceLocation GetLocation() noexcept = 0;

			virtual SurfaceData Lock(const RectU& rect) = 0;
			virtual void Unlock(SurfaceData& data) = 0;
		};
	}
}
