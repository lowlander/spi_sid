/*
 * Copyright (c) 2020 Erwin Rol <erwin@erwinrol.com
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <errno.h>
#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/spi.h>
#include <soc.h>
#include <stdint.h>

#include "c64.h"
#include "sid_spi.h"
#include "sid.h"
#include "sid_file.h"

volatile int n_refresh_cia;

K_TIMER_DEFINE(sid_timer, NULL, NULL);

void main(void)
{
  uint8_t status;
  uint8_t rd_data;

  if (sid_spi_init() < 0) {
    goto error_out;
  }

  for (uint8_t addr = 0; addr < 0x19; addr++) {
    sid_poke( addr, 0);
  }

  c64_init();

  struct sid_info info;
  sid_load_from_memory(sid_file, sid_file_size, &info);

  sid_poke(24, 15);
  c64_cpu_jsr(info.init_addr, 0);

  k_timer_start(&sid_timer, K_MSEC(20), K_MSEC(20));

  while (1) {
    k_timer_status_sync(&sid_timer);

    c64_cpu_jsr(info.play_addr, 0);

    n_refresh_cia = (int)(20000 * (c64_getmem(0xdc04) | (c64_getmem(0xdc05) << 8)) / 0x4c00);
  }

error_out:
  while(1) {
    k_sleep(K_MSEC(10));
  }
}
