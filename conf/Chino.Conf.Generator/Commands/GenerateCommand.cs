// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Razor.Templating.Core;
using Spectre.Console;
using Spectre.Console.Cli;

namespace Chino.Conf.Generator.Commands;

public class GenerateSettings : CommandSettings
{
    [Description("Board name.")]
    [CommandArgument(0, "<boardName>")]
    public string BoardName { get; set; } = null!;

    [Description("Project root.")]
    [CommandArgument(0, "<projectRoot>")]
    public string ProjectRoot { get; set; } = null!;
}

public class GenerateCommand : AsyncCommand<GenerateSettings>
{
    public override async Task<int> ExecuteAsync(CommandContext context, GenerateSettings settings)
    {
        var board = Program.GetBoard(settings.BoardName);
        AnsiConsole.WriteLine($"Generate board_desc.h for {settings.BoardName}...");
        var content = await RazorTemplateEngine.RenderAsync("~/Templates/board_desc.h.cshtml", board);
        var directory = Path.Combine(settings.ProjectRoot, "build", "conf");
        if (!Directory.Exists(settings.ProjectRoot))
        {
            AnsiConsole.WriteLine($"{settings.ProjectRoot} doesn't exists.");
            return -1;
        }

        Directory.CreateDirectory(directory);
        await File.WriteAllTextAsync(Path.Combine(directory, "board_desc.h"), content);
        return 0;
    }
}
