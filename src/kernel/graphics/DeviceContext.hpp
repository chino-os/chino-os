//
// Kernel Device
//
#pragma once
#include "GfxDefs.hpp"
#include "../device/display/Display.hpp"

namespace Chino
{
	namespace Graphics
	{
		class DeviceContext : public Object
		{
		public:
			DeviceContext(ObjectAccessor<Device::DisplayDevice>&& device);

			ObjectPtr<Surface> CreatePrimarySurface() noexcept;
			ObjectPtr<Surface> CreateOffscreenSurface(ColorFormat format, const SizeU& size);
			ObjectPtr<Surface> CreateOffscreenSurface(ColorFormat format, const SizeU& size, const SurfaceData& data, bool copy = true);

			void CopySubresource(Surface& src, Surface& dest, const RectU& srcRect, const PointU& destPosition);
		private:
			ObjectAccessor<Device::DisplayDevice> device_;
			ObjectPtr<Surface> primarySurface_;
		};
	}
}
