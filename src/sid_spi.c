/*
 * Copyright (c) 2020 Erwin Rol <erwin@erwinrol.com
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "sid_spi.h"

#include <errno.h>
#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/spi.h>
#include <soc.h>

static const struct device *gpioa;
static const struct device *gpiob;
static const struct device *gpioc;

static const struct device *spi;

static struct spi_config       spi_cfg;
static struct spi_cs_control   spi_cs;

void sid_spi_transfer( uint8_t cmd_addr, uint8_t wr_data,
                       uint8_t* status, uint8_t* rd_data)
{
  int res;

  uint8_t wr_buffer[2];
  uint8_t rd_buffer[2];

  wr_buffer[0] = cmd_addr;
  wr_buffer[1] = wr_data;

  struct spi_buf wr_bufs[] = {
    {
      .buf = &wr_buffer,
      .len = 2,
    },
  };

  struct spi_buf rd_bufs[] = {
    {
      .buf = rd_buffer,
      .len = 2,
    },
  };

  struct spi_buf_set tx = {
    .buffers = wr_bufs,
    .count = 1,
  };

  struct spi_buf_set rx = {
    .buffers = rd_bufs,
    .count = 1,
  };

  res = spi_transceive(spi, &spi_cfg, &tx, &rx);

  *status = rd_buffer[0];
  *rd_data = rd_buffer[1];
}

int sid_spi_init(void)
{
  int err;

  gpioa = device_get_binding("GPIOA");
  if (!gpioa) {
    return -1;
  }

  gpiob = device_get_binding("GPIOB");
  if (!gpiob) {
    return -1;
  }

  gpioc = device_get_binding("GPIOC");
  if (!gpioc) {
    return -1;
  }

  gpio_pin_configure(gpioc, 7, GPIO_OUTPUT | GPIO_ACTIVE_LOW);
  gpio_pin_set_raw(gpioc, 7, 1);

  gpio_pin_configure(gpioa, 9, GPIO_OUTPUT | GPIO_ACTIVE_LOW);
  gpio_pin_set_raw(gpioa, 9, 1);

  gpio_pin_configure(gpiob, 6, GPIO_OUTPUT | GPIO_ACTIVE_LOW);
  gpio_pin_set_raw(gpiob, 6, 1);

  spi = device_get_binding("SPI_1");
  if (!spi) {
    return -1;
  }

  spi_cs.delay = 0;
  spi_cs.gpio_dev = gpiob;
  spi_cs.gpio_pin = 6;
  spi_cs.gpio_dt_flags = GPIO_ACTIVE_LOW;

  spi_cfg.slave = 0;
  spi_cfg.operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8);
  spi_cfg.frequency = 10000000U;
  spi_cfg.cs = &spi_cs;

  return 0;
}
