//
// Kernel Device
//
#include "BasicDisplay.hpp"
#include <kernel/kdebug.hpp>
#include <kernel/object/ObjectManager.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <libbsp/bsp.hpp>
#include <Windows.h>
#include <process.h>
#include <d2d1.h>
#include <wrl.h>
#include <string>
#include <array>

using namespace Chino;
using namespace Chino::Device;
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
	NativeWindow()
	{

	}

	LRESULT OnWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_CREATE:
			OnCreate();
			return 0;
		case WM_PAINT:
			OnPaint();
			return 0;
		}

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
		RECT rect{ 0,0,320,240 };
		AdjustWindowRect(&rect, style, FALSE);

		hWnd_ = CreateWindow(_wndClassName, L"Basic Display", style, CW_USEDEFAULT, CW_USEDEFAULT,
			rect.right - rect.left, rect.bottom - rect.top, nullptr, nullptr, GetModuleHandle(nullptr), this);
		kassert(hWnd_);

		ShowWindow(hWnd_, SW_SHOW);
		UpdateWindow(hWnd_);

		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void OnCreate()
	{
		coassert(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2dFactory_.ReleaseAndGetAddressOf()));
	}

	void OnPaint()
	{
		auto hr = CreateGraphicsResources();
		if (SUCCEEDED(hr))
		{
			PAINTSTRUCT ps;
			BeginPaint(hWnd_, &ps);

			hWndRT_->BeginDraw();

			hWndRT_->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue));

			hr = hWndRT_->EndDraw();
			if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
				DiscardGraphicsResources();

			EndPaint(hWnd_, &ps);
		}
	}

	HRESULT CreateGraphicsResources()
	{
		HRESULT hr = S_OK;
		if (!hWndRT_)
		{
			RECT rc;
			GetClientRect(hWnd_, &rc);

			auto size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
			hr = d2dFactory_->CreateHwndRenderTarget(
				D2D1::RenderTargetProperties(),
				D2D1::HwndRenderTargetProperties(hWnd_, size),
				&hWndRT_);
		}

		return hr;
	}

	void DiscardGraphicsResources()
	{
		hWndRT_.Reset();
	}
private:
	HWND hWnd_;
	wrl::ComPtr<ID2D1Factory> d2dFactory_;
	wrl::ComPtr<ID2D1HwndRenderTarget> hWndRT_;
};

class BasicDisplayDevice : public Device, public ExclusiveObjectAccess
{
public:
	static constexpr uint16_t PixelWidth = 320;
	static constexpr uint16_t PixelHeight = 240;

	BasicDisplayDevice(const FDTDevice& fdt)
		:fdt_(fdt)
	{
		g_ObjectMgr->GetDirectory(WKD_Device).AddItem(fdt.GetName(), *this);
	}

	virtual void OnFirstOpen() override
	{
		wndThread_ = _beginthread(NativeWindow::ThreadMain, 1 * 1024 * 1024, &window_);
	}

	virtual void OnLastClose() override
	{
		window_.Quit();
	}
private:
private:
	uintptr_t wndThread_;
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
