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
#include <libopencm3/cm3/common.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/dma.h>
#include <libopencm3/stm32/rtc.h>
#include <libopencm3/stm32/crc.h>
#include <libopencm3/stm32/flash.h>
#include <libopencm3/stm32/desig.h>
#include "hwdefs.h"
#include "hwinit.h"
#include "stm32_loader.h"
#include "my_string.h"

/**
* Start clocks of all needed peripherals
*/
void clock_setup(void)
{
   RCC_CLOCK_SETUP();

   //The reset value for PRIGROUP (=0) is not actually a defined
   //value. Explicitly set 16 preemtion priorities
   SCB_AIRCR = SCB_AIRCR_VECTKEY | SCB_AIRCR_PRIGROUP_GROUP16_NOSUB;

   rcc_periph_clock_enable(RCC_GPIOA);
   rcc_periph_clock_enable(RCC_GPIOB);
   rcc_periph_clock_enable(RCC_GPIOC);
   rcc_periph_clock_enable(RCC_GPIOD);
   rcc_periph_clock_enable(RCC_USART1);
   rcc_periph_clock_enable(RCC_USART3);
   rcc_periph_clock_enable(RCC_TIM2); //Scheduler
   rcc_periph_clock_enable(RCC_TIM3); //Proximity PWM sense
   rcc_periph_clock_enable(RCC_DMA1); //ADC and UART
   rcc_periph_clock_enable(RCC_ADC1);
   rcc_periph_clock_enable(RCC_CRC);
   rcc_periph_clock_enable(RCC_AFIO); //CAN
   rcc_periph_clock_enable(RCC_CAN1); //CAN
}

/* Some pins should never be left floating at any time
 * Since the bootloader delays firmware startup by a few 100ms
 * We need to tell it which pins we want to initialize right
 * after startup
 */
void write_bootloader_pininit()
{
   uint32_t flashSize = desig_get_flash_size();
   uint32_t pindefAddr = FLASH_BASE + flashSize * 1024 - PINDEF_BLKNUM * PINDEF_BLKSIZE;
   const struct pincommands* flashCommands = (struct pincommands*)pindefAddr;

   struct pincommands commands;

   memset32((int*)&commands, 0, PINDEF_NUMWORDS);

   //!!! Customize this to match your project !!!
   //Here we specify that PC13 be initialized to ON
   //AND PB1 AND PB2 be initialized to OFF
   commands.pindef[0].port = GPIOC;
   commands.pindef[0].pin = GPIO13;
   commands.pindef[0].inout = PIN_OUT;
   commands.pindef[0].level = 1;
   commands.pindef[1].port = GPIOB;
   commands.pindef[1].pin = GPIO1 | GPIO2;
   commands.pindef[1].inout = PIN_OUT;
   commands.pindef[1].level = 0;

   crc_reset();
   uint32_t crc = crc_calculate_block(((uint32_t*)&commands), PINDEF_NUMWORDS);
   commands.crc = crc;

   if (commands.crc != flashCommands->crc)
   {
      flash_unlock();
      flash_erase_page(pindefAddr);

      //Write flash including crc, therefor <=
      for (uint32_t idx = 0; idx <= PINDEF_NUMWORDS; idx++)
      {
         uint32_t* pData = ((uint32_t*)&commands) + idx;
         flash_program_word(pindefAddr + idx * sizeof(uint32_t), *pData);
      }
      flash_lock();
   }
}

/**
* Enable Timer refresh and break interrupts
*/
void nvic_setup(void)
{
   nvic_enable_irq(NVIC_TIM2_IRQ); //Scheduler
   nvic_set_priority(NVIC_TIM2_IRQ, 0xe << 4); //second lowest priority
}

void rtc_setup()
{
   //Base clock is HSE/128 = 8MHz/128 = 62.5kHz
   //62.5kHz / (62499 + 1) = 1Hz
   rtc_auto_awake(RCC_HSE, 62499); //1000ms tick
   rtc_set_counter_val(0);
}

/**
* Setup timer for measuring 1 Khz Pilot dutycycle
*/
void tim_setup()
{
   timer_set_prescaler(TIM3, 71); //run at 1 MHz
   timer_set_period(TIM3, 65535);
   timer_direction_up(TIM3);
   timer_slave_set_mode(TIM3, TIM_SMCR_SMS_RM);
   timer_slave_set_polarity(TIM3, TIM_ET_FALLING);
   timer_slave_set_trigger(TIM3, TIM_SMCR_TS_TI1FP1);
   timer_ic_set_filter(TIM3, TIM_IC1, TIM_IC_DTF_DIV_32_N_8);
   timer_ic_set_filter(TIM3, TIM_IC2, TIM_IC_DTF_DIV_32_N_8);
   timer_ic_set_input(TIM3, TIM_IC1, TIM_IC_IN_TI1);//measures frequency
   timer_ic_set_input(TIM3, TIM_IC2, TIM_IC_IN_TI1);//measure duty cycle
   timer_set_oc_polarity_high(TIM3, TIM_OC1);
   timer_set_oc_polarity_low(TIM3, TIM_OC2);
   timer_ic_enable(TIM3, TIM_IC1);
   timer_ic_enable(TIM3, TIM_IC2);
   timer_generate_event(TIM3, TIM_EGR_UG);
   timer_enable_counter(TIM3);
}

