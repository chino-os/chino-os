using System;
using System.Collections.Generic;
using System.CommandLine;
using System.CommandLine.Invocation;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Chino.Board;
using Chino.Chip;
using Newtonsoft.Json;

namespace Chino
{
    class RenderOptions
    {
        public Guid Board { get; set; }

        public string Template { get; set; }

        public string Out { get; set; }
    }

    class Program
    {
        static Task<int> Main(string[] args)
        {
            var renderCmd = new Command("render", "render a template")
            {
                new Option<Guid>("--board", "board id") { Required = true },
                new Option<string>("--template", "template file") { Required = true },
                new Option<string>("--out", "output file") { Required = true }
            };
            renderCmd.Handler = CommandHandler.Create<RenderOptions>(OnRender);

            var rootCmd = new RootCommand()
            {
                renderCmd
            };

            return rootCmd.InvokeAsync(args);
        }

        private static async Task OnRender(RenderOptions options)
        {
            var engine = new RazorLight.RazorLightEngineBuilder()
                .UseMemoryCachingProvider()
                .UseFileSystemProject(Path.GetDirectoryName(options.Template), ".razor")
                .Build();
            var boards = GetBoardDefinitions();
            var chips = GetChipDefinitions();
            var board = boards.FirstOrDefault(x => x.Id == options.Board) ?? throw new InvalidOperationException($"Cannot find board definition for {options.Board}");
            var chip = chips.FirstOrDefault(x => x.Id == board.Chip) ?? throw new InvalidOperationException($"Cannot find chip definition for {board.Chip}");
            var context = new TemplateRenderContext { Chip = chip, Board = board };
            var result = await engine.CompileRenderAsync(Path.GetFileName(options.Template), context);
            File.WriteAllText(options.Out, result);
        }

        static IReadOnlyCollection<ChipDefinition> GetChipDefinitions()
        {
            var cdpType = typeof(IChipDefinitionProvider);
            return (from t in typeof(chipdef.Module).Assembly.ExportedTypes
                    where !t.IsAbstract && t.IsClass && t.GetInterfaces().Contains(cdpType)
                    let chips = ((IChipDefinitionProvider)Activator.CreateInstance(t)).Chips
                    from c in chips
                    select c).ToList();
        }

        static IReadOnlyCollection<BoardDefinition> GetBoardDefinitions()
        {
            var bdpType = typeof(IBoardDefinitionProvider);
            return (from t in typeof(boarddef.Module).Assembly.ExportedTypes
                    where !t.IsAbstract && t.IsClass && t.GetInterfaces().Contains(bdpType)
                    let boards = ((IBoardDefinitionProvider)Activator.CreateInstance(t)).Boards
                    from b in boards
                    select b).ToList();
        }
    }
}
