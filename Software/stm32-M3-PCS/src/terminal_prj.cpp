/*
 * This file is part of the stm32-template project.
 *
 * Copyright (C) 2020 Johannes Huebner <dev@johanneshuebner.com>
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

/* This file contains a standard set of commands that are used by the
 * esp8266 web interface.
 * You can add your own commands if needed
 */
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/usart.h>
#include "hwdefs.h"
#include "terminal.h"
#include "params.h"
#include "my_string.h"
#include "my_fp.h"
#include "printf.h"
#include "param_save.h"
#include "errormessage.h"
#include "stm32_can.h"
#include "terminalcommands.h"
#include "PCSCan.h"

static void LoadDefaults(Terminal* term, char *arg);
static void PrintParamsJson(Terminal* term, char *arg);
static void PrintSerial(Terminal* term, char *arg);
static void MapCan(Terminal* term, char *arg);
static void PrintErrors(Terminal* term, char *arg);

static Terminal* curTerm = NULL;

extern "C" const TERM_CMD termCmds[] =
{
  { "set", TerminalCommands::ParamSet },
  { "get", TerminalCommands::ParamGet },
  { "flag", TerminalCommands::ParamFlag },
  { "stream", TerminalCommands::ParamStream },
  { "defaults", LoadDefaults },
  { "save", TerminalCommands::SaveParameters },
  { "load", TerminalCommands::LoadParameters },
  { "json", PrintParamsJson },
  { "can", MapCan },
  { "serial", PrintSerial },
  { "errors", PrintErrors },
  { "reset", TerminalCommands::Reset },
  { NULL, NULL }
};

static void PrintCanMap(Param::PARAM_NUM param, uint32_t canid, uint8_t offset, uint8_t length, float gain, bool rx)
{
   const char* name = Param::GetAttrib(param)->name;
   fprintf(curTerm, "can ");

   if (rx)
      fprintf(curTerm, "rx ");
   else
      fprintf(curTerm, "tx ");
   fprintf(curTerm, "%s %d %d %d %f\r\n", name, canid, offset, length, FP_FROMFLT(gain));
}

//cantx param id offset len gain
static void MapCan(Terminal* term, char *arg)
{
   Param::PARAM_NUM paramIdx = Param::PARAM_INVALID;
   int values[4];
   int result;
   char op;
   char *ending;
   const int numArgs = 4;

   arg = my_trim(arg);

   if (arg[0] == 'p')
   {
      while (curTerm != NULL); //lock
      curTerm = term;
      Can::GetInterface(0)->IterateCanMap(PrintCanMap);
      curTerm = NULL;
      return;
   }

   if (arg[0] == 'c')
   {
      Can::GetInterface(0)->Clear();
//      ChargerCAN::MapMessages(Can::GetInterface(0));
      fprintf(term, "Default CAN mapping restored\r\n");
      return;
   }

   op = arg[0];
   arg = (char *)my_strchr(arg, ' ');

   if (0 == *arg)
   {
      fprintf(term, "Missing argument\r\n");
      return;
   }

   arg = my_trim(arg);
   ending = (char *)my_strchr(arg, ' ');

   if (*ending == 0 && op != 'd')
   {
      fprintf(term, "Missing argument\r\n");
      return;
   }

   *ending = 0;
   paramIdx = Param::NumFromString(arg);
   arg = my_trim(ending + 1);

   if (Param::PARAM_INVALID == paramIdx)
   {
      fprintf(term, "Unknown parameter\r\n");
      return;
   }

   if (op == 'd')
   {
      if (paramIdx >= Param::version)
      {
         fprintf(term, "Internal CAN map can not be deleted\r\n");
         return;
      }
      result = Can::GetInterface(0)->Remove(paramIdx);
      fprintf(term, "%d entries removed\r\n", result);
      return;
   }

   for (int i = 0; i < numArgs; i++)
   {
      ending = (char *)my_strchr(arg, ' ');

      if (0 == *ending && i < (numArgs - 1))
      {
         fprintf(term, "Missing argument\r\n");
         return;
      }

      *ending = 0;
      int iVal = my_atoi(arg);

      //allow gain values < 1 and re-interpret them
      if (i == (numArgs - 1) && iVal == 0)
      {
         values[i] = fp_atoi(arg, 16);
      }
      else
      {
         values[i] = iVal;
      }

      arg = my_trim(ending + 1);
   }

   if (op == 't')
   {
      result = Can::GetInterface(0)->AddSend(paramIdx, values[0], values[1], values[2], values[3] / 65536.0f);
   }
   else
   {
      result = Can::GetInterface(0)->AddRecv(paramIdx, values[0], values[1], values[2], values[3] / 65536.0f);
   }

   switch (result)
   {
      case CAN_ERR_INVALID_ID:
         fprintf(term, "Invalid CAN Id %x\r\n", values[0]);
         break;
      case CAN_ERR_INVALID_OFS:
         fprintf(term, "Invalid Offset %d\r\n", values[1]);
         break;
      case CAN_ERR_INVALID_LEN:
         fprintf(term, "Invalid length %d\r\n", values[2]);
         break;
      case CAN_ERR_MAXITEMS:
         fprintf(term, "Cannot map anymore items to CAN id %d\r\n", values[0]);
         break;
      case CAN_ERR_MAXMESSAGES:
         fprintf(term, "Max message count reached\r\n");
         break;
      default:
         fprintf(term, "CAN map successful, %d message%s active\r\n", result, result > 1 ? "s" : "");
   }
}

static void PrintParamsJson(Terminal* term, char *arg)
{
   arg = my_trim(arg);

   const Param::Attributes *pAtr;
   char comma = ' ';
   bool printHidden = arg[0] == 'h';

   fprintf(term, "{");
   for (uint32_t idx = 0; idx < Param::PARAM_LAST; idx++)
   {
      pAtr = Param::GetAttrib((Param::PARAM_NUM)idx);

      if ((Param::GetFlag((Param::PARAM_NUM)idx) & Param::FLAG_HIDDEN) == 0 || printHidden)
      {
         fprintf(term, "%c\r\n   \"%s\": {\"unit\":\"%s\",\"value\":%f,",comma, pAtr->name, pAtr->unit, Param::Get((Param::PARAM_NUM)idx));

         if (Param::IsParam((Param::PARAM_NUM)idx))
         {
            fprintf(term, "\"isparam\":true,\"minimum\":%f,\"maximum\":%f,\"default\":%f,\"category\":\"%s\",\"i\":%d}",
                   pAtr->min, pAtr->max, pAtr->def, pAtr->category, idx);
         }
         else
         {
            fprintf(term, "\"isparam\":false}");
         }
         comma = ',';
      }
   }

   fprintf(term, "\r\n}\r\n");
}


static void LoadDefaults(Terminal* term, char *arg)
{
   arg = arg;
   Param::LoadDefaults();
   fprintf(term, "Defaults loaded\r\n");
}

static void PrintErrors(Terminal* term, char *arg)
{
   term = term;
   arg = arg;
   ErrorMessage::PrintAllErrors();
}

static void PrintSerial(Terminal* term, char *arg)
{
   arg = arg;
   fprintf(term, "%X:%X:%X\r\n", DESIG_UNIQUE_ID2, DESIG_UNIQUE_ID1, DESIG_UNIQUE_ID0);
}
