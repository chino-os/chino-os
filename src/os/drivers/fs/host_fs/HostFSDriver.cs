// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.

using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Drivers;

public class HostFSDriver : DriverDefinition
{
    public override Guid Id { get; } = new Guid("839E1344-6055-4A72-9E95-912FB58161AD");

    public override string Name { get; } = "host_fs";

    public override string Path { get; } = "fs/host_fs";

    public override IReadOnlyList<Guid> Compatibles { get; } = [Chips.EmulatorChip.DeviceCompatibles.HostFS];
}
