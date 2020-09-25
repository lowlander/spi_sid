/*
 * Copyright (c) 2020 Erwin Rol <erwin@erwinrol.com
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "sid.h"
#include "c64.h"
#include "sid_spi.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>


void sid_poke(uint16_t reg, uint8_t val)
{
  uint8_t status;
  uint8_t rd_data;

  sid_spi_transfer(0x80 | reg, val, &status, &rd_data);
}

bool sid_load_from_memory(const uint8_t* data, size_t size, struct sid_info *info)
{
  if (!data || !size || !info)
    return false;

  unsigned char data_file_offset;

  data_file_offset = data[7];

  info->load_addr = data[8]<<8;
  info->load_addr|= data[9];

  info->init_addr = data[10]<<8;
  info->init_addr|= data[11];

  info->play_addr = data[12]<<8;
  info->play_addr|= data[13];

  info->subsongs = data[0xf]-1;
  info->start_song = data[0x11]-1;

  info->load_addr = data[data_file_offset];
  info->load_addr|= data[data_file_offset + 1] << 8;

  info->speed = data[0x15];

  c64_memset(0, 0, 64 * 1024);
  c64_memcpy(info->load_addr, &data[data_file_offset+2], size-(data_file_offset+2));

  strcpy(info->title, (const char*)&data[0x16]);
  strcpy(info->author, (const char*)&data[0x36]);
  strcpy(info->released, (const char*)&data[0x56]);

  if (info->play_addr == 0)
  {
    c64_cpu_jsr(info->init_addr, 0);
    info->play_addr = (c64_getmem(0x0315) << 8) | c64_getmem(0x0314);
  }

  return true;
}
