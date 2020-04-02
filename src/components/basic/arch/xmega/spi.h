#ifndef __SPI_H__
#define __SPI_H__

void spi_init(void);
uint8_t spi_transfer(uint8_t data);
void spi_write(uint8_t data);
uint8_t spi_read(void);
void spi_cs_high(void);
void spi_cs_low(void);
void spi_fast(void);
void spi_slow(void);

#endif // __SPI_H__
