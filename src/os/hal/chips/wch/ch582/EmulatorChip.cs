// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.

using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Chips.WCH;

public sealed class CH582Chip : ChipDefinition
{
    public CH582Chip()
    {
        Machine = new()
        {
            Name = Name,
            Devices =
            {
            },
        };
    }

    public override Guid Id { get; } = new Guid("22DFEA91-B23F-4042-88A3-B3F4B332454D");

    public override string Name { get; } = "CH582";

    public override string Vendor { get; } = Vendors.WCH;

    public static class DeviceCompatibles
    {
    }
}
