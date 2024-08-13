// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;

namespace Chino;

[AttributeUsage(AttributeTargets.Class, AllowMultiple = false)]
public sealed class BoardAttribute : Attribute
{
    public BoardAttribute(string name)
    {
        Name = name;
    }

    public string Name { get; }
}

public abstract class BoardDefinition
{
    public abstract Guid Id { get; }

    public abstract ChipDefinition Chip { get; }

    public Dictionary<string, string> SelectedPinGroups { get; set; }

    public List<DriverDefinition> Drivers { get; set; } = new();

    public virtual Task PrepareEnviromentAsync()
    {
        return Task.CompletedTask;
    }
}
