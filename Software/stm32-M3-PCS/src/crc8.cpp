/*
 * This file is part of the stm32-sine project.
 *
 * Copyright (C) 2021 David J. Fiddes <D.J@fiddes.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Generated table driven CRC-8-CCITT code from:
 * https://stackoverflow.com/a/27843120/6353
 */

#include "crc8.h"
#include <stdint.h>

/*
 * Just change this define to whatever polynomial is in use
 * A polynomial of 0x07 corresponds to x^8 + x^2 + x + 1
 */
#define CRC1B(b) ((uint8_t)((b) << 1) ^ ((b)&0x80 ? 0x07 : 0)) // MS first

/*
 * 8+1 entry enum lookup table define
 */
#define CRC(b) CRC_##b // or CRC8B(b)

enum
{
    CRC(0x01) = CRC1B(0x80),
    CRC(0x02) = CRC1B(CRC(0x01)),
    CRC(0x04) = CRC1B(CRC(0x02)),
    CRC(0x08) = CRC1B(CRC(0x04)),
    CRC(0x10) = CRC1B(CRC(0x08)),
    CRC(0x20) = CRC1B(CRC(0x10)),
    CRC(0x40) = CRC1B(CRC(0x20)),
    CRC(0x80) = CRC1B(CRC(0x40)),
    // Add 0x03 to optimise in CRCTAB1
    CRC(0x03) = CRC(0x02) ^ CRC(0x01)
};

/*
 * Build a 256 byte CRC constant lookup table, built from from a reduced
 * constant lookup table, namely CRC of each bit, 0x00 to 0x80. These will be
 * defined as enumerations to take it easy on the compiler. This depends on the
 * relation: CRC(a^b) = CRC(a)^CRC(b) In other words, we can build up each byte
 * CRC as the xor of the CRC of each bit. So CRC(0x05) = CRC(0x04)^CRC(0x01). We
 * include the CRC of 0x03 for a little more optimisation, since CRCTAB1 can use
 * it instead of CRC(0x01)^CRC(0x02), again a little easier on the compiler.
 */

#define CRCTAB1(ex) CRC(0x01) ex, CRC(0x02) ex, CRC(0x03) ex,
#define CRCTAB2(ex) CRCTAB1(ex) CRC(0x04) ex, CRCTAB1(^CRC(0x04) ex)
#define CRCTAB3(ex) CRCTAB2(ex) CRC(0x08) ex, CRCTAB2(^CRC(0x08) ex)
#define CRCTAB4(ex) CRCTAB3(ex) CRC(0x10) ex, CRCTAB3(^CRC(0x10) ex)
#define CRCTAB5(ex) CRCTAB4(ex) CRC(0x20) ex, CRCTAB4(^CRC(0x20) ex)
#define CRCTAB6(ex) CRCTAB5(ex) CRC(0x40) ex, CRCTAB5(^CRC(0x40) ex)

/*
 * This is the final lookup table. It is rough on the compiler, but generates
 * the required lookup table automagically at compile time.
 */
const uint8_t crc_table[256] = { 0, CRCTAB6() CRC(0x80), CRCTAB6(^CRC(0x80)) };
