// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.

using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Drivers;

public class HostBleDriver : DriverDefinition
{
    public override Guid Id { get; } = new Guid("BCC8FABE-F11B-4CE9-A691-0FC9D54D9F18");

    public override string Name { get; } = "host_ble";

    public override string Path { get; } = "ble/host_ble";

    public override IReadOnlyList<Guid> Compatibles { get; } = [Chips.EmulatorChip.DeviceCompatibles.HostBle];
}
