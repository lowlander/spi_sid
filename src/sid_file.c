/*
 * Copyright (c) 2020 Erwin Rol <erwin@erwinrol.com
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "sid_file.h"

const uint8_t sid_file_data[] = {
#include "big_fun_tune_5.hex"
};

const uint8_t* sid_file = sid_file_data;
const uint16_t sid_file_size = sizeof(sid_file_data);
