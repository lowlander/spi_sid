/*
 * Copyright (c) 2020 Erwin Rol <erwin@erwinrol.com
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * This file was based on the rockbox sidplayer, which was based on the
 * tinysid engine.
 *
 ****************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * SID Codec for rockbox based on the TinySID engine
 *
 * Written by Tammo Hinrichs (kb) and Rainer Sinsch in 1998-1999
 * Ported to rockbox on 14 April 2006
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 * kb explicitly points out that this emulation sounds crappy, though
 * we decided to put it open source so everyone can enjoy sidmusic
 * on rockbox
 *
 * v1.1
 * Added 16-04-2006: Rainer Sinsch
 * Removed all time critical floating point operations and
 * replaced them with quick & dirty integer calculations
 *
 * Added 17-04-2006: Rainer Sinsch
 * Improved quick & dirty integer calculations for the resonant filter
 * Improved audio quality by 4 bits
 *
 * v1.2
 * Added 17-04-2006: Dave Chapman
 * Improved file loading
 *
 * Added 17-04-2006: Rainer Sinsch
 * Added sample routines
 * Added cia timing routines
 * Added fast forwarding capabilities
 * Corrected bug in sid loading
 *
 * v1.2.1
 * Added 04-05-2006: Rainer Sinsch
 * Implemented Marco Alanens suggestion for subsong selection:
 * Select the subsong by seeking: Each second represents a subsong
 *
 */

#include "sid.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "mos6510.h"

static struct mos6510 cpu;

static uint8_t bval;
static uint16_t wval;

static uint8_t memory[65536];

uint8_t c64_getmem(uint16_t addr)
{
    return memory[addr];
}

void c64_setmem(uint16_t addr, uint8_t value)
{
  if ((addr & 0xfc00) == 0xd400) {
    sid_poke(addr & 0x1f, value);
  } else {
    memory[addr] = value;
  }
}

static uint8_t getaddr(uint8_t mode)
{
  uint16_t ad,ad2;

  switch(mode) {
    case MOS6510_MODE_IMP:
        return 0;

    case MOS6510_MODE_IMM:
        return c64_getmem(cpu.pc++);

    case MOS6510_MODE_ABS:
        ad = c64_getmem(cpu.pc++);
        ad |= 256 * c64_getmem(cpu.pc++);
        return c64_getmem(ad);

    case MOS6510_MODE_ABSX:
        ad = c64_getmem(cpu.pc++);
        ad |= 256*c64_getmem(cpu.pc++);
        ad2 = ad + cpu.x;
        return c64_getmem(ad2);

    case MOS6510_MODE_ABSY:
        ad = c64_getmem(cpu.pc++);
        ad |= 256 * c64_getmem(cpu.pc++);
        ad2 = ad + cpu.y;
        return c64_getmem(ad2);

    case MOS6510_MODE_ZP:
        ad = c64_getmem(cpu.pc++);
        return c64_getmem(ad);

    case MOS6510_MODE_ZPX:
        ad = c64_getmem(cpu.pc++);
        ad += cpu.x;
        return c64_getmem(ad & 0xff);

    case MOS6510_MODE_ZPY:
        ad = c64_getmem(cpu.pc++);
        ad += cpu.y;
        return c64_getmem(ad & 0xff);

    case MOS6510_MODE_INDX:
        ad = c64_getmem(cpu.pc++);
        ad += cpu.x;
        ad2 = c64_getmem(ad&0xff);
        ad++;
        ad2 |= c64_getmem(ad & 0xff) << 8;
        return c64_getmem(ad2);

    case MOS6510_MODE_INDY:
        ad = c64_getmem(cpu.pc++);
        ad2 = c64_getmem(ad);
        ad2 |= c64_getmem((ad+1) & 0xff) << 8;
        ad = ad2 + cpu.y;
        return c64_getmem(ad);

    case MOS6510_MODE_ACC:
        return cpu.a;
  }

  return 0;
}

static void setaddr(uint8_t mode, uint8_t val)
{
  uint16_t ad,ad2;

  switch(mode)
  {
    case MOS6510_MODE_ABS:
      ad = c64_getmem(cpu.pc - 2);
      ad |= 256 * c64_getmem(cpu.pc - 1);
      c64_setmem(ad, val);
      return;

    case MOS6510_MODE_ABSX:
      ad = c64_getmem(cpu.pc-2);
      ad |= 256 * c64_getmem(cpu.pc - 1);
      ad2 = ad + cpu.x;
      c64_setmem(ad2, val);
      return;

    case MOS6510_MODE_ZP:
      ad = c64_getmem(cpu.pc - 1);
      c64_setmem(ad, val);
      return;

    case MOS6510_MODE_ZPX:
      ad = c64_getmem(cpu.pc - 1);
      ad += cpu.x;
      c64_setmem(ad & 0xff, val);
      return;

    case MOS6510_MODE_ACC:
      cpu.a = val;
      return;

  }
}


static void putaddr(uint8_t mode, uint8_t val)
{
  uint16_t ad,ad2;

  switch(mode)
  {
    case MOS6510_MODE_ABS:
      ad = c64_getmem(cpu.pc++);
      ad |= c64_getmem(cpu.pc++) << 8;
      c64_setmem(ad, val);
      return;

    case MOS6510_MODE_ABSX:
      ad = c64_getmem(cpu.pc++);
      ad |= c64_getmem(cpu.pc++) << 8;
      ad2 = ad+cpu.x;
      c64_setmem(ad2, val);
      return;

    case MOS6510_MODE_ABSY:
      ad = c64_getmem(cpu.pc++);
      ad |= c64_getmem(cpu.pc++) << 8;
      ad2 = ad + cpu.y;
      c64_setmem(ad2, val);
      return;

    case MOS6510_MODE_ZP:
      ad = c64_getmem(cpu.pc++);
      c64_setmem(ad, val);
      return;

    case MOS6510_MODE_ZPX:
      ad = c64_getmem(cpu.pc++);
      ad += cpu.x;
      c64_setmem(ad & 0xff, val);
      return;

    case MOS6510_MODE_ZPY:
      ad = c64_getmem(cpu.pc++);
      ad += cpu.y;
      c64_setmem(ad & 0xff, val);
      return;

    case MOS6510_MODE_INDX:
      ad = c64_getmem(cpu.pc++);
      ad += cpu.x;
      ad2 = c64_getmem(ad & 0xff);
      ad++;
      ad2 |= c64_getmem(ad & 0xff) << 8;
      c64_setmem(ad2, val);
      return;

    case MOS6510_MODE_INDY:
      ad = c64_getmem(cpu.pc++);
      ad2 = c64_getmem(ad);
      ad2 |= c64_getmem((ad + 1) & 0xff) << 8;
      ad = ad2 + cpu.y;
      c64_setmem(ad, val);
      return;

    case MOS6510_MODE_ACC:
      cpu.a = val;
      return;

  }
}

static void setflags(uint8_t flag, int cond)
{
  if (cond) {
    cpu.p |= flag;
  } else {
    cpu.p &= ~flag;
  }
}

static void push(uint8_t val)
{
  c64_setmem(0x100 + cpu.s, val);

  if (cpu.s) {
    cpu.s--;
  }
}

static uint8_t pop(void)
{
  if (cpu.s < 0xff) {
    cpu.s++;
  }

  return c64_getmem(0x100 + cpu.s);
}

static void branch(uint8_t flag)
{
    int8_t dist;

    dist = (int8_t)getaddr(MOS6510_MODE_IMM);

    wval = cpu.pc + dist;

    if (flag) {
      cpu.pc = wval;
    }
}

void c64_cpu_reset(void)
{
  cpu.a = 0x00;
  cpu.x = 0x00;
  cpu.y = 0x00;
  cpu.p = 0x00;
  cpu.s = 0xff;
  cpu.pc = 0xfffc;
}

void c64_cpu_reset_to(uint16_t new_pc, uint8_t new_a)
{
  cpu.a = new_a;
  cpu.x = 0x00;
  cpu.y = 0x00;
  cpu.p = 0x00;
  cpu.s = 0xff;
  cpu.pc = new_pc;
}

static void c64_cpu_step(void)
{
  int c;

  uint8_t opc = c64_getmem(cpu.pc++);
  uint8_t cmd = mos6510_opcode_table[opc].type;
  uint8_t addr = mos6510_opcode_table[opc].mode;

  switch (cmd)
  {
    case MOS6510_TYPE_ADC:
        wval = (uint16_t)cpu.a + getaddr(addr) + ((cpu.p & MOS6510_FLAG_C) ? 1 : 0);
        setflags(MOS6510_FLAG_C, wval & 0x100);
        cpu.a = (uint8_t)wval;
        setflags(MOS6510_FLAG_Z, !cpu.a);
        setflags(MOS6510_FLAG_N, cpu.a & 0x80);
        setflags(MOS6510_FLAG_V, (!!(cpu.p & MOS6510_FLAG_C)) ^ (!!(cpu.p & MOS6510_FLAG_N)));
        break;

    case MOS6510_TYPE_AND:
        bval = getaddr(addr);
        cpu.a &= bval;
        setflags(MOS6510_FLAG_Z, !cpu.a);
        setflags(MOS6510_FLAG_N, cpu.a & 0x80);
        break;

    case MOS6510_TYPE_ASL:
        wval = getaddr(addr);
        wval <<= 1;
        setaddr(addr,(uint8_t)wval);
        setflags(MOS6510_FLAG_Z, !wval);
        setflags(MOS6510_FLAG_N, wval & 0x80);
        setflags(MOS6510_FLAG_C, wval & 0x100);
        break;

    case MOS6510_TYPE_BCC:
        branch(!(cpu.p & MOS6510_FLAG_C));
        break;

    case MOS6510_TYPE_BCS:
        branch(cpu.p & MOS6510_FLAG_C);
        break;

    case MOS6510_TYPE_BNE:
        branch(!(cpu.p & MOS6510_FLAG_Z));
        break;

    case MOS6510_TYPE_BEQ:
        branch(cpu.p & MOS6510_FLAG_Z);
        break;

    case MOS6510_TYPE_BPL:
        branch(!(cpu.p & MOS6510_FLAG_N));
        break;

    case MOS6510_TYPE_BMI:
        branch(cpu.p & MOS6510_FLAG_N);
        break;

    case MOS6510_TYPE_BVC:
        branch(!(cpu.p & MOS6510_FLAG_V));
        break;

    case MOS6510_TYPE_BVS:
        branch(cpu.p & MOS6510_FLAG_V);
        break;

    case MOS6510_TYPE_BIT:
        bval = getaddr(addr);
        setflags(MOS6510_FLAG_Z, !(cpu.a & bval));
        setflags(MOS6510_FLAG_N, bval & 0x80);
        setflags(MOS6510_FLAG_V, bval & 0x40);
        break;

    case MOS6510_TYPE_BRK:
        cpu.pc = 0; /* Just quit the emulation */
        break;

    case MOS6510_TYPE_CLC:
        setflags(MOS6510_FLAG_C, 0);
        break;

    case MOS6510_TYPE_CLD:
        setflags(MOS6510_FLAG_D, 0);
        break;

    case MOS6510_TYPE_CLI:
        setflags(MOS6510_FLAG_I, 0);
        break;

    case MOS6510_TYPE_CLV:
        setflags(MOS6510_FLAG_V, 0);
        break;

    case MOS6510_TYPE_CMP:
        bval = getaddr(addr);
        wval = (uint16_t)cpu.a - bval;
        setflags(MOS6510_FLAG_Z, !wval);
        setflags(MOS6510_FLAG_N, wval & 0x80);
        setflags(MOS6510_FLAG_C, cpu.a >= bval);
        break;

    case MOS6510_TYPE_CPX:
        bval = getaddr(addr);
        wval = (uint16_t)cpu.x-bval;
        setflags(MOS6510_FLAG_Z, !wval);
        setflags(MOS6510_FLAG_N, wval & 0x80);
        setflags(MOS6510_FLAG_C, cpu.x >= bval);
        break;

    case MOS6510_TYPE_CPY:
        bval = getaddr(addr);
        wval = (uint16_t)cpu.y - bval;
        setflags(MOS6510_FLAG_Z, !wval);
        setflags(MOS6510_FLAG_N, wval & 0x80);
        setflags(MOS6510_FLAG_C, cpu.y >= bval);
        break;

    case MOS6510_TYPE_DEC:
        bval = getaddr(addr);
        bval--;
        setaddr(addr, bval);
        setflags(MOS6510_FLAG_Z, !bval);
        setflags(MOS6510_FLAG_N, bval & 0x80);
        break;

    case MOS6510_TYPE_DEX:
        cpu.x--;
        setflags(MOS6510_FLAG_Z, !cpu.x);
        setflags(MOS6510_FLAG_N, cpu.x & 0x80);
        break;

    case MOS6510_TYPE_DEY:
        cpu.y--;
        setflags(MOS6510_FLAG_Z, !cpu.y);
        setflags(MOS6510_FLAG_N, cpu.y & 0x80);
        break;

    case MOS6510_TYPE_EOR:
        bval = getaddr(addr);
        cpu.a ^= bval;
        setflags(MOS6510_FLAG_Z, !cpu.a);
        setflags(MOS6510_FLAG_N, cpu.a & 0x80);
        break;

    case MOS6510_TYPE_INC:
        bval = getaddr(addr);
        bval++;
        setaddr(addr, bval);
        setflags(MOS6510_FLAG_Z, !bval);
        setflags(MOS6510_FLAG_N, bval & 0x80);
        break;

    case MOS6510_TYPE_INX:
        cpu.x++;
        setflags(MOS6510_FLAG_Z, !cpu.x);
        setflags(MOS6510_FLAG_N, cpu.x & 0x80);
        break;

    case MOS6510_TYPE_INY:
        cpu.y++;
        setflags(MOS6510_FLAG_Z, !cpu.y);
        setflags(MOS6510_FLAG_N, cpu.y & 0x80);
        break;

    case MOS6510_TYPE_JMP:
        wval = c64_getmem(cpu.pc++);
        wval |= 256 * c64_getmem(cpu.pc++);
        switch (addr)
        {
            case MOS6510_MODE_ABS:
                cpu.pc = wval;
                break;

            case MOS6510_MODE_IND:
                cpu.pc = c64_getmem(wval);
                cpu.pc |= 256 * c64_getmem(wval + 1);
                break;
        }
        break;

    case MOS6510_TYPE_JSR:
        push((cpu.pc + 1) >> 8);
        push((cpu.pc + 1));
        wval = c64_getmem(cpu.pc++);
        wval |= 256 * c64_getmem(cpu.pc++);
        cpu.pc = wval;
        break;

    case MOS6510_TYPE_LDA:
        cpu.a = getaddr(addr);
        setflags(MOS6510_FLAG_Z, !cpu.a);
        setflags(MOS6510_FLAG_N, cpu.a & 0x80);
        break;

    case MOS6510_TYPE_LDX:
        cpu.x = getaddr(addr);
        setflags(MOS6510_FLAG_Z, !cpu.x);
        setflags(MOS6510_FLAG_N, cpu.x & 0x80);
        break;

    case MOS6510_TYPE_LDY:
        cpu.y = getaddr(addr);
        setflags(MOS6510_FLAG_Z, !cpu.y);
        setflags(MOS6510_FLAG_N, cpu.y&0x80);
        break;

    case MOS6510_TYPE_LSR:
        bval = getaddr(addr);
        wval = (uint8_t)bval;
        wval >>= 1;
        setaddr(addr, (uint8_t)wval);
        setflags(MOS6510_FLAG_Z, !wval);
        setflags(MOS6510_FLAG_N, wval & 0x80);
        setflags(MOS6510_FLAG_C, bval & 1);
        break;

    case MOS6510_TYPE_NOP:
        break;

    case MOS6510_TYPE_ORA:
        bval = getaddr(addr);
        cpu.a |= bval;
        setflags(MOS6510_FLAG_Z, !cpu.a);
        setflags(MOS6510_FLAG_N, cpu.a & 0x80);
        break;

    case MOS6510_TYPE_PHA:
        push(cpu.a);
        break;
    case MOS6510_TYPE_PHP:
        push(cpu.p);
        break;
    case MOS6510_TYPE_PLA:
        cpu.a=pop();
        setflags(MOS6510_FLAG_Z,!cpu.a);
        setflags(MOS6510_FLAG_N,cpu.a&0x80);
        break;
    case MOS6510_TYPE_PLP:
        cpu.p=pop();
        break;
    case MOS6510_TYPE_ROL:
        bval = getaddr(addr);
        c = !!(cpu.p & MOS6510_FLAG_C);
        setflags(MOS6510_FLAG_C, bval & 0x80);
        bval <<= 1;
        bval |= c;
        setaddr(addr, bval);
        setflags(MOS6510_FLAG_N, bval & 0x80);
        setflags(MOS6510_FLAG_Z,!bval);
        break;

    case MOS6510_TYPE_ROR:
        bval = getaddr(addr);
        c = !!(cpu.p&MOS6510_FLAG_C);
        setflags(MOS6510_FLAG_C, bval & 1);
        bval >>= 1;
        bval |= 128 * c;
        setaddr(addr, bval);
        setflags(MOS6510_FLAG_N, bval & 0x80);
        setflags(MOS6510_FLAG_Z, !bval);
        break;

    case MOS6510_TYPE_RTI:
        /* Treat RTI like RTS */
    case MOS6510_TYPE_RTS:
        wval = pop();
        wval |= pop() << 8;
        cpu.pc = wval + 1;
        break;

    case MOS6510_TYPE_SBC:
        bval = getaddr(addr) ^ 0xff;
        wval=(uint16_t)cpu.a + bval + ((cpu.p & MOS6510_FLAG_C) ? 1 : 0);
        setflags(MOS6510_FLAG_C, wval & 0x100);
        cpu.a = (uint8_t)wval;
        setflags(MOS6510_FLAG_Z, !cpu.a);
        setflags(MOS6510_FLAG_N, cpu.a > 127);
        setflags(MOS6510_FLAG_V, (!!(cpu.p & MOS6510_FLAG_C)) ^ (!!(cpu.p & MOS6510_FLAG_N)));
        break;

    case MOS6510_TYPE_SEC:
        setflags(MOS6510_FLAG_C, 1);
        break;

    case MOS6510_TYPE_SED:
        setflags(MOS6510_FLAG_D, 1);
        break;

    case MOS6510_TYPE_SEI:
        setflags(MOS6510_FLAG_I, 1);
        break;

    case MOS6510_TYPE_STA:
        putaddr(addr, cpu.a);
        break;

    case MOS6510_TYPE_STX:
        putaddr(addr, cpu.x);
        break;

    case MOS6510_TYPE_STY:
        putaddr(addr, cpu.y);
        break;

    case MOS6510_TYPE_TAX:
        cpu.x = cpu.a;
        setflags(MOS6510_FLAG_Z, !cpu.x);
        setflags(MOS6510_FLAG_N, cpu.x & 0x80);
        break;

    case MOS6510_TYPE_TAY:
        cpu.y = cpu.a;
        setflags(MOS6510_FLAG_Z, !cpu.y);
        setflags(MOS6510_FLAG_N, cpu.y & 0x80);
        break;

    case MOS6510_TYPE_TSX:
        cpu.x = cpu.s;
        setflags(MOS6510_FLAG_Z, !cpu.x);
        setflags(MOS6510_FLAG_N, cpu.x & 0x80);
        break;

    case MOS6510_TYPE_TXA:
        cpu.a = cpu.x;
        setflags(MOS6510_FLAG_Z, !cpu.a);
        setflags(MOS6510_FLAG_N, cpu.a & 0x80);
        break;

    case MOS6510_TYPE_TXS:
        cpu.s = cpu.x;
        break;

    case MOS6510_TYPE_TYA:
        cpu.a = cpu.y;
        setflags(MOS6510_FLAG_Z, !cpu.a);
        setflags(MOS6510_FLAG_N, cpu.a & 0x80);
        break;

  }
}

void c64_cpu_jsr(uint16_t new_pc, uint8_t new_a)
{
  cpu.a = new_a;
  cpu.x = 0x00;
  cpu.y = 0x00;
  cpu.p = 0x00;
  cpu.s = 0xFF;
  cpu.pc = new_pc;
  push(0);
  push(0);

  while (cpu.pc > 1)
    c64_cpu_step();
}

void c64_init()
{
  memset(memory, 0, sizeof(memory));

  c64_cpu_reset();
}

void c64_memcpy(uint16_t dest, const uint8_t* src, uint32_t size)
{
  if (dest + size < 64*1024) {
    memcpy(&memory[dest], src, size);
  }
}

void c64_memset(uint16_t dest, uint8_t val, uint32_t size)
{
  if (dest + size < 64*1024) {
    memset(&memory[dest], val, size);
  }
}
