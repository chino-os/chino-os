// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.

using System;
using System.Collections.Generic;
using System.Text;

namespace Chino;

public abstract class BoardDefinition
{
    public Guid Id { get; }

    public ChipDefinition Chip { get; }

    public Dictionary<string, string> SelectedPinGroups { get; set; }

    public IReadOnlyList<DriverDefinition> Drivers { get; set; }
}
