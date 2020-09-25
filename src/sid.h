/*
 * Copyright (c) 2020 Erwin Rol <erwin@erwinrol.com
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SID_H
#define SID_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void sid_poke(uint16_t reg, uint8_t val);

struct sid_info
{
    uint16_t load_addr;
    uint16_t init_addr;
    uint16_t play_addr;
    uint8_t  subsongs;
    uint8_t  start_song;
    uint8_t  speed;
    char     title[32];
    char     author[32];
    char     released[32];
};

bool sid_load_from_memory(const uint8_t* data, size_t size, struct sid_info* info);

#endif /* SID_H */
