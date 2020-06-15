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
                Chip = Chip.Win32Chip.Win32,
                Drivers = new[]
                {
                    Driver.Win32ConsoleDriver.Win32Console,
                    Driver.Win32FSDriver.Win32FS,
                    Driver.Win32NetIfDriver.Win32NetIf
                }
            });
        }
    }
}
