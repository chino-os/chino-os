// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.

using System;
using System.Collections.Frozen;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Threading.Tasks;
using Chino.Conf.Generator.Commands;
using Spectre.Console.Cli;

namespace Chino.Conf.Generator;

public static class Program
{
    private static readonly IReadOnlyDictionary<string, Type> _builtinBoards = GetBoardDefinitions();

    public static int Main(string[] args)
    {
        var app = new CommandApp();

        app.Configure(config =>
        {
            config.AddCommand<GenerateCommand>("gen");
        });

        return app.Run(args);
    }

    public static BoardDefinition GetBoard(string boardName) => (BoardDefinition)Activator.CreateInstance(_builtinBoards[boardName])!;

    private static IReadOnlyDictionary<string, Type> GetBoardDefinitions()
    {
        return (from t in typeof(Boarddef.Module).Assembly.ExportedTypes
                where !t.IsAbstract && t.IsClass
                let attr = t.GetCustomAttribute<BoardAttribute>()
                where attr != null
                select new { Name = attr.Name, Type = t }).ToFrozenDictionary(x => x.Name, x => x.Type, StringComparer.InvariantCultureIgnoreCase);
    }
}
