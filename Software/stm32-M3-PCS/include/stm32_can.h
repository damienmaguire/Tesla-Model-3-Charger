/*
 * This file is part of the libopeninv project.
 *
 * Copyright (C) 2016 Nail Güzel
 * Johannes Huebner <dev@johanneshuebner.com>
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
#ifndef STM32_CAN_H_INCLUDED
#define STM32_CAN_H_INCLUDED
#include "params.h"

#define CAN_ERR_INVALID_ID -1
#define CAN_ERR_INVALID_OFS -2
#define CAN_ERR_INVALID_LEN -3
#define CAN_ERR_MAXMESSAGES -4
#define CAN_ERR_MAXITEMS -5

#ifndef MAX_ITEMS
#define MAX_ITEMS 70
#endif // MAX_ITEMS_PER_MESSAGE

#ifndef MAX_MESSAGES
#define MAX_MESSAGES 10
#endif // MAX_MESSAGES

#ifndef SENDBUFFER_LEN
#define SENDBUFFER_LEN 20
#endif // SENDBUFFER_LEN

#ifndef MAX_USER_MESSAGES
#define MAX_USER_MESSAGES 10
#endif // MAX_USER_MESSAGES


class CANIDMAP;
class SENDBUFFER;

class Can
{
public:
   enum baudrates
   {
      Baud125, Baud250, Baud500, Baud800, Baud1000, BaudLast
   };

   Can(uint32_t baseAddr, enum baudrates baudrate, bool remap = false);
   void Clear(void);
   void SetBaudrate(enum baudrates baudrate);
   void Send(uint32_t canId, uint32_t data[2]) { Send(canId, data, 8); }
   void Send(uint32_t canId, uint8_t data[8], uint8_t len) { Send(canId, (uint32_t*)data, len); }
   void Send(uint32_t canId, uint32_t data[2], uint8_t len);
   void SendAll();
   void SDOWrite(uint8_t remoteNodeId, uint16_t index, uint8_t subIndex, uint32_t data);
   void Save();
   void SetReceiveCallback(void (*recv)(uint32_t, uint32_t*));
   bool RegisterUserMessage(uint32_t canId);
   void ClearUserMessages();
   uint32_t GetLastRxTimestamp();
   int AddSend(Param::PARAM_NUM param, uint32_t canId, uint8_t offsetBits, uint8_t length, float gain);
   int AddRecv(Param::PARAM_NUM param, uint32_t canId, uint8_t offsetBits, uint8_t length, float gain);
   int AddSend(Param::PARAM_NUM param, uint32_t canId, uint8_t offsetBits, uint8_t length, float gain, int8_t offset);
   int AddRecv(Param::PARAM_NUM param, uint32_t canId, uint8_t offsetBits, uint8_t length, float gain, int8_t offset);
   int Remove(Param::PARAM_NUM param);
   bool FindMap(Param::PARAM_NUM param, uint32_t& canId, uint8_t& offset, uint8_t& length, float& gain, bool& rx);
   void IterateCanMap(void (*callback)(Param::PARAM_NUM, uint32_t, uint8_t, uint8_t, float, bool));
   void HandleRx(int fifo);
   void HandleTx();
   void SetNodeId(uint8_t id) { nodeId = id; }
   static Can* GetInterface(int index);

private:
   static volatile bool isSaving;

   struct CANPOS
   {
      float gain;
      uint16_t mapParam;
      int8_t offset;
      uint8_t offsetBits;
      uint8_t numBits;
      uint8_t next;
   };

   struct CANIDMAP
   {
      #ifdef CAN_EXT
      uint32_t canId;
      #else
      uint16_t canId;
      #endif // CAN_EXT
      uint8_t first;
   };

   struct SENDBUFFER
   {
      uint32_t id;
      uint32_t len;
      uint32_t data[2];
   };

   CANIDMAP canSendMap[MAX_MESSAGES];
   CANIDMAP canRecvMap[MAX_MESSAGES];
   CANPOS canPosMap[MAX_ITEMS + 1]; //Last item is a "tail"
   uint32_t lastRxTimestamp;
   SENDBUFFER sendBuffer[SENDBUFFER_LEN];
   int sendCnt;
   void (*recvCallback)(uint32_t, uint32_t*);
   uint16_t userIds[MAX_USER_MESSAGES];
   int nextUserMessageIndex;
   uint32_t canDev;
   uint8_t nodeId;

   void ProcessSDO(uint32_t data[2]);
   void ClearMap(CANIDMAP *canMap);
   int RemoveFromMap(CANIDMAP *canMap, Param::PARAM_NUM param);
   int Add(CANIDMAP *canMap, Param::PARAM_NUM param, uint32_t canId, uint8_t offsetBits, uint8_t length, float gain, int8_t offset);
   uint32_t SaveToFlash(uint32_t baseAddress, uint32_t* data, int len);
   int LoadFromFlash();
   CANIDMAP *FindById(CANIDMAP *canMap, uint32_t canId);
   int CopyIdMapExcept(CANIDMAP *source, CANIDMAP *dest, Param::PARAM_NUM param);
   void ReplaceParamEnumByUid(CANIDMAP *canMap);
   void ReplaceParamUidByEnum(CANIDMAP *canMap);
   void ConfigureFilters();
   void SetFilterBank(int& idIndex, int& filterId, uint16_t* idList);
   void SetFilterBank29(int& idIndex, int& filterId, uint32_t* idList);
   uint32_t GetFlashAddress();

   static Can* interfaces[];
};


#endif
