//
// Chino Bootloader
//
extern "C"
{
#include <efi.h>
#include <efilib.h>
}
#include "../kernel/kernel_iface.hpp"

#define T(x) reinterpret_cast<const CHAR16*>(L##x)

#define ThrowIfError(status) \
ThrowIfErrorM(status, T("Unexpected exception: 0x%08x at %s:%d\n"), status & 0xFFFFFFFF, __func__, __LINE__);

static const wchar_t KernelFilePath[] = LR"(chino\system\kernel)";

class Bootloader
{
public:
	Bootloader(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
		:imageHandle_(ImageHandle), systemTable_(SystemTable)
	{
		InitializeLib(ImageHandle, SystemTable);
		Print(T("Booting...\n"));

		EFI_LOADED_IMAGE* loadedImage;
		EFI_FILE_IO_INTERFACE* volume;

		// 获取 Loader 所在的卷
		ThrowIfError(BS->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&loadedImage));
		ThrowIfError(BS->HandleProtocol(loadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&volume));

		EFI_FILE_HANDLE rootFS, fileHandle;
		ThrowIfError(volume->OpenVolume(volume, &rootFS));
		// 读取文件
		ThrowIfErrorM(rootFS->Open(rootFS, &fileHandle, (CHAR16*)KernelFilePath, EFI_FILE_MODE_READ, 0), T("Cannot Open Chino Kernel File.\r\n"));
	}

	void Run();

	template<typename ...Args>
	void ThrowIfErrorM(EFI_STATUS status, const CHAR16* format, Args ...args)
	{
		if (EFI_ERROR(status))
		{
			Print(format, args...);
			Exit(status, 0, nullptr);
		}
	}

	template<typename ...Args>
	void Log(const CHAR16* format, Args ...args)
	{
		Print(format, args...);
	}
private:
	EFI_HANDLE imageHandle_;
	EFI_SYSTEM_TABLE* systemTable_;
};

void Bootloader::Run()
{
	EFI_LOADED_IMAGE* loadedImage;
	EFI_FILE_IO_INTERFACE* volume;
	// 获取 Loader 所在的卷
	ThrowIfError(BS->HandleProtocol(imageHandle_, &gEfiLoadedImageProtocolGuid, (void**)&loadedImage));
	ThrowIfError(BS->HandleProtocol(loadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&volume));

	EFI_FILE_HANDLE rootFS, fileHandle;
	ThrowIfError(volume->OpenVolume(volume, &rootFS));
	// 读取文件
	ThrowIfErrorM(rootFS->Open(rootFS, &fileHandle, (CHAR16*)KernelFilePath, EFI_FILE_MODE_READ, 0), T("Cannot Open Chino Kernel File.\r\n"));
	
	EFI_FILE_INFO fileInfo;
	UINTN fileInfoSize = sizeof(fileInfo);
	ThrowIfError(fileHandle->GetInfo(fileHandle, &gEfiFileInfoGuid, &fileInfoSize, &fileInfo));

	Log(T("Kernel Size: %d bytes\n"), fileInfo.FileSize);
	//BS->AllocatePool(EFI_ChinoKernel_Code, fileInfo.Size, )
}

extern "C" 
{
	EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
	{
		Bootloader bootloader(ImageHandle, SystemTable);
		bootloader.Run();
		return EFI_SUCCESS;
	}
}