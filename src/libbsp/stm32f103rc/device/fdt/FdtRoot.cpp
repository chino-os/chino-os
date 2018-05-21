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

using namespace Chino::Device;

class FdtRootDriver : public Driver
{
public:
	virtual void Install() override
	{
		g_Logger->PutFormat("fdt: %z\n", g_fdtSize);
		int depth = 0;
		auto first_node = fdt_next_node(g_fdtData, -1, &depth);
		if (first_node >= 0)
			ForeachNode(g_fdtData, first_node, depth);

		for (auto& device : fdtDevices_)
		{
			auto driver = device.TryLoadDriver();
			if (driver)
				g_DeviceMgr->InstallDriver(std::move(driver));
		}
	}
private:
	void ForeachNode(const void* fdt, int node, int depth)
	{
		fdtDevices_.emplace_back(fdt, node, depth);
		int subnode;
		fdt_for_each_subnode(subnode, fdt, node)
		{
			ForeachNode(fdt, subnode, fdt_node_depth(fdt, subnode));
		}
	}
private:
	std::vector<FDTDevice> fdtDevices_;
};

std::unique_ptr<Driver> Chino::Device::BSPInstallRootDriver(const BootParameters& bootParams)
{
	return std::make_unique<FdtRootDriver>();
}
