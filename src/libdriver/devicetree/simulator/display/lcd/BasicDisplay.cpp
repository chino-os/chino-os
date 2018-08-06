//
// Kernel Device
//
#include "BasicDisplay.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <kernel/device/display/Display.hpp>
#include <kernel/threading/ThreadSynchronizer.hpp>
#include <libbsp/bsp.hpp>
#include <Windows.h>
#include <process.h>
#include <ddraw.h>
#include <wrl.h>
#include <string>
#include <array>

using namespace Chino;
using namespace Chino::Device;
using namespace Chino::Graphics;
using namespace Chino::Threading;
namespace wrl = Microsoft::WRL;

DEFINE_FDT_DRIVER_DESC_1(BasicDisplayDriver, "display", "simulator,basic-display");

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static const wchar_t _wndClassName[] = L"Chino Simulator Window";

#define coassert(x) kassert(SUCCEEDED(x))

enum WindowLongPtrParam
{
	WLPP_NativeWindowPtr,
	WLPP_Count
};

struct WindowClassRegiser
{
	WindowClassRegiser()
	{
		auto hInstance = GetModuleHandle(nullptr);

		WNDCLASSEX wndClass = { 0 };
		wndClass.cbSize = sizeof(wndClass);
		wndClass.lpszClassName = _wndClassName;
		wndClass.cbWndExtra = WLPP_Count * sizeof(LONG_PTR);
		wndClass.style = CS_HREDRAW | CS_VREDRAW;
		wndClass.hInstance = hInstance;
		wndClass.lpfnWndProc = WndProc;
		wndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

		kassert(RegisterClassEx(&wndClass));
	}
};

class NativeWindow
{
public:
	NativeWindow(HANDLE readyEvent)
		:readyEvent_(std::move(readyEvent))
	{

	}

	IDirectDrawSurface7* GetBackSurface() noexcept
	{
		return offSurface_.Get();
	}

	LRESULT OnWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	static void ThreadMain(void* this_)
	{
		auto self = reinterpret_cast<NativeWindow*>(this_);
		self->Main();
	}

	void Quit()
	{
		PostMessage(hWnd_, WM_QUIT, 0, 0);
	}
private:
	void Main()
	{
		static WindowClassRegiser clsReg;

		auto style = WS_OVERLAPPED | WS_CAPTION;
		RECT rect{ 0,0,320,480 };
		AdjustWindowRect(&rect, style, FALSE);

		hWnd_ = CreateWindow(_wndClassName, L"Basic Display", style, CW_USEDEFAULT, CW_USEDEFAULT,
			rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, GetModuleHandle(nullptr), this);
		kassert(hWnd_);

		CreateGraphicsResources();

		ShowWindow(hWnd_, SW_SHOW);
		UpdateWindow(hWnd_);

		SetEvent(readyEvent_);

		MSG msg;
		while (true)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					break;

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				RECT rect{ 0,0 };
				ClientToScreen(hWnd_, (LPPOINT)&rect);
				rect.right = rect.left + 320;
				rect.bottom = rect.top + 480;
				coassert(mainSurface_->Blt(&rect, offSurface_.Get(), nullptr, DDBLT_WAIT, nullptr));
				Sleep(1);
			}
		}
	}

	void CreateGraphicsResources()
	{
		coassert(DirectDrawCreateEx(nullptr, &ddraw_, IID_IDirectDraw7, nullptr));
		coassert(ddraw_->SetCooperativeLevel(hWnd_, DDSCL_NORMAL));

		{
			DDSURFACEDESC2 ddsd{};
			ddsd.dwSize = sizeof(ddsd);
			ddsd.dwFlags = DDSD_CAPS;
			ddsd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
			coassert(ddraw_->CreateSurface(&ddsd, &mainSurface_, nullptr));

			wrl::ComPtr<IDirectDrawClipper> clipper;
			coassert(ddraw_->CreateClipper(0, &clipper, nullptr));
			coassert(clipper->SetHWnd(0, hWnd_));
			coassert(mainSurface_->SetClipper(clipper.Get()));
		}

		{
			DDSURFACEDESC2 ddsd{};
			ddsd.dwSize = sizeof(ddsd);
			ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
			ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN;

			RECT rect;
			GetClientRect(hWnd_, &rect);
			ddsd.dwWidth = rect.right - rect.left;
			ddsd.dwHeight = rect.bottom - rect.top;
			ddsd.ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
			ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
			ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
			ddsd.ddpfPixelFormat.dwRBitMask = 0b11111 << 11;
			ddsd.ddpfPixelFormat.dwGBitMask = 0b111111 << 5;
			ddsd.ddpfPixelFormat.dwBBitMask = 0b11111;

			coassert(ddraw_->CreateSurface(&ddsd, &offSurface_, nullptr));
		}
	}

	void DiscardGraphicsResources()
	{

	}
private:
	HWND hWnd_;
	HANDLE readyEvent_;
	wrl::ComPtr<IDirectDraw7> ddraw_;
	wrl::ComPtr<IDirectDrawSurface7> mainSurface_, offSurface_;
};

class BasicDisplaySurface : public Surface
{
public:
	BasicDisplaySurface(NativeWindow& window)
		:window_(window)
	{

	}

	IDirectDrawSurface7* GetBackSurface() noexcept
	{
		return window_.GetBackSurface();
	}

	virtual SizeU GetPixelSize() noexcept override
	{
		return { 320,480 };
	}

	virtual ColorFormat GetFormat() noexcept override
	{
		return ColorFormat::B5G6R5_UNORM;
	}

	virtual SurfaceLocation GetLocation() noexcept override
	{
		return SurfaceLocation::DeviceMemory;
	}

	virtual SurfaceData Lock(const RectU& rect) override
	{
		throw std::runtime_error("Not supported.");
	}

	virtual void Unlock(SurfaceData& data) override
	{
		throw std::runtime_error("Not supported.");
	}
private:
	NativeWindow & window_;
};

class BasicDisplayDevice : public DisplayDevice, public ExclusiveObjectAccess
{
public:
	static constexpr uint16_t PixelWidth = 320;
	static constexpr uint16_t PixelHeight = 240;

	BasicDisplayDevice(const FDTDevice& fdt)
		:fdt_(fdt), readyEvent_(CreateEvent(nullptr, FALSE, FALSE, nullptr)), window_(readyEvent_)
	{
		g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
	}

	virtual ObjectPtr<Surface> OpenPrimarySurface() override
	{
		return MakeObject<BasicDisplaySurface>(window_);
	}

	virtual void Clear(Graphics::Surface& src, const RectU& rect, const Graphics::ColorValue& color) override
	{
		auto devSurface = dynamic_cast<BasicDisplaySurface*>(&src);
		kassert(devSurface);

		RECT drect{ rect.Left, rect.Top, rect.Right, rect.Bottom };
		DDSURFACEDESC2 ddsd{};
		ddsd.dwSize = sizeof(ddsd);

		coassert(devSurface->GetBackSurface()->Lock(&drect, &ddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY, nullptr));

		auto bytes = rect.GetSize().Width * rect.GetSize().Height * GetPixelBytes(ColorFormat::B5G6R5_UNORM);

		auto data = reinterpret_cast<uint16_t*>(ddsd.lpSurface);
		auto value = Rgb565::From(color).Value;
		for (size_t y = 0; y < rect.GetSize().Height; y++)
		{
			for (size_t x = 0; x < rect.GetSize().Width; x++)
				data[x] = value;

			data += ddsd.lPitch / 2;
		}

		devSurface->GetBackSurface()->Unlock(&drect);
	}

	virtual void CopySubresource(Surface& src, Surface& dest, const RectU& srcRect, const PointU& destPosition) override
	{
		auto* devSurface = dynamic_cast<BasicDisplaySurface*>(&src);

		// Copy from device
		if (devSurface)
		{
			RECT rect;
			rect.left = srcRect.Left;
			rect.top = srcRect.Top;
			rect.right = srcRect.Right;
			rect.bottom = srcRect.Bottom;

			DDSURFACEDESC2 ddsd;
			coassert(devSurface->GetBackSurface()->Lock(&rect, &ddsd, DDLOCK_WAIT | DDLOCK_READONLY, nullptr));
			devSurface->GetBackSurface()->Unlock(&rect);
		}

		devSurface = dynamic_cast<BasicDisplaySurface*>(&dest);
		// Copy to device
		kassert(devSurface);
		{
			RECT rect;
			DDSURFACEDESC2 ddsd;
			rect.left = destPosition.X;
			rect.top = destPosition.Y;
			rect.right = destPosition.X + srcRect.GetSize().Width;
			rect.bottom = destPosition.Y + srcRect.GetSize().Height;

			auto locker = src.Lock(srcRect);
			coassert(devSurface->GetBackSurface()->Lock(&rect, &ddsd, DDLOCK_WAIT | DDLOCK_WRITEONLY, nullptr));

			auto lineSize = srcRect.GetSize().Width * GetPixelBytes(src.GetFormat());
			auto srcData = locker.Data.data();
			auto destData = reinterpret_cast<uint8_t*>(ddsd.lpSurface);
			for (size_t y = 0; y < srcRect.GetSize().Height; y++)
			{
				auto begin = srcData + y * locker.Stride;
				std::copy(begin, begin + lineSize, destData + y * ddsd.lPitch);
			}

			devSurface->GetBackSurface()->Unlock(&rect);
			src.Unlock(locker);
		}
	}
protected:
	virtual void OnFirstOpen() override
	{
		wndThread_ = _beginthread(NativeWindow::ThreadMain, 1 * 1024 * 1024, &window_);
		WaitForSingleObject(readyEvent_, INFINITE);
	}

	virtual void OnLastClose() override
	{
		window_.Quit();
	}
private:
private:
	uintptr_t wndThread_;
	HANDLE readyEvent_;
	NativeWindow window_;
	const FDTDevice& fdt_;
};

LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (msg == WM_NCCREATE)
	{
		auto params = reinterpret_cast<const CREATESTRUCT*>(lParam);
		SetWindowLongPtr(hWnd, WLPP_NativeWindowPtr * sizeof(LONG_PTR), (LONG_PTR)params->lpCreateParams);
	}

	auto nativeWindow = reinterpret_cast<NativeWindow*>(GetWindowLongPtr(hWnd, WLPP_NativeWindowPtr));
	if (nativeWindow)
		return nativeWindow->OnWindowProc(hWnd, msg, wParam, lParam);
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

BasicDisplayDriver::BasicDisplayDriver(const FDTDevice& device)
	:device_(device)
{

}

void BasicDisplayDriver::Install()
{
	g_DeviceMgr->InstallDevice(MakeObject<BasicDisplayDevice>(device_));
}
