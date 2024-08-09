// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.

using System;
using System.Collections.Generic;
using System.Text;
using Chino.Chips;

namespace Chino.Drivers;

public class LwipSocketDriver : DriverDefinition
{
    public override Guid Id { get; } = new Guid("8AAB4C14-E4EA-4288-A1F3-3D906F4832EB");

    public override string Name { get; } = "lwip_socket";

    public override string Path { get; } = "socket/lwip_socket";

    public override IReadOnlyList<Guid> Compatibles { get; } = [SystemDevices.DeviceCompatibles.LwipSocket];
}
