//
// Kernel Device
//
#include "Acpi.hpp"
#include "../../kdebug.hpp"
#include "../DeviceManager.hpp"

extern "C"
{
#include <efilink.h>
#include <pci22.h>
#include <MemoryMappedConfigurationSpaceAccessTable.h>
}

using namespace Chino::Device;

AcpiDriver::AcpiDriver(const BootParameters& bootParams)
	:rsdp_(bootParams.Acpi.Rsdp)
{
	kassert(bootParams.Acpi.Rsdp);
}

PCI_TYPE00* FindPCIDevice(EFI_ACPI_MEMORY_MAPPED_ENHANCED_CONFIGURATION_SPACE_BASE_ADDRESS_ALLOCATION_STRUCTURE* mcfg, UINT8 classCode[])
{
	UINT64 startBus = mcfg->StartBusNumber;
	UINT64 endBus = mcfg->EndBusNumber;
	for (size_t bus = 0; bus <= endBus; bus++)
	{
		for (size_t dev = 0; dev <= PCI_MAX_DEVICE; dev++)
		{
			UINT64 address0 = mcfg->BaseAddress + ((bus - startBus) << 20 | dev << 15);
			PCI_DEVICE_INDEPENDENT_REGION* header0 = (PCI_DEVICE_INDEPENDENT_REGION*)address0;
			size_t funcCount = header0->HeaderType & HEADER_TYPE_MULTI_FUNCTION ? PCI_MAX_FUNC + 1 : 1;
			for (size_t func = 0; func < funcCount; func++)
			{
				UINT64 address = mcfg->BaseAddress + ((bus - startBus) << 20 | dev << 15 | func << 12);
				PCI_DEVICE_INDEPENDENT_REGION* header = (PCI_DEVICE_INDEPENDENT_REGION*)address;

				if (header->VendorId == 0xFFFF) continue;

				if (header->HeaderType != HEADER_TYPE_DEVICE && header->HeaderType != HEADER_TYPE_MULTI_FUNCTION)
					continue;

				if (header->ClassCode[2] == classCode[0] &&
					header->ClassCode[1] == classCode[1] &&
					header->ClassCode[0] == classCode[2])
					return (PCI_TYPE00*)header;
			}
		}
	}

	kassert(!"Device not found.");
}

EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_SPACE_ACCESS_TABLE* FindMCFG(EFI_ACPI_DESCRIPTION_HEADER* xsdt)
{
	size_t entries = (xsdt->Length - sizeof(EFI_ACPI_DESCRIPTION_HEADER)) / sizeof(UINT64);

	UINT64* entryBegin = (UINT64*)(xsdt + 1);
	for (size_t i = 0; i < entries; i++)
	{
		EFI_ACPI_DESCRIPTION_HEADER* header = (EFI_ACPI_DESCRIPTION_HEADER*)entryBegin[i];
		if (header->Signature == EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_SPACE_ACCESS_TABLE_SIGNATURE)
			return (EFI_ACPI_MEMORY_MAPPED_CONFIGURATION_SPACE_ACCESS_TABLE*)header;
	}

	kassert(!"MCFG not found.");
}

void AcpiDriver::Install()
{
	auto xsdt = reinterpret_cast<EFI_ACPI_DESCRIPTION_HEADER*>(rsdp_->XsdtAddress);
	auto mcfg = FindMCFG(xsdt);

	g_BootVideo->PutFormat(L"PCI Config Address: %lx\n", mcfg->Configuration.BaseAddress);

	UINT64 startBus = mcfg->Configuration.StartBusNumber;
	UINT64 endBus = mcfg->Configuration.EndBusNumber;
	for (size_t bus = 0; bus <= endBus; bus++)
	{
		for (size_t dev = 0; dev <= PCI_MAX_DEVICE; dev++)
		{
			UINT64 address0 = mcfg->Configuration.BaseAddress + ((bus - startBus) << 20 | dev << 15);
			PCI_DEVICE_INDEPENDENT_REGION* header0 = (PCI_DEVICE_INDEPENDENT_REGION*)address0;
			size_t funcCount = header0->HeaderType & HEADER_TYPE_MULTI_FUNCTION ? PCI_MAX_FUNC + 1 : 1;
			for (size_t func = 0; func < funcCount; func++)
			{
				UINT64 address = mcfg->Configuration.BaseAddress + ((bus - startBus) << 20 | dev << 15 | func << 12);
				PCI_DEVICE_INDEPENDENT_REGION* header = (PCI_DEVICE_INDEPENDENT_REGION*)address;
				if (header->VendorId == 0xFFFF) continue;

				pciDevices_.emplace_back(bus, dev, func, header);
			}
		}
	}

	for (auto& device : pciDevices_)
	{
		auto driver = device.TryLoadDriver();
		if (driver)
			g_DeviceMgr->InstallDriver(std::move(driver));
	}
}
