using System;
using System.Collections.Generic;
using System.CommandLine;
using System.CommandLine.Invocation;
using System.IO;
using System.Linq;
using System.Threading.Tasks;
using Chino.Chip;
using Newtonsoft.Json;
using Scriban;

namespace Chino
{
    class BoardConfig
    {
        public Guid Chip { get; set; }
    }

    class RenderOptions
    {
        public string Cfg { get; set; }

        public string Template { get; set; }

        public string Out { get; set; }
    }

    class Program
    {
        static int Main(string[] args)
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

            return rootCmd.Invoke(args);
        }

        private static void OnRender(RenderOptions options)
        {
            var chips = GetChipDefinitions();
            var config = JsonConvert.DeserializeObject<BoardConfig>(File.ReadAllText(options.Cfg));
            var chip = chips.FirstOrDefault(x => x.Id == config.Chip) ?? throw new InvalidOperationException($"Cannot find chip definition for {config.Chip}");
            var context = new TemplateRenderContext { Chip = chip };
            var template = Template.ParseLiquid(File.ReadAllText(options.Template));
            File.WriteAllText(options.Out, template.Render(context));
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
