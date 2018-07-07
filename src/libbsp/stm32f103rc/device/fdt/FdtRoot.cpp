//
// Kernel Device
//
#include "Fdt.hpp"
#include <libbsp/bsp.hpp>
#include <kernel/kdebug.hpp>
#include <kernel/device/DeviceManager.hpp>
#include <incbin/incbin.h>
#include <libfdt/libfdt.h>

INCBIN(_fdt, "../../devicetree/stm32f103rc.dtb");

using namespace Chino;
using namespace Chino::Device;

class FdtRootDriver : public Driver
{
public:
	virtual void Install() override
	{
		g_Logger->PutFormat("fdt: %z\n", g_fdtSize);
		std::vector<ObjectPtr<FDTDevice>> fdtDevices;
		int depth = 0;
		auto first_node = fdt_next_node(g_fdtData, -1, &depth);
		if (first_node >= 0)
			ForeachNode(fdtDevices, g_fdtData, first_node, depth);

		for (auto& device : fdtDevices)
		{
			auto driver = device->TryLoadDriver();
			if (driver)
				g_DeviceMgr->InstallDriver(*driver);
		}
	}
private:
	void ForeachNode(std::vector<ObjectPtr<FDTDevice>>& fdtDevices, const void* fdt, int node, int depth)
	{
		fdtDevices.emplace_back(MakeObject<FDTDevice>(fdt, node, depth));
		int subnode;
		fdt_for_each_subnode(subnode, fdt, node)
		{
			ForeachNode(fdtDevices, fdt, subnode, fdt_node_depth(fdt, subnode));
		}
	}
};

Chino::ObjectPtr<Driver> Chino::Device::BSPInstallRootDriver(const BootParameters& bootParams)
{
	return MakeObject<FdtRootDriver>();
}
