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
#ifndef __CRC8_H_
#define __CRC8_H_

#include <stdint.h>

/*
 * Precalculated look up table for CRC calculation
 */
extern const uint8_t crc_table[256];

/**
 *  \brief Calculate 8-bit CRC
 *
 *
 * Calculate an 8-bit CRC of a block of data. The algorithm used depends on
 * library configuration.
 *
 * \param[in] p pointer to the data to take we wish to compute the CRC of
 * \param[in] len length of the data
 * \param[in] crc Initial value for the CRC algorithm (or CRC of previous data)
 *
 * \return Calculated CRC value
 */
inline uint8_t crc8(uint8_t* p, uint8_t len, uint8_t crc)
{
    while (len--)
    {
        crc = crc_table[crc ^ *p++];
    }

    return crc;
}

/**
 * \brief Calculate 8-bit CRC of a byte
 *
 * Calculate an 8-bit CRC of a single byte. The algorithm used depends on
 * library configuration.
 *
 * \param[in] input data byte to compute the CRC of
 * \param[in] crc Initial value for the CRC algorithm (or CRC of previous data)
 *
 * \return Calculated CRC value
 */
inline uint8_t crc8(uint8_t input, uint8_t crc)
{
    crc = crc_table[crc ^ input];

    return crc;
}

#endif /* __CRC8_H_ */
