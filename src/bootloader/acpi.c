//#include <stddef.h>
//#include <efi.h>
//#include <efilib.h>
//
//extern EFI_HANDLE gImageHandle;
//
//#define ExitIfError(status) \
//{ EFI_STATUS s = status; if (EFI_ERROR(s)) { Print(L"Error (%s:%d): %d\n", __func__, __LINE__, (int)s); BS->Exit(gImageHandle, s, 0, NULL); } }
//
//#define ExitIfNot(value, expected, tag) \
//{ int s = value; if (s != expected) { Print(L"Error %s (%s:%d): value %d, expected: %d\n", tag, __func__, __LINE__, (int)s, (int)expected); BS->Exit(gImageHandle, -1, 0, NULL); } }
//
//
//struct acpi_header
//{
//	uint32_t signature;
//	uint32_t length;
//};
//
//typedef EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER RSDP_t;
//
//EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE* FindFADT(EFI_ACPI_DESCRIPTION_HEADER* xsdt)
//{
//	size_t entries = (xsdt->Length - sizeof(EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);
//
//	UINT64* entryBegin = (UINT64*)(xsdt + 1);
//	for (size_t i = 0; i < entries; i++)
//	{
//		EFI_ACPI_DESCRIPTION_HEADER* header = (EFI_ACPI_DESCRIPTION_HEADER*)entryBegin[i];
//		if (header->Signature == EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE)
//			return (EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE*)header;
//	}
//
//	ExitIfError(EFI_NOT_FOUND);
//}
//
//EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_SPACE_ACCESS_TABLE* FindMCFG(EFI_ACPI_DESCRIPTION_HEADER* xsdt)
//{
//	size_t entries = (xsdt->Length - sizeof(EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);
//
//	UINT64* entryBegin = (UINT64*)(xsdt + 1);
//	for (size_t i = 0; i < entries; i++)
//	{
//		EFI_ACPI_DESCRIPTION_HEADER* header = (EFI_ACPI_DESCRIPTION_HEADER*)entryBegin[i];
//		if (header->Signature == EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_SPACE_ACCESS_TABLE_SIGNATURE)
//			return (EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_SPACE_ACCESS_TABLE*)header;
//	}
//
//	ExitIfError(EFI_NOT_FOUND);
//}
//
//EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER* FindMADT(EFI_ACPI_DESCRIPTION_HEADER* xsdt)
//{
//	size_t entries = (xsdt->Length - sizeof(EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);
//
//	UINT64* entryBegin = (UINT64*)(xsdt + 1);
//	for (size_t i = 0; i < entries; i++)
//	{
//		EFI_ACPI_DESCRIPTION_HEADER* header = (EFI_ACPI_DESCRIPTION_HEADER*)entryBegin[i];
//		if (header->Signature == EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE)
//			return (EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER*)header;
//	}
//
//	ExitIfError(EFI_NOT_FOUND);
//}
//
//PCI_TYPE00* FindPCIDevice(EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE* mcfg, UINT8 classCode[])
//{
//	UINT64 startBus = mcfg->StartBusNumber;
//	UINT64 endBus = mcfg->EndBusNumber;
//	for (size_t bus = 0; bus <= endBus; bus++)
//	{
//		for (size_t dev = 0; dev <= PCI_MAX_DEVICE; dev++)
//		{
//			UINT64 address0 = mcfg->BaseAddress + ((bus - startBus) << 20 | dev << 15);
//			PCI_DEVICE_INDEPENDENT_REGION* header0 = (PCI_DEVICE_INDEPENDENT_REGION*)address0;
//			size_t funcCount = header0->HeaderType & HEADER_TYPE_MULTI_FUNCTION ? PCI_MAX_FUNC + 1 : 1;
//			for (size_t func = 0; func < funcCount; func++)
//			{
//				UINT64 address = mcfg->BaseAddress + ((bus - startBus) << 20 | dev << 15 | func << 12);
//				PCI_DEVICE_INDEPENDENT_REGION* header = (PCI_DEVICE_INDEPENDENT_REGION*)address;
//
//				if (header->VendorId == 0xFFFF) continue;
//
//				if (header->HeaderType != HEADER_TYPE_DEVICE && header->HeaderType != HEADER_TYPE_MULTI_FUNCTION)
//					continue;
//
//				if (header->ClassCode[2] == classCode[0] &&
//					header->ClassCode[1] == classCode[1] &&
//					header->ClassCode[0] == classCode[2])
//					return (PCI_TYPE00*)header;
//			}
//		}
//	}
//
//	ExitIfError(EFI_NOT_FOUND);
//}
//
//void DumpACPI()
//{
//	EFI_CONFIGURATION_TABLE* efiRdsp;
//
//	RSDP_t* rsdp = (RSDP_t*)efiRdsp->VendorTable;
//	Print(L"RSDP: %lX\n", rsdp);
//
//	EFI_ACPI_DESCRIPTION_HEADER* xsdtHeader = (EFI_ACPI_DESCRIPTION_HEADER*)rsdp->XsdtAddress;
//	{
//		size_t entries = (xsdtHeader->Length - sizeof(EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);
//
//		UINT64* entryBegin = (UINT64*)(xsdtHeader + 1);
//		Print(L"XSDT: ");
//		for (size_t i = 0; i < entries; i++)
//		{
//			EFI_ACPI_DESCRIPTION_HEADER* header = (EFI_ACPI_DESCRIPTION_HEADER*)entryBegin[i];
//			char* str = (char*)&header->Signature;
//			Print(L"%c%c%c%c ", str[0], str[1], str[2], str[3]);
//		}
//	}
//
//	EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER* madt = FindMADT(xsdtHeader);
//	Print(L"\nLocal APIC Address: %X\n", madt->LocalApicAddress);
//
//
//	EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE* fadt = FindFADT(xsdtHeader);
//	EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_SPACE_ACCESS_TABLE* mcfg = FindMCFG(xsdtHeader);
//
//	Print(L"PCI Bar: %lx\n Start Bus: %d\n", mcfg->Configuration.BaseAddress, (int)mcfg->Configuration.StartBusNumber);
//
//#if 0
//	UINT64 startBus = mcfg->Configuration.StartBusNumber;
//	UINT64 endBus = mcfg->Configuration.EndBusNumber;
//	for (size_t bus = 0; bus <= endBus; bus++)
//	{
//		for (size_t dev = 0; dev <= PCI_MAX_DEVICE; dev++)
//		{
//			UINT64 address0 = mcfg->Configuration.BaseAddress + ((bus - startBus) << 20 | dev << 15);
//			PCI_DEVICE_INDEPENDENT_REGION* header0 = (PCI_DEVICE_INDEPENDENT_REGION*)address0;
//			size_t funcCount = header0->HeaderType & HEADER_TYPE_MULTI_FUNCTION ? PCI_MAX_FUNC + 1 : 1;
//			for (size_t func = 0; func < funcCount; func++)
//			{
//				UINT64 address = mcfg->Configuration.BaseAddress + ((bus - startBus) << 20 | dev << 15 | func << 12);
//				PCI_DEVICE_INDEPENDENT_REGION* header = (PCI_DEVICE_INDEPENDENT_REGION*)address;
//				if (header->VendorId == 0xFFFF) continue;
//
//				Print(L"Bus(%d)/Dev(%d)/Func(%d): Class(0x%x, 0x%x, 0x%x)\n", (int)bus, (int)dev, (int)func, (int)header->ClassCode[2], (int)header->ClassCode[1], (int)header->ClassCode[0]);
//			}
//		}
//	}
//#endif
//
//	UINT8 isaBridgeClassCode[] = { PCI_CLASS_BRIDGE , PCI_CLASS_ISA , 0 };
//	PCI_TYPE00* isaBridgeConfig = FindPCIDevice(&mcfg->Configuration, isaBridgeClassCode);
//	Print(L"Isa Bridge: Vendor(%X) \n", isaBridgeConfig->Hdr.VendorId);
//
//	UINT8 vgaClassCode[] = { PCI_CLASS_DISPLAY_CTRL , PCI_CLASS_VGA , 0 };
//	PCI_TYPE00* vgaConfig = FindPCIDevice(&mcfg->Configuration, vgaClassCode);
//	Print(L"VGA Controller: Vendor(%X) \n", vgaConfig->Hdr.VendorId);
//}