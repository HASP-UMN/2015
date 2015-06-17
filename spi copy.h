#ifndef _SPI_H_
#define _SPI_H_

#include <stdint.h>

inline int file_valid(const char* path);
int spi_init(const char *device, uint8_t mode, uint8_t bpw, uint32_t speed);
int spi_transfer(int spi_handle, uint8_t *tx, uint8_t *rx, uint32_t len);
int spi_free (int spi_handle);

#endi