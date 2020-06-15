using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Chino
{
    public class TemplateRenderContext
    {
        public ChipDefinition Chip { get; set; }

        public BoardDefinition Board { get; set; }

        public IReadOnlyList<DriverDefinition> Drivers { get; set; }

        public IReadOnlyList<DeviceNode> FindCompatibleDevices(Guid driverId)
        {
            var driver = Drivers.First(x => x.Id == driverId);
            return (from d in FlattenDevices()
                    let c = d as IHasCompatible
                    where c != null && c.Compatible.Intersect(driver.Compatible).Any()
                    select d).ToList();
        }

        public IReadOnlyList<DeviceNode> FlattenDevices()
        {
            var visited = new HashSet<DeviceNode>();
            var nodes = new List<DeviceNode>();

            void Visit(DeviceNode node)
            {
                if (visited.Add(node))
                {
                    nodes.Add(node);

                    var hasChild = node as IHasChildrenDevices;
                    if (hasChild != null)
                    {
                        foreach (var child in hasChild.Children)
                            Visit(child);
                    }
                }
            }

            Visit(Chip.Root);
            return nodes;
        }
    }
}
