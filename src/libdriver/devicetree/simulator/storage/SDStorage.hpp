//
// Kernel Device
//
#pragma once

#include <libdriver/devicetree/Fdt.hpp>

namespace Chino
{
namespace Device
{
    class SDStorageDriver : public Driver
    {
    public:
        DECLARE_FDT_DRIVER(SDStorageDriver);

        virtual void Install() override;

    private:
        const FDTDevice& device_;
    };
}
}
