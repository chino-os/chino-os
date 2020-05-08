using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Board
{
    public class BoardDefinition
    {
        public Guid Id { get; set; }

        public Guid Chip { get; set; }

        public Dictionary<string, string> SelectedPinGroups { get; set; }
    }

    public interface IBoardDefinitionProvider
    {
        IReadOnlyCollection<BoardDefinition> Boards { get; }
    }
}
