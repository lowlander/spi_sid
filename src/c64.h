/*
 * Copyright (c) 2020 Erwin Rol <erwin@erwinrol.com
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef C64_H
#define C64_H

uint8_t c64_getmem(uint16_t addr);
void c64_setmem(uint16_t addr, uint8_t value);
void c64_cpu_jsr(uint16_t new_pc, uint8_t new_a);
void c64_init(void);
void c64_memcpy(uint16_t dest, const uint8_t* src, uint32_t size);
void c64_memset(uint16_t dest, uint8_t val, uint32_t size);

void c64_cpu_reset(void);
void c64_cpu_reset_to(uint16_t new_pc, uint8_t new_a);


#endif /* C64_H */
