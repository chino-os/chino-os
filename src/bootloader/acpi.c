#include <efi.h>
#include <efilib.h>
#include <Acpi2_0.h>
#include "../kernel/acpi.h"

extern EFI_HANDLE gImageHandle;

#define ExitIfError(status) \
{ EFI_STATUS s = status; if (EFI_ERROR(s)) { Print(L"Error (%s:%d): %d\n", __func__, __LINE__, (int)s); BS->Exit(gImageHandle, s, 0, NULL); } }

EFI_STATUS searchEfiConfigurationTable(const EFI_GUID          *guid_p,
	EFI_CONFIGURATION_TABLE **entry_pp)
{
	EFI_STATUS ret;
	EFI_CONFIGURATION_TABLE *cfg_table_p = ST->ConfigurationTable;
	EFI_CONFIGURATION_TABLE *entry_p;
	UINTN i, count = ST->NumberOfTableEntries;
	for (i = 0, ret = EFI_NOT_FOUND; i<count; i++) {
		entry_p = &(cfg_table_p[i]);
		if (0 == CompareGuid((EFI_GUID*)guid_p, &(entry_p->VendorGuid))) {
			ret = EFI_SUCCESS;
			*entry_pp = entry_p;
			break;
		}
	}
	return ret;
}

struct acpi_header
{
	uint32_t signature;
	uint32_t length;
};

const EFI_GUID gEfiAcpiTableGuid = ACPI_20_TABLE_GUID;

typedef EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER RSDP_t;

void DumpACPI()
{
	EFI_CONFIGURATION_TABLE* efiRdsp;
	ExitIfError(searchEfiConfigurationTable(&gEfiAcpiTableGuid, &efiRdsp));

	RSDP_t* rsdp = (RSDP_t*)efiRdsp->VendorTable;

	Print(L"%lX\n", rsdp);

	//for (uintptr_t cntAcpiAddr = acpiBegin; cntAcpiAddr < acpiEnd;)
	//{
	//	struct acpi_header* cntAcpi = (struct acpi_header*)cntAcpiAddr;
	//	char* str = (char*)&cntAcpi->signature;
	//	Print(L"%c%c%c%c ", str[0], str[1], str[2], str[3]);
	//	if (cntAcpi->signature == 'GFCM')
	//	{
	//		Print(L"Find MCFG At %lX\n", cntAcpiAddr);
	//		break;
	//	}
	//
	//	cntAcpiAddr += cntAcpi->length;
	//}
}