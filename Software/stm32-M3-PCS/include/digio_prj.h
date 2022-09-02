#ifndef PinMode_PRJ_H_INCLUDED
#define PinMode_PRJ_H_INCLUDED

#include "hwdefs.h"

/* Here you specify generic IO pins, i.e. digital input or outputs.
 * Inputs can be floating (INPUT_FLT), have a 30k pull-up (INPUT_PU)
 * or pull-down (INPUT_PD) or be an output (OUTPUT)
*/

#define DIG_IO_LIST \
    DIG_IO_ENTRY(enable_in,   GPIOB, GPIO3,  PinMode::INPUT_FLT)   \
    DIG_IO_ENTRY(d2_in,       GPIOB, GPIO4,  PinMode::INPUT_FLT)   \
    DIG_IO_ENTRY(led_out,     GPIOC, GPIO13, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(hvena_out,   GPIOA, GPIO2,  PinMode::OUTPUT)      \
    DIG_IO_ENTRY(acpres_out,  GPIOA, GPIO3,  PinMode::OUTPUT)      \
    DIG_IO_ENTRY(evseact_out, GPIOB, GPIO15, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(pcsena_out,  GPIOB, GPIO14, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(dcdcena_out, GPIOB, GPIO13, PinMode::OUTPUT)      \
    DIG_IO_ENTRY(chena_out,   GPIOB, GPIO12, PinMode::OUTPUT)      \

#endif // PinMode_PRJ_H_INCLUDED
