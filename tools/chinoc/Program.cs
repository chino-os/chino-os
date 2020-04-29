using System;
using System.Collections.Generic;
using System.CommandLine;
using System.CommandLine.Invocation;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Chino.Chip;
using Newtonsoft.Json;

namespace Chino
{
    public class BoardConfig
    {
        public Guid Chip { get; set; }

        public Dictionary<string, string> SelectedPinGroups { get; set; }
    }

    class RenderOptions
    {
        public string Cfg { get; set; }

        public string Template { get; set; }

        public string Out { get; set; }
    }

    class Program
    {
        static Task<int> Main(string[] args)
        {
            var renderCmd = new Command("render", "render a template")
            {
                new Option<string>("--cfg", "board config file") { Required = true },
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
            var chips = GetChipDefinitions();
            var config = JsonConvert.DeserializeObject<BoardConfig>(File.ReadAllText(options.Cfg));
            var chip = chips.FirstOrDefault(x => x.Id == config.Chip) ?? throw new InvalidOperationException($"Cannot find chip definition for {config.Chip}");
            var context = new TemplateRenderContext { Chip = chip, Board = config };
            var result = await engine.CompileRenderAsync(Path.GetFileName(options.Template), context);
            File.WriteAllText(options.Out, result);
        }

        static IReadOnlyCollection<ChipDefinition> GetChipDefinitions()
        {
            var cdpType = typeof(IChipDefinitionProvider);
            return (from t in typeof(Program).Assembly.ExportedTypes
                    where !t.IsAbstract && t.IsClass && t.GetInterfaces().Contains(cdpType)
                    let chips = ((IChipDefinitionProvider)Activator.CreateInstance(t)).Chips
                    from c in chips
                    select c).ToList();
        }
    }
}
