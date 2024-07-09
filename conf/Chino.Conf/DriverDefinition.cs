using System;
using System.Collections.Generic;
using System.Text;

namespace Chino
{
    public class DriverDefinition
    {
        public Guid Id { get; set; }

        public string Name { get; set; }

        public string Path { get; set; }

        public IReadOnlyList<Guid> Compatible { get; set; }
    }

    public interface IDriverDefinitionProvider
    {
        IReadOnlyCollection<DriverDefinition> Drivers { get; }
    }
}
