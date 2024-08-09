// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
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

    [Description("Build root.")]
    [CommandArgument(1, "<buildRoot>")]
    public string BuildRoot { get; set; } = null!;
}

public class GenerateCommand : AsyncCommand<GenerateSettings>
{
    public override async Task<int> ExecuteAsync(CommandContext context, GenerateSettings settings)
    {
        var board = Program.GetBoard(settings.BoardName);
        AnsiConsole.WriteLine($"Generate info:");
        AnsiConsole.WriteLine($"Board: {settings.BoardName}");

        var directory = Path.Combine(settings.BuildRoot, "gen", "chino", "conf");
        if (!Directory.Exists(settings.BuildRoot))
        {
            AnsiConsole.WriteLine($"{settings.BuildRoot} doesn't exists.");
            return -1;
        }

        Directory.CreateDirectory(directory);

        await Generate(directory, "board_desc.h", board);
        await Generate(directory, "board_init.inl", board);
        await Generate(directory, "drivers.cmake", board);
        return 0;
    }

    private async Task Generate(string directory, string fileName, BoardDefinition board)
    {
        AnsiConsole.WriteLine($"Generate {fileName}...");
        var content = await RazorTemplateEngine.RenderAsync($"~/Templates/{fileName}.cshtml", board);
        await File.WriteAllTextAsync(Path.Combine(directory, fileName), content);
    }
}
