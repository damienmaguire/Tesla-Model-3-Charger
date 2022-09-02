/*
 * This file is part of the stm32-... project.
 *
 * Copyright (C) 2021 Johannes Huebner <dev@johanneshuebner.com>
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
#include "chargercan.h"

void ChargerCAN::MapMessages(Can* can)
{
   can->AddRecv(Param::hwaclim, 0x207, 32, 9, 0.06666f); //gain 0.06666
   can->AddRecv(Param::hwaclim, 0x209, 32, 9, 0.06666f); //gain 0.06666
   can->AddRecv(Param::hwaclim, 0x20B, 32, 9, 0.06666f); //gain 0.06666
   can->AddRecv(Param::c1iac, 0x207, 41, 9, 0.06666f); //gain 0.06666
   can->AddRecv(Param::c2iac, 0x209, 41, 9, 0.06666f); //gain 0.06666
   can->AddRecv(Param::c3iac, 0x20B, 41, 9, 0.06666f); //gain 0.06666
   can->AddRecv(Param::c1uac, 0x207, 8, 8, 1);
   can->AddRecv(Param::c2uac, 0x209, 8, 8, 1);
   can->AddRecv(Param::c3uac, 0x20B, 8, 8, 1);
   can->AddRecv(Param::c1flag, 0x207, 17, 2, 1);
   can->AddRecv(Param::c2flag, 0x209, 17, 2, 1);
   can->AddRecv(Param::c3flag, 0x20B, 17, 2, 1);
   can->AddRecv(Param::c1stt, 0x217, 0, 8, 1);
   can->AddRecv(Param::c2stt, 0x219, 0, 8, 1);
   can->AddRecv(Param::c3stt, 0x21B, 0, 8, 1);
   can->AddRecv(Param::c1idc, 0x227, 32, 16, 0.000839233f); //gain 0.000839233
   can->AddRecv(Param::c2idc, 0x229, 32, 16, 0.000839233f); //gain 0.000839233
   can->AddRecv(Param::c3idc, 0x22B, 32, 16, 0.000839233f); //gain 0.000839233
   can->AddRecv(Param::c1udc, 0x227, 16, 16, 0.01052856f); //gain 0.01052856
   can->AddRecv(Param::c2udc, 0x229, 16, 16, 0.01052856f); //gain 0.01052856
   can->AddRecv(Param::c3udc, 0x22B, 16, 16, 0.01052856f); //gain 0.01052856
   can->AddRecv(Param::c1tmp1, 0x237, 0, 8, 1, -40); //offset -40°C
   can->AddRecv(Param::c2tmp1, 0x239, 0, 8, 1, -40); //offset -40°C
   can->AddRecv(Param::c3tmp1, 0x23B, 0, 8, 1, -40); //offset -40°C
   can->AddRecv(Param::c1tmp2, 0x237, 8, 8, 1, -40); //offset -40°C
   can->AddRecv(Param::c2tmp2, 0x239, 8, 8, 1, -40); //offset -40°C
   can->AddRecv(Param::c3tmp2, 0x23B, 8, 8, 1, -40); //offset -40°C
   can->AddRecv(Param::c1tmpin, 0x237, 40, 8, 1, -40); //offset -40°C
   can->AddRecv(Param::c2tmpin, 0x239, 40, 8, 1, -40); //offset -40°C
   can->AddRecv(Param::c3tmpin, 0x23B, 40, 8, 1, -40); //offset -40°C
   //We don't have enough space for all messages, so we discard these
   //can->AddRecv(Param::c1tmplim, 0x247, 0, 8, 7); //gain 0.234375
   //can->AddRecv(Param::c2tmplim, 0x249, 0, 8, 7); //gain 0.234375
   //can->AddRecv(Param::c3tmplim, 0x24B, 0, 8, 7); //gain 0.234375
   /***** CHAdeMO RX *****/
   can->AddRecv(Param::canenable, 0x102, 40, 1, 1);
   can->AddRecv(Param::idcspnt,   0x102, 24, 8, 1);
   can->AddRecv(Param::udclim,    0x102, 8, 16, 1);
   can->AddRecv(Param::soc,       0x102, 48, 8, 1);

   /***** Charger TX ******/
   can->AddSend(Param::udcspnt, 0x45c, 0, 16, 100); //gain 100
   //Send 0xe14 when opmode is 0 and 0x2e14 when opmode=1
   can->AddSend(Param::opmode, 0x45c, 24, 8, 32, 0xe); //opmode is 1 when running
   //constant value 0x14, version just used as dummy, multiplied by 0
   can->AddSend(Param::version, 0x45c, 16, 8, 0, 0x14);
   //same as above
   can->AddSend(Param::version, 0x45c, 48, 8, 0, (int8_t)0x90);
   can->AddSend(Param::version, 0x45c, 56, 8, 0, (int8_t)0x8c);

   can->AddSend(Param::version, 0x42c, 0, 8, 0, 0x42);
   can->AddSend(Param::version, 0x42c, 8, 8, 0, (int8_t)0xBB); //TODO: AC voltage
   can->AddSend(Param::aclim,  0x42c, 16, 16, 1500);
   can->AddSend(Param::opmode, 0x42c, 32, 8, 154, 100);

   can->AddSend(Param::version, 0x43c, 0, 8, 0, 0x42);
   can->AddSend(Param::version, 0x43c, 8, 8, 0, (int8_t)0xBB); //TODO: AC voltage
   can->AddSend(Param::aclim, 0x43c, 16, 16, 1500);
   can->AddSend(Param::opmode, 0x43c, 32, 8, 154, 100);

   can->AddSend(Param::version, 0x44c, 0, 8, 0, 0x42);
   can->AddSend(Param::version, 0x44c, 8, 8, 0, (int8_t)0xBB); //TODO: AC voltage
   can->AddSend(Param::aclim, 0x44c, 16, 16, 1500);
   can->AddSend(Param::opmode, 0x44c, 32, 8, 154, 100);

   can->AddSend(Param::version, 0x368, 0, 8, 0, 0x03);
   can->AddSend(Param::version, 0x368, 8, 8, 0, 0x49);
   can->AddSend(Param::version, 0x368, 16, 8, 0, 0x29);
   can->AddSend(Param::version, 0x368, 24, 8, 0, 0x11);
   can->AddSend(Param::version, 0x368, 40, 8, 0, 0x0c);
   can->AddSend(Param::version, 0x368, 48, 8, 0, 0x40);
   can->AddSend(Param::version, 0x368, 56, 8, 0, (int8_t)0xff);

   /***** CHAdeMO TX *****/
   can->AddSend(Param::version, 0x108, 8, 16, 107); //output 428V max = 4*107
   can->AddSend(Param::idclim, 0x108, 24, 8, 1);
   can->AddSend(Param::udc, 0x109, 8, 16, 1);
   can->AddSend(Param::idc, 0x109, 24, 16, 1);
   can->AddSend(Param::opmode, 0x109, 40, 3, 5); //Set charging and connlock at once
}
