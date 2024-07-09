using System;
using System.Collections.Generic;
using System.Text;

namespace Chino
{
    public class MemoryRange
    {
        public string Name { get; set; }

        public string Attribute { get; set; }

        public ulong Start { get; set; }

        public UnitsNet.Information Length { get; set; }
    }

    public struct RegRange
    {
        public ulong Start { get; set; }

        public UnitsNet.Information Length { get; set; }
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

        public IReadOnlyDictionary<string, object> Properties { get; set; } = new Dictionary<string, object>();
    }

    public interface IHasChildrenDevices
    {
        IReadOnlyList<DeviceNode> Children { get; }
    }

    public interface IHasCompatible
    {
        IReadOnlyList<Guid> Compatible { get; }
    }

    public class MachineNode : DeviceNode, IHasChildrenDevices
    {
        public IReadOnlyList<DeviceNode> Devices { get; set; } = Array.Empty<DeviceNode>();

        IReadOnlyList<DeviceNode> IHasChildrenDevices.Children => Devices;
    }

    public class SimpleBusNode : DeviceNode, IHasChildrenDevices
    {
        public IReadOnlyList<DeviceNode> Children { get; set; } = Array.Empty<DeviceNode>();
    }

    public class SimpleDeviceNode : DeviceNode, IHasCompatible
    {
        public IReadOnlyList<Guid> Compatible { get; set; } = Array.Empty<Guid>();

        public IReadOnlyList<RegRange> Regs { get; set; } = Array.Empty<RegRange>();

        public IReadOnlyList<PinGroup> PinGroups { get; set; } = Array.Empty<PinGroup>();
    }

    public class ChipDefinition
    {
        public Guid Id { get; set; }

        public string Vendor { get; set; }

        public string Name { get; set; }

        public IReadOnlyList<MemoryRange> Memories { get; set; } = Array.Empty<MemoryRange>();

        public IReadOnlyList<PinBank> PinBanks { get; set; } = Array.Empty<PinBank>();

        public IReadOnlyList<PinGroup> PinGroups { get; set; } = Array.Empty<PinGroup>();

        public MachineNode Root { get; set; }
    }

    public interface IChipDefinitionProvider
    {
        IReadOnlyCollection<ChipDefinition> Chips { get; }
    }
}
