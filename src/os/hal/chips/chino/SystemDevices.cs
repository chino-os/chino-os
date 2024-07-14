// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Chino.Chips;

public static class SystemDevices
{
    public static SimpleDeviceNode StreamConsole(SimpleDeviceNode streamDevice) =>
        new SimpleDeviceNode
        {
            Name = "stream_console",
            Compatibles = [DeviceCompatibles.StreamConsole],
            ReferencedDevices =
            {
                streamDevice,
            },
        };

    public static class DeviceCompatibles
    {
        public static readonly Guid StreamConsole = new Guid("C2DA89E4-74D9-40E1-921E-E78FAEF8AED0");
    }
}
