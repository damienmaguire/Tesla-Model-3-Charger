#include "PCSCan.h"


bool mux3b2=true;
bool mux545=true;
uint16_t PCS_Power_Req=0;
uint16_t DCDCSpnt=0;
uint16_t DCDCAmps=0;
uint16_t ACLim=0;
float ACPwr=0;
uint16_t ACVolts=0;
uint16_t ACAmps=0;
uint16_t HVVolts=0;
float LVVolts=0;
float DCDCPwr=0;
int16_t TempLocal=0;
uint16_t AmbTemp=0;
uint16_t ATemp=0;
uint16_t BTemp=0;
uint16_t CTemp=0;
uint16_t DCDCTemp=0;
uint16_t DCDCBTemp=0;
uint8_t ACILim=0;
uint8_t Count545=0;
float ChgPavail=0;
float IOut_PhA=0;
float IOut_PhB=0;
float IOut_PhC=0;
float IOut_Total=0;
uint8_t PCSGrid=0;
uint8_t PCS_HW=0;
uint8_t PCS_CHG_STAT=0;
uint8_t mux2C4=0;
uint8_t mux76C=0;
uint8_t PCSBootId=0;
uint8_t PCSAlertPage=0;
uint8_t PCSAlertId=0;
uint8_t PCSAlertSource=0;
uint8_t PCS_AlertCnt=0;
static uint8_t pcs_alert_matrix[10]= {0,0,0,0,0,0,0,0,0,0};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////PCS CAN Messages To Receive
///////////////////////////////////////////////////////////////////////////

void PCSCan::handle204(uint32_t data[2])  //PCS Chg status.
                                          //Power,Amps,PCS config,grid stat
{
    uint8_t* bytes = (uint8_t*)data;      //byte 0 bits 4,5 = Chg state. 0=standby, 1=blocked, 2=enabled,3=faulted.
                                          //byte 0 bits 6,7 = grid config. 0=none, 1=single phase, 2=three phase, 3=three phase delta


    PCS_HW=(bytes[7]>>3)&0x03;            //byte 7 bits 3,4 = HW type. 0=48A single phase , 1=32A single phase, 2=Three phase.
    Param::SetInt(Param::PCS_Type, PCS_HW);
    if(PCS_HW==0)Param::SetInt(Param::hwaclim,48);
    if(PCS_HW==1)Param::SetInt(Param::hwaclim,32);
    if(PCS_HW==2)Param::SetInt(Param::hwaclim,16);

    PCS_CHG_STAT=(bytes[0])&0x0f;          //byte 0 bits 0-3 = charger main state. 0=init, 1=idle, 2=startup, 3=wait for line voltage
                                          //4=qualify line config, 5=enable, 7=shutdown, 8=faulted, 9=clear faults
    Param::SetInt(Param::CHG_STAT, PCS_CHG_STAT);

    ChgPavail=bytes[3]*0.1f;                      //byte 2 = available charger power x 0.1 in kw.
    Param::SetFloat(Param::CHGPAvail, ChgPavail);

    PCSGrid=(bytes[0]>>6)&0x3;                   //byte 0 bits 6,7 = grid config. 0=none, 1=single phase, 2=three phase, 3=three phase delta
    Param::SetInt(Param::GridCFG, PCSGrid);
}

void PCSCan::handle224(uint32_t data[2])  //DCDC Info

{
    uint8_t* bytes = (uint8_t*)data;


    DCDCAmps = (((bytes[3]<<8 | bytes[2])&0xFFF)*0.1);//dcdc actual current. 12 bit unsigned int in bits 16-27. scale 0.1.
    Param::SetFloat(Param::idcdc, DCDCAmps);
    DCDCPwr=DCDCAmps*LVVolts;
    Param::SetFloat(Param::powerdcdc, DCDCPwr);


}

void PCSCan::handle264(uint32_t data[2])  //PCS Chg Line Status

{
    uint8_t* bytes = (uint8_t*)data;//

  ACLim = (((bytes[5]<<8 | bytes[4])&0x3ff)*0.1);
  ACPwr = ((bytes[3])*.1f);
  ACVolts = (((bytes[1]<<8 | bytes[0])&0x3FFF)*0.033);
  ACAmps = (((bytes[2]<<9 | bytes[1])>>7)*0.1);

  Param::SetFloat(Param::powerac, ACPwr);
  Param::SetFloat(Param::uac, ACVolts);
  Param::SetFloat(Param::iac, ACAmps);
  Param::SetFloat(Param::ChgACLim, ACLim);

}

void PCSCan::handle2A4(uint32_t data[2])  //PCS Temps

{
    uint8_t* bytes = (uint8_t*)data;
                                        //PCS_ambientTemp bits 44 to 54 x 0.1
                                        //PCS_chgPhATemp bits 0-10 x0.1
                                        //PCS_chgPhbTemp bits 11-21 x0.1
                                        //PCS_chgPhcTemp bits 22-32 x0.1
                                        //DCDC Temp bits 33-43 x0.1

    ATemp=((bytes[1]<<8 | bytes[0]));
    BTemp=((bytes[2]<<8 | bytes[1])>>3);
    CTemp=((bytes[4]<<15 | bytes[3]<<7 | bytes[2]>>1)>>5);
    DCDCTemp=((bytes[5]<<8 | bytes[4])>>1);
    DCDCBTemp=(((bytes[7]<<8| bytes[6])>>7)&0x1FF)*0.293542;
    AmbTemp=((bytes[6]<<8 | bytes[5])>>4);
    Param::SetFloat(Param::ChgATemp, ProcessTemps(ATemp));
    Param::SetFloat(Param::ChgBTemp, ProcessTemps(BTemp));
    Param::SetFloat(Param::ChgCTemp, ProcessTemps(CTemp));
    Param::SetFloat(Param::DCDCTemp, ProcessTemps(DCDCTemp));
    Param::SetFloat(Param::DCDCBTemp, DCDCBTemp);
    Param::SetFloat(Param::PCSAmbTemp, ProcessTemps(AmbTemp));

}


void PCSCan::handle2C4(uint32_t data[2])  //PCS Logging

{
    uint8_t* bytes = (uint8_t*)data;
    mux2C4=(bytes[0]);
    if((mux2C4==0xE6) || (mux2C4==0xC6))//if in mux 6 grab the info...
    {
     HVVolts = (((bytes[3]<<8 | bytes[2])&0xFFF)*0.146484); //measured hv voltage. 12 bit unsigned int in bits 16-27. scale 0.146484.
     LVVolts = ((((bytes[1]<<9 | bytes[0])>>6))*0.0390625f); //measured lv voltage. 10 bit unsigned int in bits 5-14. scale 0.0390626.
    }
    Param::SetFloat(Param::udc, HVVolts);
    Param::SetFloat(Param::ulv, LVVolts);


}

void PCSCan::handle3A4(uint32_t data[2])  //PCS Alert Matrix

{//0=None, 1=CP_MIA, 2=BMS_MIA, 3=HVP_MIA, 4=UNEX_AC, 5=CHG_VRAT, 6=DCDC_VRAT, 7=VCF_MIA, 8=CAN_RAT, 9=UI_MIA
    uint8_t* bytes = (uint8_t*)data;
    PCSAlertPage = bytes[0]&0xf;



}

void PCSCan::handle424(uint32_t data[2])  //PCS Alert Log

{
    uint8_t* bytes = (uint8_t*)data;
    PCSAlertId=bytes[0];
    if(Param::GetBool(Param::AlertLog))
    {
    pcs_alert_matrix[PCS_AlertCnt]=PCSAlertId;
    PCS_AlertCnt++;
    if(PCS_AlertCnt>10)PCS_AlertCnt=0;
    Param::SetInt(Param::PCSAlertCnt, PCS_AlertCnt);
    }
}

void PCSCan::handle504(uint32_t data[2])  //PCS Boot ID

{
    uint8_t* bytes = (uint8_t*)data;
    PCSBootId=bytes[7];
    Param::SetInt(Param::PCSBoot, PCSBootId);

}

void PCSCan::handle76C(uint32_t data[2])  //PCS Debug output

{
    uint8_t* bytes = (uint8_t*)data;//Mux id in byte 0.
                                        //Mux 0x0C(12) = chg phase A outputs
                                        //Mux 0x16(22) = chg phase B outputs
                                        //Mux 0x20(32) = chg phase C outputs
                                        //IOout=bytes 2,3 14 bit unsigned scale 0.0025
    mux76C=(bytes[0]);
    if(mux76C==0x0C)                    //Calculate total DC output current from all 3 charger modules.
    {
      IOut_PhA=((bytes[2]<<8 | bytes[1])&0x3ff)*0.0025f;
    }
    else if(mux76C==0x16)
    {
      IOut_PhB=((bytes[2]<<8 | bytes[1])&0x3ff)*0.0025f;
    }
    else if(mux76C==0x20)
    {
      IOut_PhC=((bytes[2]<<8 | bytes[1])&0x3ff)*0.0025f;
    }

    IOut_Total=IOut_PhA+IOut_PhB+IOut_PhC;
    Param::SetFloat(Param::idc, IOut_Total);

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////PCS CAN Messages to Send
///////////////////////////////////////////////////////////////////////////////////////

void PCSCan::Msg20A()
{  //HVP contactor state. Static msg.
   uint8_t bytes[6];
   bytes[0]=0xF6;
   bytes[1]=0x15;
   bytes[2]=0x09;
   bytes[3]=0x82;
   bytes[4]=0x18;
   bytes[5]=0x01;
   Can::GetInterface(0)->Send(0x20A, (uint32_t*)bytes,6);
}

void PCSCan::Msg212()
{  //BMS status. Static msg
   uint8_t bytes[8];
   bytes[0]=0xB9;
   bytes[1]=0x1C;
   bytes[2]=0x94;
   bytes[3]=0xAD;
   bytes[4]=0xC3;
   bytes[5]=0x15;
   bytes[6]=0x06;
   bytes[7]=0x63;
   Can::GetInterface(0)->Send(0x212, (uint32_t*)bytes,8);
}


void PCSCan::Msg21D()
{
   //CP EVSE Status. Populate with Cable lim and pilot lim? I think PCS does not care about Limits here in certain frimware.
   uint8_t bytes[8];
   bytes[0]=0x2D;//2D FOR EU TYPE 2, 5D FOR US TYPE 1 INPUTS
   bytes[1]=Param::GetInt(Param::evselim)*2;//pilot current. 8 bits. scale 0.5.
   bytes[2]=0x00;
   bytes[3]=Param::GetInt(Param::cablelim);//cable current limit. scale 1. 7 bits. 0x20=32Amps
   bytes[4]=0x80;
   bytes[5]=0x00;
   bytes[6]=0x60;
   bytes[7]=0x10;
   Can::GetInterface(0)->Send(0x21D, (uint32_t*)bytes,8);
}

void PCSCan::Msg22A()
{
   //HVP PCS control.
   uint8_t activate = Param::GetInt(Param::activate);
   uint8_t bytes[4];
   bytes[0]=0x00;//precharge request voltage. 16 bit signed int. scale 0.1. Bytes 0 and 1.
   bytes[1]=0x00;
                                                                            //BUG HERE
   if(activate==0)bytes[2]=0x70;//Shutdown both chg and dcdc
   if(activate==1)bytes[2]=0x75;//Charger en
   if(activate==2)bytes[2]=0x79;//DCDC en
   if(activate==3)bytes[2]=0x7D;//Charger en and DCDC en
   bytes[3]=0x17;
   Can::GetInterface(0)->Send(0x22A, (uint32_t*)bytes,4);
}

void PCSCan::Msg232()
{
   //BMS Contactor request.Static msg.
   uint8_t bytes[8];
   bytes[0]=0x0A;
   bytes[1]=0x02;
   bytes[2]=0xD5;
   bytes[3]=0x09;
   bytes[4]=0xCB;
   bytes[5]=0x04;
   bytes[6]=0x00;
   bytes[7]=0x00;
   Can::GetInterface(0)->Send(0x232, (uint32_t*)bytes,8);

}

void PCSCan::Msg23D()
{
   //CP Charge Status
   uint8_t bytes[4];
   ACILim=Param::GetInt(Param::iaclim)*2;
  if(!DigIo::chena_out.Get()) bytes[0]=0x05;//Set charger enabled.
  if(DigIo::chena_out.Get()) bytes[0]=0x0A;//Set charger disabled
   bytes[1]=ACILim; //charge current limit. gain 0.5. 0x40 = 64 dec =32A. Populate AC lim in here.
   bytes[2]=0xFF;//Internal max current limit.
   bytes[3]=0x0F;
   Can::GetInterface(0)->Send(0x23D, (uint32_t*)bytes,4);
}

void PCSCan::Msg25D()
{
   //CP Status. Only byte 0 bits 0 and 1 are important to the PCS
   uint8_t bytes[8];
   bytes[0]=0xD9; //D9 FOR EURO. D8 FOR US. 0=US Tesla , 1=Euro IEC , 2=GB, 3=IEC CCS. Must match PCS type!
   bytes[1]=0x8C;
   bytes[2]=0x01;
   bytes[3]=0xB5;
   bytes[4]=0x4A;
   bytes[5]=0xC1;
   bytes[6]=0x0A;
   bytes[7]=0xE0;
   Can::GetInterface(0)->Send(0x25D, (uint32_t*)bytes,8);
}

void PCSCan::Msg2B2()
{
   //Charge Power Request
   PCS_Power_Req=Param::GetFloat(Param::pacspnt)*1000.0f;
   uint8_t bytes[5];//US hvcon sends this as dlc=3. EU sends as dlc=5.So firmware rev change this msg.
                    //A missmatch here will trigger a can rationality error.
   bytes[0]=PCS_Power_Req & 0xFF;//KW scale 0.001 16 bit unsigned in bytes 0 and 1. e.g. 0x0578 = 1400 dec = 1400Watts=1.4kW.
   bytes[1]=PCS_Power_Req >> 8;
   bytes[2]=0x00;//byte 2 bit 1 may be an ac charge enable in older firmware. Bit 0 is PCS clear fault command in all revs.
   bytes[3]=0x00;
   bytes[4]=0x00;


   Can::GetInterface(0)->Send(0x2B2, (uint32_t*)bytes,5);
}

void PCSCan::Msg321()
{
   uint8_t bytes[8];//VCFront sensors. Static.
   bytes[0]=0x2C;
   bytes[1]=0xB6;
   bytes[2]=0xA8;
   bytes[3]=0x7F;
   bytes[4]=0x02;
   bytes[5]=0x7F;
   bytes[6]=0x00;
   bytes[7]=0x00;
   Can::GetInterface(0)->Send(0x321, (uint32_t*)bytes,8);
}

void PCSCan::Msg333()
{

   uint8_t bytes[4];//UI charge request message. Can be used as an ac current limit.
   bytes[0]=0x04;//leaving at 48A for now.
   bytes[1]=0x30;//byte one. 7 bits scale 1. 0x30=48A.
   bytes[2]=0x29;
   bytes[3]=0x07;
   Can::GetInterface(0)->Send(0x333, (uint32_t*)bytes,4);
}

void PCSCan::Msg3A1()
{
   DCDCSpnt=Param::GetFloat(Param::udcdc)*100.0f;
   uint8_t bytes[8];//VCFront vehicle status
   bytes[0]=0x09;//This message contains the 12v dcdc target setpoint. bits 16-26 as an 11bit unsigned int. scale 0.01
   bytes[1]=0x62;
   bytes[2]=DCDCSpnt & 0xFF;//78 , d gives us a 14v target.
   bytes[3]=((DCDCSpnt >> 8)|0x99);//0x9D;
   bytes[4]=0x08;
   bytes[5]=0x2C;
   bytes[6]=0x12;
   bytes[7]=0x5A;
   Can::GetInterface(0)->Send(0x3A1, (uint32_t*)bytes,8);
}


void PCSCan::Msg3B2()
{
   uint8_t bytes[8];//This msg changed drastically between 2019 and 2020-2021 model firmwares. PCS pays close attention to these two muxes.
    if(mux3b2)//BMS log2 message
    {
   bytes[0]=0xE5;//mux 5=charging.
   bytes[1]=0x0D;
   bytes[2]=0xEB;
   bytes[3]=0xFF;
   bytes[4]=0x0C;
   bytes[5]=0x66;
   bytes[6]=0xBB;
   bytes[7]=0x11;
   Can::GetInterface(0)->Send(0x3B2, (uint32_t*)bytes,8);
   mux3b2=false;
    }
    else
    {
   bytes[0]=0xE3;//mux 3=charge termination
   bytes[1]=0x5D;
   bytes[2]=0xFB;
   bytes[3]=0xFF;
   bytes[4]=0x0C;
   bytes[5]=0x66;
   bytes[6]=0xBB;
   bytes[7]=0x06;
   Can::GetInterface(0)->Send(0x3B2, (uint32_t*)bytes,8);
   mux3b2=true;
    }
}

void PCSCan::Msg545()
{
   uint8_t bytes[8];
    if(mux545)//VCFront
    {
   bytes[0]=0x14;
   bytes[1]=0x00;
   bytes[2]=0x3F;
   bytes[3]=0x70;
   bytes[4]=0x9F;
   bytes[5]=0x01;
   bytes[6]=(Count545<<4)|0xA;
   bytes[7]=CalcPCSChecksum((uint8_t*)bytes, 0x545);
   Can::GetInterface(0)->Send(0x545, (uint32_t*)bytes,8);
   mux545=false;
    }
    else
    {
   bytes[0]=0x03;
   bytes[1]=0x19;
   bytes[2]=0x64;
   bytes[3]=0x32;
   bytes[4]=0x19;
   bytes[5]=0x00;
   bytes[6]=(Count545<<4);
   bytes[7]=CalcPCSChecksum((uint8_t*)bytes, 0x545);
   Can::GetInterface(0)->Send(0x545, (uint32_t*)bytes,8);
   mux545=true;
    }
        Count545++;
        if(Count545>0x0F) Count545=0;
}

void PCSCan::AlertHandler()  //Alert handler

{
    Param::SetInt(Param::PCSAlerts, pcs_alert_matrix[Param::GetInt(Param::Alerts)]);
    if(!Param::GetBool(Param::AlertLog))
    {
       PCS_AlertCnt=0;
       Param::SetInt(Param::PCSAlertCnt, PCS_AlertCnt);
       for(int i=0; i<10; i++)pcs_alert_matrix[i]=0;//Clear log and counter when PCS alert logging disabled
    }

}

 static uint8_t CalcPCSChecksum(uint8_t *bytes, uint16_t id)
{
   uint16_t checksum_calc=0;
   for(int b=0; b<7; b++)
   {
      checksum_calc = checksum_calc + bytes[b];
   }
   checksum_calc += id + (id >> 8);
   checksum_calc &= 0xFF;

   return (uint8_t)checksum_calc;
}


static int16_t ProcessTemps(uint16_t InVal)
{
    int16_t value = InVal & 0x3ff;
    if (InVal & 0x400) value -= 0x3ff;
    value = value * 0.1 + 40;value = InVal & 0x3ff;
    if (InVal & 0x400) value -= 0x3ff;
    value = value * 0.1 + 40;
    return value;
}

