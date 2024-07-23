using System;
using System.Collections.Generic;
using System.Text;
using Chino.Chips;
using Chino.Drivers;

namespace Chino.Boards;

[Board("Emulator")]
public class EmulatorBoard : BoardDefinition
{
    public EmulatorBoard()
    {
        Drivers = new()
        {
            new HostConsoleDriver(),
            new HostFSDriver(),
            new HostSerialDriver(),
            new StreamConsoleDriver(),
        };
    }

    public override Guid Id { get; } = new Guid("26B54718-A4C6-4D9A-AE35-8F3C8BF8E7AE");

    public override ChipDefinition Chip { get; } = new EmulatorChip();
}
