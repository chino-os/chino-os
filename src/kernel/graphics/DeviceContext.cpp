//
// Kernel Device
//
#include "DeviceContext.hpp"
#include "../kdebug.hpp"

using namespace Chino;
using namespace Chino::Graphics;

class SoftwareSurface : public Surface
{

};

DeviceContext::DeviceContext(ObjectAccessor<Device::DisplayDevice>&& device)
	:device_(std::move(device))
{
	primarySurface_ = device_->OpenPrimarySurface();
}

ObjectPtr<Surface> DeviceContext::CreatePrimarySurface() noexcept
{
	return primarySurface_;
}

ObjectPtr<Surface> DeviceContext::CreateOffscreenSurface(ColorFormat format, size_t width, size_t height)
{
	return nullptr;
}

void DeviceContext::CopySubresource(Surface& src, Surface& dest, const RectU& srcRect, const PointU& destPosition)
{

}
