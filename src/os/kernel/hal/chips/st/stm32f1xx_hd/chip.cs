using System;
using System.Collections.Generic;
using UnitsNet.NumberExtensions.NumberToInformation;

namespace Chino.Chip
{
    public class STM32F1XX_HD : IChipDefinitionProvider
    {
        public static readonly Guid STM32F103ZET6 = new Guid("03A3F064-FFBA-4DE2-BAE2-51FD3AC8E11D");

        private readonly List<ChipDefinition> _chips = new List<ChipDefinition>();

        public IReadOnlyCollection<ChipDefinition> Chips => _chips;

        public STM32F1XX_HD()
        {
            var gpioA = new SimpleDeviceNode { Name = "GpioA", Regs = new[] { new RegRange { Start = 0x40010800, Length = 0x400.Bytes() } } };
            var portA = new PinBank { Name = "PortA", PinsCount = 16, Device = gpioA };

            var pgUsart1A = new PinGroup
            {
                Name = "Usart1-A",
                Pins = new[]
                {
                    new PinFunction { Name = "TX", Pin = new PinRef(portA, 9) },
                    new PinFunction { Name = "RX", Pin = new PinRef(portA, 10) }
                }
            };

            _chips.Add(new ChipDefinition
            {
                Id = STM32F103ZET6,
                Vendor = "ST",
                Name = "STM32F103ZET6",
                Memories = new[]
                {
                    new MemoryRange { Name = "FLASH", Attribute = "rx", Start = 0x08000000, Length = 512.Kibibytes() },
                    new MemoryRange { Name = "RAM", Attribute = "rw", Start = 0x20000000, Length = 64.Kibibytes() }
                },
                PinBanks = new[] { portA },
                PinGroups = new[]
                {
                    pgUsart1A
                },
                Root = new MachineNode
                {
                    Name = "STM32F103ZET6",
                    Devices = new[]
                    {
                        new SimpleBusNode
                        {
                            Name = "soc",
                            Children = new[]
                            {
                                gpioA,

                                new SimpleDeviceNode
                                {
                                    Name = "Rcc",
                                    Regs = new[] { new RegRange { Start = 0x40021800, Length = 0x400.Bytes() } },
                                },
                                new SimpleDeviceNode
                                {
                                    Name = "Usart1",
                                    Compatible = new[] { DeviceCompatibles.Usart },
                                    Regs = new[] { new RegRange { Start = 0x40013800, Length = 0x400.Bytes() } },
                                    PinGroups = new[] { pgUsart1A }
                                }
                            }
                        }
                    }
                }
            });
        }

        public static class DeviceCompatibles
        {
            public static readonly Guid Usart = new Guid("13AD5373-AB75-4C57-B991-7EE00075F48D");
        }
    }
}
