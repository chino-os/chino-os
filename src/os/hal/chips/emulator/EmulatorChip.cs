// Copyright (c) SunnyCase. All rights reserved.
// Licensed under the Apache license. See LICENSE file in the project root for full license information.

using System;
using System.Collections.Generic;
using System.Text;

namespace Chino.Chips;

public sealed class EmulatorChip : ChipDefinition
{
    public EmulatorChip()
    {
        SimpleDeviceNode host_console;

        Machine = new()
        {
            Name = Name,
            Devices =
            {
                (host_console = new SimpleDeviceNode
                {
                    Name = "host_console",
                    Compatibles = [DeviceCompatibles.HostConsole],
                }),
                new SimpleDeviceNode
                {
                    Name = "host_fs",
                    Compatibles = [DeviceCompatibles.HostFS],
                },
                new SimpleDeviceNode
                {
                    Name = "host_serial",
                    Compatibles = [DeviceCompatibles.HostSerial],
                    Properties =
                    {
                        { "PortName", "COM5" },
                    },
                },

                SystemDevices.StreamConsole(host_console),
            },
        };
    }

    public override Guid Id { get; } = new Guid("67F7B818-643A-43CB-A95F-BC7E1ABFBFD9");

    public override string Name { get; } = "Emulator";

    public override string Vendor { get; } = Vendors.Chino;

    public static class DeviceCompatibles
    {
        public static readonly Guid HostConsole = new Guid("ABA5BEB4-E5B2-4B44-A07E-969ECEA00EA6");
        public static readonly Guid HostFS = new Guid("C6F17CB5-7D0F-4DE4-AFFA-D0496495A0ED");
        public static readonly Guid HostNetIf = new Guid("59DBBA5D-ED63-4962-B597-688E9447A23B");
        public static readonly Guid HostSerial = new Guid("8D460A89-F506-4A60-8F68-04C18E6F97ED");
    }
}
