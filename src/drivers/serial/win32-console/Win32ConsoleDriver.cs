using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Driver
{
    public class Win32ConsoleDriver : IDriverDefinitionProvider
    {
        public static readonly Guid Win32Console = new Guid("9B836279-3E8F-4921-BA04-4A4A27F64336");

        private readonly List<DriverDefinition> _drivers = new List<DriverDefinition>();

        public IReadOnlyCollection<DriverDefinition> Drivers => _drivers;

        public Win32ConsoleDriver()
        {
            _drivers.Add(new DriverDefinition
            {
                Id = Win32Console,
                Name = "win32-console",
                Compatible = new[] { Chip.Win32Chip.DeviceCompatibles.Console },
                Path = "serial/win32-console"
            });
        }
    }
}
