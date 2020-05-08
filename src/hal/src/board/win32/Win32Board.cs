using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Board
{
    public class Win32Board : IBoardDefinitionProvider
    {
        public static readonly Guid Win32 = new Guid("26B54718-A4C6-4D9A-AE35-8F3C8BF8E7AE");

        private readonly List<BoardDefinition> _boards = new List<BoardDefinition>();

        public IReadOnlyCollection<BoardDefinition> Boards => _boards;

        public Win32Board()
        {
            _boards.Add(new BoardDefinition
            {
                Id = Win32,
                Chip = Chip.Win32Chip.Win32
            });
        }
    }
}
