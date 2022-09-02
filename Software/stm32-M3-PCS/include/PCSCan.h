#ifndef PCSCan_h
#define PCSCan_h

/*  This library supports the messages for the Tesla Model 3 PCS

*/

#include <stdint.h>
#include "stm32_can.h"
#include "params.h"
#include "digio.h"


class PCSCan
{

public:

    static		void Msg20A();
    static		void Msg212();
    static		void Msg21D();
    static		void Msg22A();
    static		void Msg232();
    static		void Msg23D();
    static		void Msg25D();
    static		void Msg2B2();
    static		void Msg321();
    static		void Msg333();
    static		void Msg3A1();
    static		void Msg3B2();
    static		void Msg545();

    static void handle204(uint32_t data[2]);
    static void handle224(uint32_t data[2]);
    static void handle264(uint32_t data[2]);
    static void handle2A4(uint32_t data[2]);
    static void handle2C4(uint32_t data[2]);
    static void handle3A4(uint32_t data[2]);
    static void handle424(uint32_t data[2]);
    static void handle504(uint32_t data[2]);
    static void handle76C(uint32_t data[2]);
    static void AlertHandler();



private:

};

    static uint8_t CalcPCSChecksum(uint8_t *data, uint16_t id);
    static int16_t ProcessTemps(uint16_t InVal);

#endif /* PCSCan_h */
