// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.

using System;
using System.Collections.Generic;
using System.Text;
using Chino.Chips;
using Chino.Chips.WCH;
using Chino.Drivers;

namespace Chino.Boards;

[Board("CH582")]
public class CH582Board : BoardDefinition
{
    public CH582Board()
    {
        Drivers = new()
        {
            new StreamStdioDriver(),
        };
    }

    public override Guid Id { get; } = new Guid("6B88E7F1-89F3-4507-958F-77FD1BA0B147");

    public override ChipDefinition Chip { get; } = new CH582Chip();
}
