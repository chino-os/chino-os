// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.

using System;
using System.Collections.Generic;
using System.Text;
using Chino.Chips;
using Chino.Drivers;

namespace Chino.Boards;

[Board("Emulator")]
public class EmulatorBoard : BoardDefinition
{
    private readonly bool _useHostSocket = true;

    public EmulatorBoard()
    {
        Drivers = new()
        {
            new HostConsoleDriver(),
            new HostFSDriver(),
            new HostSerialDriver(),
            _useHostSocket ? new HostSocketDriver() : new LwipSocketDriver(),
            new HostBleDriver(),

            new StreamStdioDriver(),
        };
    }

    public override Guid Id { get; } = new Guid("26B54718-A4C6-4D9A-AE35-8F3C8BF8E7AE");

    public override ChipDefinition Chip { get; } = new EmulatorChip();
}
