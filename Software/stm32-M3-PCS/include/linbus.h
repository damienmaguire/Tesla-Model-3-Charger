/*
 * This file is part of the stm32-car project.
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
#ifndef LINBUS_H
#define LINBUS_H


class LinBus
{
   public:
      /** Default constructor */
      LinBus(uint32_t usart, int baudrate);
      void Request(uint8_t id, uint8_t* data, uint8_t len);
      bool HasReceived(uint8_t pid, uint8_t requiredLen);
      uint8_t* GetReceivedBytes() { return &recvBuffer[payloadIndex]; }

   protected:

   private:
      struct HwInfo
      {
         uint32_t usart;
         uint8_t dmatx;
         uint8_t dmarx;
         uint32_t port;
         uint16_t pin;
      };

      static uint8_t Checksum(uint8_t pid, uint8_t* data, int len);
      static uint8_t Parity(uint8_t id);

      static const HwInfo hwInfo[];
      static const int payloadIndex = 3;
      static const int pidIndex = 2;
      uint32_t usart;
      const HwInfo* hw;
      uint8_t sendBuffer[11];
      uint8_t recvBuffer[12];
};

#endif // LINBUS_H
