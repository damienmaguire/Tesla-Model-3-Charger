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
#include <stdint.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/can.h>
#include <libopencm3/stm32/iwdg.h>
#include <libopencm3/stm32/crc.h>
#include "stm32_can.h"
#include "terminal.h"
#include "params.h"
#include "hwdefs.h"
#include "digio.h"
#include "hwinit.h"
#include "anain.h"
#include "param_save.h"
#include "my_math.h"
#include "errormessage.h"
#include "printf.h"
#include "stm32scheduler.h"
#include "PCSCan.h"

static Stm32Scheduler* scheduler;
static Can* can;
static uint32_t startTime;
static bool CAN_Enable=false;
static uint8_t PreChTmr=30;//precharge delay of 3 seconds before light off dcdc during charge mode.
static uint16_t ChgPower=0;
static uint8_t pwrDntmr=10;
static bool ZeroPower=false;

static void EvseRead()
{
   const int threshProxType1 = 2200;
   const int threshProx = 3700;
   const int thresh13A = 3200;
   const int thresh20A = 2800;
   const int thresh32A = 1800;
   const int thresh63A = 1000;
   int val = AnaIn::cablelim.Get();

   if (timer_get_flag(TIM3, TIM_SR_CC2IF))
   {
      //The relationship between duty cycle and maximum current is linear
      //until 85% = 51A. Above that it becomes non-linear but that is not
      //relevant for our 10kW charger.
      float evselim = timer_get_ic_value(TIM3, TIM_IC2) / 10;
      evselim *= 0.666666f;
      Param::SetFloat(Param::evselim, evselim);
   }
   else
   {
      //If no PWM detected, set limit to 0
      Param::SetInt(Param::evselim, 0);
   }

   if (Param::GetInt(Param::inputype) == INP_TYPE2)
   {
      if (val > threshProx)
      {
         Param::SetInt(Param::proximity, 0);
         Param::SetInt(Param::cablelim, 0);
      }
      else
      {
         Param::SetInt(Param::proximity, 1);

         if (val > thresh13A)
         {
            Param::SetInt(Param::cablelim, 13);
         }
         else if (val > thresh20A)
         {
            Param::SetInt(Param::cablelim, 20);
         }
         else if (val > thresh32A)
         {
            Param::SetInt(Param::cablelim, 32);
         }
         else if (val > thresh63A)
         {
            Param::SetInt(Param::cablelim, 63);
         }
      }
   }
   else if (Param::GetInt(Param::inputype) == INP_TYPE1)
   {
      if (val > threshProxType1)
      {
         Param::SetInt(Param::proximity, 0);
         Param::SetInt(Param::cablelim, 0);
      }
      else
      {
         Param::SetInt(Param::proximity, 1);
         Param::SetInt(Param::cablelim, 40);
      }
   }
   else
   {
      Param::SetInt(Param::proximity, 0);
      Param::SetInt(Param::cablelim, 32);
   }
}

static void DisableAll()
{
   DigIo::hvena_out.Clear();
   DigIo::acpres_out.Clear();
   DigIo::evseact_out.Clear();
   DigIo::pcsena_out.Clear();
   DigIo::dcdcena_out.Set();
   DigIo::chena_out.Set();
   CAN_Enable=false;
}

static bool IsEvseInput()
{
   enum inputs input = (enum inputs)Param::GetInt(Param::inputype);
   return input == INP_TYPE1 || input == INP_TYPE2 || input == INP_TYPE2_3P || input == INP_TYPE2_AUTO;
}

static bool CheckStartCondition()
{
   return (IsEvseInput() && Param::GetBool(Param::proximity) && Param::Get(Param::cablelim) > FP_FROMFLT(1.4) && Param::GetBool(Param::enable)) ||
         (!IsEvseInput() && Param::GetBool(Param::enable));

}

static bool CheckVoltage()
{
   static int timeout = 0;

   if (Param::Get(Param::udc) > Param::Get(Param::udclim))
   {
      timeout++;
   }
   else
   {
      timeout = 0;
   }

   return timeout > 10;
}

static bool CheckChargerFaults()
{
    return 0;
}

static bool CheckUnplugged()
{
   return IsEvseInput() && !Param::GetBool(Param::proximity);
}


static bool CheckTimeout()
{
   uint32_t now = rtc_get_counter_val();
   uint32_t timeout = Param::GetInt(Param::timelim);


   timeout *= 60;

   return timeout > 0 && (now - startTime) > timeout;
}

static bool CheckDelay()
{
   uint32_t now = rtc_get_counter_val();
   uint32_t start = Param::GetInt(Param::timedly) * 60;
   if(start<60)
   {
       return 1;
   }
   else
   {
       return start > 0 && (now - startTime) > start;
   }

}




static void PCSEnable()
{
 bool enable = DigIo::enable_in.Get();
 Param::SetInt(Param::enable, enable);
 bool Drv_en = DigIo::d2_in.Get();
 Param::SetInt(Param::Drive_En, Drv_en);

}


static void ChargerStateMachine()
{
   static states state = OFF;
   uint8_t activate = Param::GetInt(Param::activate);
   uint8_t PCS_CHG_Status=Param::GetInt(Param::CHG_STAT);

  // if (!Param::GetBool(Param::enable) && !Param::GetBool(Param::Drive_En))
  // {
   //   state = OFF;
  // }

   switch (state)
   {
      default:
      case OFF:
         Param::SetInt(Param::opmode, 0);
         DisableAll();
         PreChTmr=Param::GetInt(Param::PreChT)*10;//reset precharge tmer
         pwrDntmr=10;//reset powerdown timer
         ZeroPower=true;//charger power =0 in off.
         if (CheckStartCondition())
         {
            startTime = rtc_get_counter_val();
            state = WAITSTART;
         }
        if(Param::GetBool(Param::Drive_En)) state = DRIVE;
         break;
      case WAITSTART:
         if (CheckDelay()) state = ENABLE;
         if (CheckUnplugged()) state = OFF;
         break;
      case ENABLE:
         DigIo::hvena_out.Set();
         PreChTmr--;//decrement precharge timer
         if(PreChTmr==0)
         {
         DigIo::pcsena_out.Set();
         CAN_Enable=true;
         state = ACTIVATE;
         }
         break;
      case DRIVE:
         ZeroPower=true;//charger powe =0 in drive.
         DigIo::pcsena_out.Set();
         DigIo::chena_out.Set();//charger off
         Param::SetInt(Param::opmode, 2);
         DigIo::dcdcena_out.Clear();
         CAN_Enable=true;
         if (!Param::GetBool(Param::Drive_En)) state = OFF;
         break;
      case ACTIVATE:
         Param::SetInt(Param::opmode, 1);
         startTime = rtc_get_counter_val();
         if (activate & 1)
            DigIo::chena_out.Clear();
         else
            DigIo::chena_out.Set();
         if (activate & 2)
            DigIo::dcdcena_out.Clear();
         else
            DigIo::dcdcena_out.Set();
         state = EVSEACTIVATE;
         break;
      case EVSEACTIVATE:
          if(PCS_CHG_Status==3)//Activate EVSE when PCS enters wait for AC state.
          {
         DigIo::evseact_out.Set();
         DigIo::acpres_out.Set();
         ZeroPower=false;//release zero charger power clamp
          }
         if (CheckVoltage() || CheckTimeout())
         {
             ZeroPower=true;
             state = STOP;
         }

         if (CheckUnplugged())
         {
            ZeroPower=true;
            DigIo::acpres_out.Clear();
            DigIo::evseact_out.Clear();
            state = STOP;
         }
         if (CheckChargerFaults())
         {
            ZeroPower=true;
            DigIo::acpres_out.Clear();
            state = STOP;
         }
         if (!Param::GetBool(Param::enable)) state = STOP;
         break;
      case STOP:
         ZeroPower=true;
         if(pwrDntmr!=0) pwrDntmr--;
         if(pwrDntmr==0)
         {
         DisableAll();
         Param::SetInt(Param::opmode, 0);
         if (CheckUnplugged()) state = OFF;
         }

         break;
   }

   Param::SetInt(Param::state, state);
}


uint16_t ChgPwrRamp()
{
uint8_t Charger_state=Param::GetInt(Param::CHG_STAT);
uint16_t Charger_Pwr_Max=Param::GetInt(Param::pacspnt)*1000;
if(Charger_state!=6) ChgPower=0; //Set power =0 unless charger is enabled.
if(ZeroPower) ChgPower=0;
else
{
if(ChgPower<Charger_Pwr_Max)ChgPower+=10;
if(ChgPower>Charger_Pwr_Max)ChgPower-=10;
}

return ChgPower;
}


static void Ms10Task(void)
{
    if(CAN_Enable)
    {
    //Send 10ms PCS CAN when enabled.
    PCSCan::Msg13D();
    PCSCan::Msg22A();
    PCSCan::Msg3B2();

    }

}

static void Ms50Task(void)
{
    if(CAN_Enable)
    {
    //Send 50ms PCS CAN when enabled.
    PCSCan::Msg545();
    }
}

//sample 100ms task
static void Ms100Task(void)
{
   DigIo::led_out.Toggle();
   //The boot loader enables the watchdog, we have to reset it
   //at least every 2s or otherwise the controller is hard reset.
   iwdg_reset();
   //Calculate CPU load. Don't be surprised if it is zero.
   float cpuLoad = scheduler->GetCpuLoad() / 10.0f;
   //This sets a fixed point value WITHOUT calling the parm_Change() function
   Param::SetFloat(Param::cpuload, cpuLoad);
   //Set timestamp of error message
   ErrorMessage::SetTime(rtc_get_counter_val());
   Param::SetInt(Param::uptime, rtc_get_counter_val());
   Param::SetFloat(Param::uaux, AnaIn::uaux.Get() / 223.418f);
   EvseRead();
   //ResetValuesInOffMode();
   PCSEnable();
  // CalcAcCurrentLimit();
   ChargerStateMachine();
   PCSCan::AlertHandler();


    if(CAN_Enable)
    {
//Send 100ms PCS CAN when enabled.
   PCSCan::Msg20A();
   PCSCan::Msg212();
   PCSCan::Msg21D();
   PCSCan::Msg232();
   PCSCan::Msg23D();
   PCSCan::Msg25D();
   PCSCan::Msg2B2(ChgPwrRamp());
   PCSCan::Msg321();
   PCSCan::Msg333();
   PCSCan::Msg3A1();
    }

}





/** This function is called when the user changes a parameter */
void Param::Change(Param::PARAM_NUM paramNum)
{

   s32fp spnt;

   switch (paramNum)
   {


      default:
         //Handle general parameter changes here. Add paramNum labels for handling specific parameters
         break;
   }
}

static void CanCallback(uint32_t id, uint32_t data[2]) //This is where we go when a defined CAN message is received.
{
    switch (id)
    {
    case 0x204:
        PCSCan::handle204(data);//
        break;
    case 0x224:
        PCSCan::handle224(data);//
        break;
    case 0x264:
        PCSCan::handle264(data);//
        break;
    case 0x2A4:
        PCSCan::handle2A4(data);//
        break;
    case 0x2C4:
        PCSCan::handle2C4(data);//
        break;
    case 0x3A4:
        PCSCan::handle3A4(data);//
        break;
    case 0x424:
        PCSCan::handle424(data);//
        break;
    case 0x504:
        PCSCan::handle504(data);//
        break;
    case 0x76C:
        PCSCan::handle76C(data);//
        break;
    default:


        break;
    }
}


//Whichever timer(s) you use for the scheduler, you have to
//implement their ISRs here and call into the respective scheduler
extern "C" void tim2_isr(void)
{
   scheduler->Run();
}

extern "C" int main(void)
{
   extern const TERM_CMD termCmds[];

   clock_setup(); //Must always come first
   rtc_setup();
   ANA_IN_CONFIGURE(ANA_IN_LIST);
   DIG_IO_CONFIGURE(DIG_IO_LIST);
   AnaIn::Start(); //Starts background ADC conversion via DMA
   write_bootloader_pininit(); //Instructs boot loader to initialize certain pins
   gpio_primary_remap(AFIO_MAPR_SWJ_CFG_JTAG_OFF_SW_ON, AFIO_MAPR_CAN1_REMAP_PORTB);
   tim_setup(); //Use timer3 for sampling pilot PWM
   nvic_setup(); //Set up some interrupts
   parm_load(); //Load stored parameters
   Param::Change(Param::idckp); //Call callback once for parameter propagation
   Param::Change(Param::idclim); //Call callback once for parameter propagation


   Stm32Scheduler s(TIM2); //We never exit main so it's ok to put it on stack
   scheduler = &s;
   //Initialize CAN1, including interrupts. Clock must be enabled in clock_setup()
   Can c(CAN1, Can::Baud500, true);
   c.SetNodeId(5);
   //store a pointer for easier access
   can = &c;

    // Set up CAN  callback and messages to listen for
    c.SetReceiveCallback(CanCallback);
    c.RegisterUserMessage(0x204);//PCS Charge Status
    c.RegisterUserMessage(0x224);//PCS DCDC Status
    c.RegisterUserMessage(0x264);//PCS Chg Line Status
    c.RegisterUserMessage(0x2A4);//PCS Temps
    c.RegisterUserMessage(0x2C4);//PCS Logging
    c.RegisterUserMessage(0x3A4);//PCS Alert Matrix
    c.RegisterUserMessage(0x424);//PCS Alert Log
    c.RegisterUserMessage(0x504);//PCS Boot ID
    c.RegisterUserMessage(0x76C);//PCS Debug output





   Terminal t3(USART3, termCmds);

   //Up to four tasks can be added to each timer scheduler
   //AddTask takes a function pointer and a calling interval in milliseconds.
   //The longest interval is 655ms due to hardware restrictions
   //You have to enable the interrupt (int this case for TIM2) in nvic_setup()
   //There you can also configure the priority of the scheduler over other interrupts
    s.AddTask(Ms100Task, 100);
    s.AddTask(Ms50Task, 50);
    s.AddTask(Ms10Task, 10);

   //backward compatibility, version 4 was the first to support the "stream" command
   Param::SetInt(Param::version, 4);

   //In version 1.11 this changed from mV to V
   if (Param::GetInt(Param::udcspnt) > 420)
   {
      Param::SetFloat(Param::udcspnt, Param::GetFloat(Param::udcspnt) / 1000);
   }

   //Now all our main() does is running the terminal
   //All other processing takes place in the scheduler or other interrupt service routines
   //The terminal has lowest priority, so even loading it down heavily will not disturb
   //our more important processing routines.
   while(1)
   {
      t3.Run();
   }


   return 0;
}

