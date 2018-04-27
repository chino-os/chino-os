//
// Chino Bootloader
//
#include <efi.h>
#include <efilib.h>
#include <elfload.h>
#include <portable.h>
#include "../kernel/kernel_iface.h"

static CHAR16 KernelFilePath[] = L"CHINO\\SYSTEM\\KERNEL";
static EFI_HANDLE gImageHandle;

#define ExitIfError(status) \
{ EFI_STATUS s = status; if (EFI_ERROR(s)) { Print(L"Error (%s:%d): %d\n", __func__, __LINE__, (int)s); BS->Exit(gImageHandle, s, 0, NULL); } }

#define ExitIfNot(value, expected, tag) \
{ int s = value; if (s != expected) { Print(L"Error %s (%s:%d): value %d, expected: %d\n", tag, __func__, __LINE__, (int)s, (int)expected); BS->Exit(gImageHandle, -1, 0, NULL); } }

bool ReadFile(struct el_ctx *ctx, void *dest, size_t nb, size_t offset)
{
	EFI_FILE_HANDLE file = (EFI_FILE_HANDLE)ctx->user;
	ExitIfError(file->SetPosition(file, offset));
	UINTN toRead = nb;
	ExitIfError(file->Read(file, &toRead, dest));
	ExitIfNot(toRead, nb, L"Not read All");
	return true;
}

static void *alloccb(
	el_ctx *ctx,
	Elf_Addr phys,
	Elf_Addr virt,
	Elf_Addr size)
{
	(void)ctx;
	(void)phys;
	(void)size;
	return (void*)virt;
}

static CHAR16 *OsLoaderMemoryTypeDesc[EfiMaxMemoryType] = {
	L"reserved  ",
	L"LoaderCode",
	L"LoaderData",
	L"BS_code   ",
	L"BS_data   ",
	L"RT_code   ",
	L"RT_data   ",
	L"available ",
	L"Unusable  ",
	L"ACPI_recl ",
	L"ACPI_NVS  ",
	L"MemMapIO  ",
	L"MemPortIO ",
	L"PAL_code  "
};

void SetGraphicsMode(struct BootParameters* bootParam);

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	gImageHandle = ImageHandle;
	InitializeLib(ImageHandle, SystemTable);

	EFI_LOADED_IMAGE* loadedImage;
	EFI_FILE_IO_INTERFACE* volume;

	EFI_SERIAL_IO_PROTOCOL* uart1;
	//ExitIfError(BS->LocateProtocol(&gEfiSerialIoProtocolGuid, NULL, (void**)&uart1));
	//CHAR8 str[] = "Hello\n"; UINTN size = sizeof(str);
	//ExitIfError(uart1->Write(uart1, &size, str));
	DumpACPI();

	ExitIfError(BS->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&loadedImage));
	ExitIfError(BS->HandleProtocol(loadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&volume));

	EFI_FILE_HANDLE rootFS, fileHandle;
	ExitIfError(volume->OpenVolume(volume, &rootFS));
	// 读取文件
	ExitIfError(rootFS->Open(rootFS, &fileHandle, KernelFilePath, EFI_FILE_MODE_READ, 0));
	
	//EFI_FILE_INFO fileInfo; UINTN s = sizeof(fileInfo);
	//ExitIfError(fileHandle->GetInfo(fileHandle, &gEfiFileInfoGuid, &s, &fileInfo));
	//
	//Print(L"Kernel Size: %d bytes\n", (int)fileInfo.FileSize);

	el_ctx ctx = { .user = fileHandle, .pread = ReadFile };
	ExitIfNot(el_init(&ctx), EL_OK, L"Init elf failed");

	// 加载 Kernel
	Elf_Addr kernelBase;
	ExitIfError(BS->AllocatePool(EFI_ChinoKernel_Code, ctx.memsz, (void**)&kernelBase));
	ctx.base_load_vaddr = ctx.base_load_paddr = kernelBase;
	ExitIfNot(el_load(&ctx, alloccb), EL_OK, L"load elf failed");
	ExitIfNot(el_relocate(&ctx), EL_OK, L"relocate elf failed");
	ExitIfError(fileHandle->Close(fileHandle));
	ExitIfError(rootFS->Close(rootFS));
	
	// 跳转 Kernel
	kernel_entry_t kernelEntry = (kernel_entry_t)(ctx.ehdr.e_entry + kernelBase);
	struct BootParameters bootParam = {};
	ExitIfError(BS->AllocatePool(EFI_ChinoKernel_Data, ChinoKernel_StackSize, (void**)&bootParam.StackPointer));
	bootParam.StackPointer = bootParam.StackPointer + ChinoKernel_StackSize - 1;
	bootParam.EfiRuntimeService = RT;

	// 设置视频
	SetGraphicsMode(&bootParam);

	// 填写 Memroy Map
	UINTN entries, mapKey, descriptorSize;
	UINT32 descriptorVersion;
	UINTN totalSize = 0;
	EFI_MEMORY_DESCRIPTOR* descriptor;
	BS->GetMemoryMap(&totalSize, NULL, &mapKey, &descriptorSize, &descriptorVersion);
	ExitIfError(BS->AllocatePool(EFI_ChinoKernel_Data, totalSize, (void**)&descriptor));
	ExitIfError(BS->GetMemoryMap(&totalSize, descriptor, &mapKey, &descriptorSize, &descriptorVersion));

	ExitIfError(BS->ExitBootServices(ImageHandle, mapKey));
	bootParam.EfiMemoryDescriptor = descriptor;

	PortEnterKernel(&bootParam, kernelEntry);

	return EFI_SUCCESS;
}

enum
{
	DESIRED_HREZ = 800,
	DESIRED_VREZ = 600,
	DESIRED_PIXEL_FORMAT = PixelBlueGreenRedReserved8BitPerColor
};

void SetGraphicsMode(struct BootParameters* bootParam)
{
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
	EFI_HANDLE* handleBuffer;
	UINTN handleCount = 0;
	UINTN sizeOfInfo;
	EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* gopModeInfo;
	ExitIfError(BS->LocateHandleBuffer(ByProtocol, &gEfiGraphicsOutputProtocolGuid, NULL, &handleCount, &handleBuffer));
	ExitIfError(BS->HandleProtocol(handleBuffer[0], &gEfiGraphicsOutputProtocolGuid, (void**)&gop));

	size_t modeIndex = 0;
	for (;; modeIndex++) {
		ExitIfError(gop->QueryMode(gop, modeIndex, &sizeOfInfo, &gopModeInfo));
		if (gopModeInfo->HorizontalResolution == DESIRED_HREZ &&
			gopModeInfo->VerticalResolution == DESIRED_VREZ &&
			gopModeInfo->PixelFormat == DESIRED_PIXEL_FORMAT)
			break;
	}
	ExitIfError(gop->SetMode(gop, modeIndex));

	Print(L"Change Graphics Mode: %dx%d\n", DESIRED_HREZ, DESIRED_VREZ);
	bootParam->FrameBufferBase = (uintptr_t)gop->Mode->FrameBufferBase;
	bootParam->FrameBufferSize = (uintptr_t)gop->Mode->FrameBufferSize;
	bootParam->FrameBufferWidth = DESIRED_HREZ;
	bootParam->FrameBufferHeight = DESIRED_VREZ;
}