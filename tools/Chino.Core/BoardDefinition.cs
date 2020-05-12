using System;
using System.Collections.Generic;
using System.Text;

namespace Chino
{
    public class BoardDefinition
    {
        public Guid Id { get; set; }

        public Guid Chip { get; set; }

        public Dictionary<string, string> SelectedPinGroups { get; set; }

        public IReadOnlyList<Guid> Drivers { get; set; }
    }

    public interface IBoardDefinitionProvider
    {
        IReadOnlyCollection<BoardDefinition> Boards { get; }
    }
}
