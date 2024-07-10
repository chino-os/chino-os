// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.

using System;
using System.Collections.Generic;
using System.Text;

namespace Chino;

public record struct RegRange(ulong Start, UnitsNet.Information Size)
{
}

public record class MemoryRange(string Name, string Attribute, ulong Start, UnitsNet.Information Size)
{
}

public class PinBank
{
    public string Name { get; set; }

    public uint PinsCount { get; set; }

    public DeviceNode Device { get; set; }
}

public struct PinRef
{
    public PinBank Bank { get; set; }

    public uint Index { get; set; }

    public PinRef(PinBank bank, uint index)
    {
        Bank = bank;
        Index = index;
    }
}

public class PinFunction
{
    public string Name { get; set; }

    public PinRef Pin { get; set; }
}

public class PinGroup
{
    public string Name { get; set; }

    public IReadOnlyList<PinFunction> Pins { get; set; } = Array.Empty<PinFunction>();
}

public class DeviceNode
{
    public string Name { get; set; }

    public IReadOnlyList<Guid> Compatibles { get; set; } = Array.Empty<Guid>();

    public IReadOnlyList<RegRange> Regs { get; set; } = Array.Empty<RegRange>();

    public IReadOnlyList<PinGroup> PinGroups { get; set; } = Array.Empty<PinGroup>();

    public Dictionary<string, object> Properties { get; set; } = new();
}

public interface IHasChildrenDevices
{
    IReadOnlyList<DeviceNode> Children { get; }
}

public interface IHasCompatible
{
    IReadOnlyList<Guid> Compatibles { get; }
}

public class MachineNode : DeviceNode, IHasChildrenDevices
{
    public List<DeviceNode> Devices { get; set; } = new();

    IReadOnlyList<DeviceNode> IHasChildrenDevices.Children => Devices;
}

public class SimpleBusNode : DeviceNode, IHasChildrenDevices
{
    public IReadOnlyList<DeviceNode> Children { get; set; } = Array.Empty<DeviceNode>();
}

public class SimpleDeviceNode : DeviceNode, IHasCompatible
{
}

public abstract class ChipDefinition
{
    public abstract Guid Id { get; }

    public abstract string Vendor { get; }

    public abstract string Name { get; }

    public IReadOnlyList<MemoryRange> Memories { get; set; } = Array.Empty<MemoryRange>();

    public IReadOnlyList<PinBank> PinBanks { get; set; } = Array.Empty<PinBank>();

    public IReadOnlyList<PinGroup> PinGroups { get; set; } = Array.Empty<PinGroup>();

    public MachineNode Machine { get; set; } = new MachineNode();
}
