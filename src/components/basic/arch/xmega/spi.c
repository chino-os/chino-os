#include <avr/io.h>
#include "spi.h"

#define SPI_SS_bm     0x10
#define SPI_MOSI_bm   0x20
#define SPI_MISO_bm   0x40
#define SPI_SCK_bm    0x80

#define FOO           0x00

void spi_init(void)
{
  PORTC.DIRSET = SPI_SCK_bm | SPI_MOSI_bm | SPI_SS_bm;
  PORTC.DIRCLR = SPI_MISO_bm;
  PORTC.OUTSET = SPI_SS_bm;
  spi_fast();
}

void spi_fast(void)
{
  SPIC.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_0_gc | SPI_PRESCALER_DIV4_gc;
}

void spi_slow(void)
{
  SPIC.CTRL = SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_0_gc | SPI_PRESCALER_DIV128_gc;
}

uint8_t spi_transfer(uint8_t data)
{
  SPIC.DATA = data;
  while(!(SPIC.STATUS & (SPI_IF_bm)));
  return SPIC.DATA;
}

void spi_write(uint8_t data)
{
  // PORTC.OUTCLR = SPI_SS_bm;
  spi_transfer(data);
  // PORTC.OUTSET = SPI_SS_bm;
}

uint8_t spi_read(void)
{
  uint8_t data;

  // PORTC.OUTCLR = SPI_SS_bm;
  data = spi_transfer(FOO);
  // PORTC.OUTSET = SPI_SS_bm;

  return data;
}

void spi_cs_high(void)
{
  PORTC.OUTSET = SPI_SS_bm;
  // PORTC.OUTCLR = SPI_SS_bm;
}

void spi_cs_low(void)
{
  PORTC.OUTCLR = SPI_SS_bm;
 //  PORTC.OUTSET = SPI_SS_bm;
}
