// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.

using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Drivers;

public class HostConsoleDriver : DriverDefinition
{
    public override Guid Id { get; } = new Guid("9B836279-3E8F-4921-BA04-4A4A27F64336");

    public override string Name { get; } = "host_console";

    public override string Path { get; } = "serial/host-console";

    public override IReadOnlyList<Guid> Compatibles { get; } = [Chips.EmulatorChip.DeviceCompatibles.HostConsole];
}
