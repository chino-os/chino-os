﻿// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.

using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Drivers;

public class StreamStdioDriver : DriverDefinition
{
    public override Guid Id { get; } = new Guid("EEEE73A8-E2FF-42F2-9140-C54CF4424948");

    public override string Name { get; } = "stream_stdio";

    public override string Path { get; } = "stdio/stream_stdio";

    public override IReadOnlyList<Guid> Compatibles { get; } = [Chips.SystemDevices.DeviceCompatibles.StreamStdio];
}
