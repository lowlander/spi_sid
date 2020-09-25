/*
 * Copyright (c) 2020 Erwin Rol <erwin@erwinrol.com
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SID_SPI_H
#define SID_SPI_H

#include <stdint.h>

void sid_spi_transfer( uint8_t cmd_addr, uint8_t wr_data,
                       uint8_t* status, uint8_t* rd_data);
int sid_spi_init(void);

#endif /* SID_SPI_H */
