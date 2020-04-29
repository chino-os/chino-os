using System;
using System.Collections.Generic;
using System.Text;

namespace Chino
{
    public class TemplateRenderContext
    {
        public Chip.ChipDefinition Chip { get; set; }

        public BoardConfig Board { get; set; }
    }
}
