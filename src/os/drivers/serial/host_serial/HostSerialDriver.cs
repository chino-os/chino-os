// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.

using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Drivers;

public class HostSerialDriver : DriverDefinition
{
    public override Guid Id { get; } = new Guid("5ED2E20B-72C6-4C9F-B2D4-24E922AA58F0");

    public override string Name { get; } = "host_serial";

    public override string Path { get; } = "serial/host_serial";

    public override IReadOnlyList<Guid> Compatibles { get; } = [Chips.EmulatorChip.DeviceCompatibles.HostSerial];
}
