//
// Kernel Device
//
#pragma once
#include "../Device.hpp"
#include <kernel/graphics/GfxDefs.hpp>

namespace Chino
{
	namespace Device
	{
		class DisplayDevice : public Device
		{
		public:
			virtual ObjectPtr<Graphics::Surface> OpenPrimarySurface(ObjectAccess access) = 0;

			virtual void CopySubresource(Graphics::Surface& src, Graphics::Surface& dest, const Graphics::RectU& srcRect, const Graphics::PointU& destPosition) = 0;
		};
	}
}
