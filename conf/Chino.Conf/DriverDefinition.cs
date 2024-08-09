// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.

using System;
using System.Collections.Generic;
using System.Text;

namespace Chino;

public abstract class DriverDefinition
{
    public abstract Guid Id { get; }

    public abstract string Name { get; }

    public abstract string Path { get; }

    public abstract IReadOnlyList<Guid> Compatibles { get; }
}
