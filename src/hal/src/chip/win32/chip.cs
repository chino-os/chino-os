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
                    Name = "Win32"
                }
            });
        }
    }
}
