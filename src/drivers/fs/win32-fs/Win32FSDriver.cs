using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Driver
{
    public class Win32FSDriver : IDriverDefinitionProvider
    {
        public static readonly Guid Win32FS = new Guid("09719CD9-29F1-48B6-9590-33BFD56C3506");

        private readonly List<DriverDefinition> _drivers = new List<DriverDefinition>();

        public IReadOnlyCollection<DriverDefinition> Drivers => _drivers;

        public Win32FSDriver()
        {
            _drivers.Add(new DriverDefinition
            {
                Id = Win32FS,
                Name = "win32-fs",
                Compatible = new[] { Chip.Win32Chip.DeviceCompatibles.Win32FS },
                Path = "fs/win32-fs"
            });
        }
    }
}
