using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Driver
{
    public class Win32NetIfDriver : IDriverDefinitionProvider
    {
        public static readonly Guid Win32NetIf = new Guid("DE617FBB-7EB9-4EC9-A176-A6C8F475A2C4");

        private readonly List<DriverDefinition> _drivers = new List<DriverDefinition>();

        public IReadOnlyCollection<DriverDefinition> Drivers => _drivers;

        public Win32NetIfDriver()
        {
            _drivers.Add(new DriverDefinition
            {
                Id = Win32NetIf,
                Name = "win32-netif",
                Compatible = new[] { Chip.Win32Chip.DeviceCompatibles.Win32NetIf },
                Path = "netif/win32-netif"
            });
        }
    }
}
