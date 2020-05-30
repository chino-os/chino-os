using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Chip
{
    public class Win32Chip : IChipDefinitionProvider
    {
        public static readonly Guid Win32 = new Guid("67F7B818-643A-43CB-A95F-BC7E1ABFBFD9");

        private readonly List<ChipDefinition> _chips = new List<ChipDefinition>();

        public IReadOnlyCollection<ChipDefinition> Chips => _chips;

        public Win32Chip()
        {
            _chips.Add(new ChipDefinition
            {
                Id = Win32,
                Vendor = "Chino",
                Name = "Win32",
                Root = new MachineNode
                {
                    Name = "Win32",
                    Devices = new[]
                    {
                        new SimpleDeviceNode
                        {
                            Name = "Console",
                            Compatible = new[] { DeviceCompatibles.Console }
                        },
                        new SimpleDeviceNode
                        {
                            Name = "Win32FS",
                            Compatible = new[] { DeviceCompatibles.Win32FS },
                            Properties = new Dictionary<string, object>
                            {
                                { "Path", "share" }
                            }
                        }
                    }
                }
            });
        }

        public static class DeviceCompatibles
        {
            public static readonly Guid Console = new Guid("ABA5BEB4-E5B2-4B44-A07E-969ECEA00EA6");
            public static readonly Guid Win32FS = new Guid("C6F17CB5-7D0F-4DE4-AFFA-D0496495A0ED");
        }
    }
}
