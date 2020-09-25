/*
 * Copyright (c) 2020 Erwin Rol <erwin@erwinrol.com
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef MOS6510_H
#define MOS6510_H

#include <stdint.h>

#define MOS6510_FLAG_N  128
#define MOS6510_FLAG_V  64
#define MOS6510_FLAG_B  16
#define MOS6510_FLAG_D  8
#define MOS6510_FLAG_I  4
#define MOS6510_FLAG_Z  2
#define MOS6510_FLAG_C  1

#define MOS6510_MODE_IMP  0
#define MOS6510_MODE_IMM  1
#define MOS6510_MODE_ABS  2
#define MOS6510_MODE_ABSX 3
#define MOS6510_MODE_ABSY 4
#define MOS6510_MODE_ZP   6
#define MOS6510_MODE_ZPX  7
#define MOS6510_MODE_ZPY  8
#define MOS6510_MODE_IND  9
#define MOS6510_MODE_INDX 10
#define MOS6510_MODE_INDY 11
#define MOS6510_MODE_ACC  12
#define MOS6510_MODE_REL  13
#define MOS6510_MODE_XXX  14

#define MOS6510_TYPE_ADC  0
#define MOS6510_TYPE_AND  1
#define MOS6510_TYPE_ASL  2
#define MOS6510_TYPE_BCC  3
#define MOS6510_TYPE_BCS  4
#define MOS6510_TYPE_BEQ  5
#define MOS6510_TYPE_BIT  6
#define MOS6510_TYPE_BMI  7
#define MOS6510_TYPE_BNE  8
#define MOS6510_TYPE_BPL  9
#define MOS6510_TYPE_BRK  10
#define MOS6510_TYPE_BVC  11
#define MOS6510_TYPE_BVS  12
#define MOS6510_TYPE_CLC  13
#define MOS6510_TYPE_CLD  14
#define MOS6510_TYPE_CLI  15
#define MOS6510_TYPE_CLV  16
#define MOS6510_TYPE_CMP  17
#define MOS6510_TYPE_CPX  18
#define MOS6510_TYPE_CPY  19
#define MOS6510_TYPE_DEC  20
#define MOS6510_TYPE_DEX  21
#define MOS6510_TYPE_DEY  22
#define MOS6510_TYPE_EOR  23
#define MOS6510_TYPE_INC  24
#define MOS6510_TYPE_INX  25
#define MOS6510_TYPE_INY  26
#define MOS6510_TYPE_JMP  27
#define MOS6510_TYPE_JSR  28
#define MOS6510_TYPE_LDA  29
#define MOS6510_TYPE_LDX  30
#define MOS6510_TYPE_LDY  31
#define MOS6510_TYPE_LSR  32
#define MOS6510_TYPE_NOP  33
#define MOS6510_TYPE_ORA  34
#define MOS6510_TYPE_PHA  35
#define MOS6510_TYPE_PHP  36
#define MOS6510_TYPE_PLA  37
#define MOS6510_TYPE_PLP  38
#define MOS6510_TYPE_ROL  39
#define MOS6510_TYPE_ROR  40
#define MOS6510_TYPE_RTI  41
#define MOS6510_TYPE_RTS  42
#define MOS6510_TYPE_SBC  43
#define MOS6510_TYPE_SEC  44
#define MOS6510_TYPE_SED  45
#define MOS6510_TYPE_SEI  46
#define MOS6510_TYPE_STA  47
#define MOS6510_TYPE_STX  48
#define MOS6510_TYPE_STY  49
#define MOS6510_TYPE_TAX  50
#define MOS6510_TYPE_TAY  51
#define MOS6510_TYPE_TSX  52
#define MOS6510_TYPE_TXA  53
#define MOS6510_TYPE_TXS  54
#define MOS6510_TYPE_TYA  55
#define MOS6510_TYPE_XXX  56

#define MOS6510_TYPE_SLO  57
#define MOS6510_TYPE_ANC  58
#define MOS6510_TYPE_RLA  59
#define MOS6510_TYPE_SRE  60
#define MOS6510_TYPE_ALR  61
#define MOS6510_TYPE_RRA  62
#define MOS6510_TYPE_ARR  63
#define MOS6510_TYPE_SAX  64
#define MOS6510_TYPE_XAA  65
#define MOS6510_TYPE_AHX  66
#define MOS6510_TYPE_TAS  67
#define MOS6510_TYPE_SHY  68
#define MOS6510_TYPE_SHX  69
#define MOS6510_TYPE_LAX  70
#define MOS6510_TYPE_LAS  71
#define MOS6510_TYPE_DCP  72
#define MOS6510_TYPE_AXS  73
#define MOS6510_TYPE_ISC  74

struct mos6510 {
  uint8_t a;
  uint8_t x;
  uint8_t y;
  uint8_t s;
  uint8_t p;
  uint16_t pc;
};

struct mos6510_opcode {
  uint8_t type;
  uint8_t mode;
};

extern const struct mos6510_opcode mos6510_opcode_table[256];

#endif /* MOS6510_H */
