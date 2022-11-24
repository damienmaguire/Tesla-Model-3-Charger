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

/* This file contains all parameters used in your project
 * See main.cpp on how to access them.
 * If a parameters unit is of format "0=Choice, 1=AnotherChoice" etc.
 * It will be displayed as a dropdown in the web interface
 * If it is a spot value, the decimal is translated to the name, i.e. 0 becomes "Choice"
 * If the enum values are powers of two, they will be displayed as flags, example
 * "0=None, 1=Flag1, 2=Flag2, 4=Flag3, 8=Flag4" and the value is 5.
 * It means that Flag1 and Flag3 are active -> Display "Flag1 | Flag3"
 *
 * Every parameter/value has a unique ID that must never change. This is used when loading parameters
 * from flash, so even across firmware versions saved parameters in flash can always be mapped
 * back to our list here. If a new value is added, it will receive its default value
 * because it will not be found in flash.
 * The unique ID is also used in the CAN module, to be able to recover the CAN map
 * no matter which firmware version saved it to flash.
 * Make sure to keep track of your ids and avoid duplicates. Also don't re-assign
 * IDs from deleted parameters because you will end up loading some random value
 * into your new parameter!
 * IDs are 16 bit, so 65535 is the maximum
 */

//Define a version string of your firmware here
#define VER 1.01.R

/* Entries must be ordered as follows:
   1. Saveable parameters (id != 0)
   2. Temporary parameters (id = 0)
   3. Display values
 */
//Next param id (increase when adding new parameter!): 24
//Next value Id: 2051
/*              category     name         unit       min     max     default id */
#define PARAM_LIST \
    PARAM_ENTRY(CAT_CHARGER, pacspnt,     "kW",      0,      11,     11,     2  ) \
    PARAM_ENTRY(CAT_CHARGER, idclim,      "A",       0,      45,     45,     3   ) \
    PARAM_ENTRY(CAT_CHARGER, iaclim,      "A",       0,      72,     16,     4  ) \
    PARAM_ENTRY(CAT_CHARGER, idcspnt,     "A",       0,      45,     45,     5   ) \
    PARAM_ENTRY(CAT_CHARGER, udcspnt,     "V",       50,     420,    403,    6   ) \
    PARAM_ENTRY(CAT_CHARGER, udclim,      "V",       50,     420,    398,    7   ) \
    PARAM_ENTRY(CAT_CHARGER, timelim,     "minutes", -1,     10000,  -1,     8  ) \
    PARAM_ENTRY(CAT_CHARGER, timedly,     "minutes", -1,     10000,  -1,     9  ) \
    PARAM_ENTRY(CAT_CHARGER, inputype,    INPUTS,    0,      3,      1,      10  ) \
    PARAM_ENTRY(CAT_CHARGER, cancontrol,  OFFON,     0,      1,      0,      11  ) \
    PARAM_ENTRY(CAT_CHARGER, idckp,       "",        0,      10000,  1,      12  ) \
    PARAM_ENTRY(CAT_CHARGER, idcki,       "",        0,      10000,  10,     13  ) \
    PARAM_ENTRY(CAT_CHARGER, activate,    DEVS,      0,      3,      3,      14  ) \
    PARAM_ENTRY(CAT_DCDC,    udcdc,       "V",       12,     15,     14,     15  ) \
    PARAM_ENTRY(CAT_DCDC,    PreChT,      "sec",     1,      10,     3,      23  ) \
    PARAM_ENTRY(CAT_GEN,     Alerts,      "",        0,      9,      0,      16  ) \
    PARAM_ENTRY(CAT_GEN,     AlertLog,    OFFON,     0,      1,      1,      17  ) \
    VALUE_ENTRY(opmode,      OPMODES, 2000 ) \
    VALUE_ENTRY(state,       STATES,  2001 ) \
    VALUE_ENTRY(version,     VERSTR,  2003 ) \
    VALUE_ENTRY(uptime,      "s",     2048 ) \
    VALUE_ENTRY(lasterr,     errorListString,  2002 ) \
    VALUE_ENTRY(uaux,        "V",     2005 ) \
    VALUE_ENTRY(useraclim,   "W",     2006 ) \
    VALUE_ENTRY(hwaclim,     "A",     2007 ) \
    VALUE_ENTRY(cablelim,    "A",     2008 ) \
    VALUE_ENTRY(evselim,     "A",     2009 ) \
    VALUE_ENTRY(powerac,     "kW",    2010 ) \
    VALUE_ENTRY(powerdcdc,   "W",     2011 ) \
    VALUE_ENTRY(udc,         "V",     2012 ) \
    VALUE_ENTRY(ulv,         "V",     2013 ) \
    VALUE_ENTRY(uac,         "V",     2014 ) \
    VALUE_ENTRY(iac,         "A",     2015 ) \
    VALUE_ENTRY(idc,         "A",     2016 ) \
    VALUE_ENTRY(idcdc,       "A",     2017 ) \
    VALUE_ENTRY(ChgACLim,    "A",     2018 ) \
    VALUE_ENTRY(PCS_Type,    TYPES,   2019 ) \
    VALUE_ENTRY(CHG_STAT,    C_STAT,  2025 ) \
    VALUE_ENTRY(CHGPAvail,   "kW",    2026 ) \
    VALUE_ENTRY(GridCFG,     GCFG,    2027 ) \
    VALUE_ENTRY(proximity,   OFFON,   2020 ) \
    VALUE_ENTRY(enable,      OFFON,   2021 ) \
    VALUE_ENTRY(Drive_En,    OFFON,   2038 ) \
    VALUE_ENTRY(canenable,   OFFON,   2022 ) \
    VALUE_ENTRY(ChgATemp,    "C",    2031 ) \
    VALUE_ENTRY(ChgBTemp,    "C",    2032 ) \
    VALUE_ENTRY(ChgCTemp,    "C",    2033 ) \
    VALUE_ENTRY(DCDCTemp,    "C",    2034 ) \
    VALUE_ENTRY(DCDCBTemp,   "C",    2035 ) \
    VALUE_ENTRY(PCSAmbTemp,  "C",    2036 ) \
    VALUE_ENTRY(modelcode,   MODCODES,2023 ) \
    VALUE_ENTRY(PCSBoot,      "dig"   ,2028 ) \
    VALUE_ENTRY(PCSAlerts,    ALERTS  ,2029 ) \
    VALUE_ENTRY(PCSAlertCnt,  "dig"  ,2037 ) \
    VALUE_ENTRY(cpuload,     "%",     2024 ) \



/***** Enum String definitions *****/
#define OPMODES      "0=Off, 1=Run, 2=Drive"
#define CHARGERS     "1=Charger1, 2=Charger2, 4=Charger3"
#define C_STAT       "0=Init, 1=Idle, 2=Startup, 3=WaitAC, 4=Qualify, 5=Config, 6=Enable, 7=Shutdown, 8=Faulted, 9=CLRFaults"
#define OFFON        "0=Off, 1=On"
#define CHFLAGS      "0=None, 1=Enabled, 2=Fault, 4=CheckAlive"
#define TYPES        "0=48A_1P, 1=32A_1P, 2=16A_3P"
#define GCFG         "0=None, 1=1P, 2=3P, 3=3PD"
#define STATES       "0=Off, 1=WaitStart, 2=Enable, 3=Activate, 4=Run, 5=Stop, 6=DRIVE"
#define INPUTS       "0=Type2, 2=Type1, 3=Manual"
#define POLARITIES   "0=ActiveHigh, 1=ActiveLow"
#define CAT_TEST     "Testing"
#define CAT_CHARGER  "Charger"
#define CAT_COMM     "Communication"
#define DEVS         "0=None, 1=Charger, 2=DC-DC, 3=Both"
#define MODELS       "0=EU, 1=US"
#define MODCODES     "45=EU, 93=US"
#define CAT_TEST     "Testing"
#define CAT_CHARGER  "Charger"
#define CAT_DCDC     "DC/DC Converter"
#define CAT_GEN      "General"
#define ALERTS       "0=None, 1=01chgHwInputOc, 2=02chgHwOutputOc, 3=03chgHwInputOv, 4=04chgHwIntBusOv, 5=05chgOutputOv, 6=06chgPrechargeFailedScr, 7=07chgPhaseTempHot, 8=08chgPhaseOverTemp, 9=09chgPfcCurrentRegulation, 10=10chgIntBusVRegulation, 11=11chgLlcCurrentRegulation, 12=12chgPfcIBandTracerFault, 13=13chgPrechargeFailedBoost, 14=14chgTempRationality, 15=15chg12vUv, 16=16chgAllPhasesFaulted, 17=17chgWallPowerRemoval, 18=18chgUnknownGridConfig, 19=19acChargePowerLimited, 20=20chgEnableLineMismatch, 21=21hvpMia, 22=22bmsMia, 23=23cpMia, 24=24vcfrontMia, 25=25cpu2Malfunction, 26=26watchdogAlarmed, 27=27chgInsufficientCooling, 28=28chgOutputUv, 29=29chgPowerRationality, 30=30canRationality, 31=31uiMia, 32=32unused, 33=33hvBusUv, 34=34hvBusOv, 35=35lvBusUv, 36=36lvBusOv, 37=37resonantTankOc, 38=38claFaulted, 39=39sdModuleClkFault, 40=40dcdcMaxPowerReached, 41=41dcdcOverTemp, 42=42dcdcEnableLineMismatch, 43=43hvBusPrechargeFailure, 44=4412vSupportRegulation, 45=45hvBusLowImpedance, 46=46hvBusHighImpedence, 47=47lvBusLowImpedance, 48=48lvBusHighImpedance, 49=49dcdcTempRationality, 50=50dcdc12VsupportFaulted, 51=51chgIntBusUv, 52=52acVoltageNotPresent, 53=53chgInputVDropHigh, 54=54chgInputVDropTooHigh, 55=55chgLineImedanceHigh, 56=56chgLineImedanceTooHigh, 57=57chgInputOverFreq, 58=58chgInputUnderFreq, 59=59chgInputOvRms, 60=60chgInputOvPeak, 61=61chgVLineRationality, 62=62chgILineRationality, 63=63chgVOutRationality, 64=64chgIOutRationality, 65=65chgPllNotLocked, 66=66dcdcHvRationality, 67=67dcdcLvRationality, 68=68dcdcTankvRationality, 69=69chgPfcLineDidt, 70=70chgPfcLineDvdt, 71=71chgPfcILoopRationality, 72=72cpu2ClaStopped, 73=73unexpectedAcInputVoltage, 74=74hvBusDischargeFailure, 75=75hvBusDischargeTimeout, 76=76dcdcEnDeassertedErr, 77=77microGridEnergyLow, 78=78chgStopDcdcTooHot, 79=79eepromOperationError, 80=80damagedPhaseDetected, 81=81dcdcPchgTimeout, 82=82dcdcPchgUnsafeDiVoltage, 83=83triggerOdin, 84=84unused, 85=85dcdcFetsNotSwitching, 86=86dcdcInsufficientCooling, 87=87nvramRecordStatusError, 88=88pchgParameters, 89=89hvBusDischargeIrrational, 90=90expectedAcVoltageSourceMissing, 91=91chgIntBusRationality, 92=92chgPowerLimitedByBusRipple, 93=93powerRailRationality,  94=94pcsDcdcNeedService, 95=95dcdcSensorlessModeActive, 96=96microGridOverLoaded, 97=97rebootPhaseDetected, 98=98gridFreqDroopDetectedSilent, 99=99microGridOverLoadedSilent, 100=100microGridEnergyLowSilent, 101=101phMachineModelIrrational, 102=102resetWithDCDCCmdAsserted"

#define VERSTR STRINGIFY(4=VER)

/***** enums ******/

enum inputs
{
   INP_TYPE2,
   INP_TYPE2_3P,
   INP_TYPE1,
   INP_MANUAL,
   INP_MANUAL_3P,
   INP_TYPE2_AUTO
};

enum states
{
   OFF,
   WAITSTART,
   ENABLE,
   ACTIVATE,
   EVSEACTIVATE,
   DRIVE_START,
   DRIVE,
   STOP
};

enum _canspeeds
{
   CAN_PERIOD_100MS = 0,
   CAN_PERIOD_10MS,
   CAN_PERIOD_LAST
};

enum _modes
{
   MOD_OFF = 0,
   MOD_RUN,
   MOD_LAST
};

enum _chflags
{
   FLAG_NONE = 0,
   FLAG_ENABLED = 1,
   FLAG_FAULT = 2,
   FLAG_CHECK = 4
};

//Generated enum-string for possible errors
extern const char* errorListString;

