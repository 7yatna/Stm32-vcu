#include <i3LIM.h>

static uint8_t CP_Amps=0;
static uint8_t PP_Amps=0;
static uint8_t CP_Mode=0;
static uint8_t CP_Typ=0;
static uint8_t Batt_Cmp=0;
static uint8_t Flap_stat=0;
static uint8_t Cont_stat=0;
static uint8_t Cont_test=0;
static uint16_t Cont_Volts=0;
static uint16_t V_Batt=0;
static uint16_t V_Avail=0;
static uint16_t minV_Avail=0;
static uint16_t I_Avail=0;
static uint16_t CCS_Vmeas=0;
static uint16_t CCS_Imeas=0;
static uint8_t V_Batt2=0;
static int32_t I_Batt=0;
static int32_t I_Math=0;
static uint16_t SOC_Local=0;
static uint16_t Wh_Local=0;
static bool PP=false;
s32fp CHG_Pwr=0; //calculated charge power. 12 bit value scale x25. Values based on 50kw DC fc and 1kw and 3kw ac logs. From bms???
int16_t  FC_Cur=0; //10 bit signed int with the ccs dc current command.scale of 1.
uint8_t  EOC_Time=0x00; //end of charge time in minutes.
uint8_t CHG_Status=0;  //observed values 0 when not charging , 1 and transition to 2 when commanded to charge. only 4 bits used.
                    //seems to control led colour.
uint8_t CHG_Req=0;  //observed values 0 when not charging , 1 when requested to charge. only 1 bit used in logs so far.
uint8_t CHG_Ready=0;  //indicator to the LIM that we are ready to charge. observed values 0 when not charging , 1 when commanded to charge. only 2 bits used.
uint8_t CONT_Ctrl=0;  //4 bits with DC ccs contactor command.

#define Status_NotRdy 0x0 //no led
#define Status_Rdy 0x2  //pulsing blue led when on charge. 0x2. 0x1 = 1 red flash then off
#define Req_Charge 0x1
#define Req_EndCharge 0x0
#define Chg_Rdy 0x1
#define Chg_NotRdy 0x0

#define No_Chg 0x0
#define AC_Chg 0x1
#define DC_Chg 0x2

void i3LIMClass::handle3B4(uint32_t data[2])  //Lim data

{
    uint8_t* bytes = (uint8_t*)data;// arrgghhh this converts the two 32bit array into bytes. See comments are useful:)
    CP_Amps=bytes[0];
    Param::SetInt(Param::PilotLim,CP_Amps);
    PP_Amps=bytes[1];
    Param::SetInt(Param::CableLim,PP_Amps);
    PP=(bytes[2]&0x1);
    Param::SetInt(Param::PlugDet,PP);
    CP_Mode=(bytes[4]&0xF);
     switch (CP_Mode)
    {
        case 0x8:
        CP_Typ=0;   //No pilot signal
        break;

        case 0x9:
        CP_Typ=1;   //Standard AC pilot. Not charging
        break;

        case 0xA:
        CP_Typ=2;   //Standard AC pilot. charging
        break;

        case 0xC:
        CP_Typ=3;   //5% pilot requesting CCS
        break;

        case 0xD:
        CP_Typ=4;   //CCS greenphy comms
        break;

        default:
        CP_Typ=0;   //No pilot signal
        break;
    }
    Param::SetInt(Param::PilotTyp,CP_Typ);

    Cont_Volts=bytes[7]*2;
   // Cont_Volts=FP_MUL(Cont_Volts,2);
    Param::SetInt(Param::CCS_V_Con,Cont_Volts);//voltage measured on the charger side of the hv ccs contactors in the car

}

void i3LIMClass::handle29E(uint32_t data[2])  //Lim data. Available current and voltage from the ccs charger

{
uint8_t* bytes = (uint8_t*)data;// arrgghhh this converts the two 32bit array into bytes. See comments are useful:)
V_Avail=((bytes[2]<<8)|(bytes[1]));
V_Avail=FP_TOINT(FP_DIV(V_Avail,10));
Param::SetInt(Param::CCS_V_Avail,V_Avail);//available voltage from ccs charger

I_Avail=((bytes[4]<<8)|(bytes[3]));
I_Avail=FP_TOINT(FP_DIV(I_Avail,10));
Param::SetInt(Param::CCS_I_Avail,I_Avail);//available current from ccs charger
}

void i3LIMClass::handle2B2(uint32_t data[2])  //Lim data. Current and Votage as measured by the ccs charger

{
uint8_t* bytes = (uint8_t*)data;// arrgghhh this converts the two 32bit array into bytes. See comments are useful:)
CCS_Vmeas=((bytes[1]<<8)|(bytes[0]));
CCS_Vmeas=FP_TOINT(FP_DIV(CCS_Vmeas,10));
Param::SetInt(Param::CCS_V,CCS_Vmeas);//Voltage measurement from ccs charger

CCS_Imeas=((bytes[3]<<8)|(bytes[2]));
CCS_Imeas=FP_TOINT(FP_DIV(CCS_Imeas,10));
Param::SetInt(Param::CCS_I,CCS_Imeas);//Current measurement from ccs charger
Batt_Cmp=bytes[4]&&0xc0;    //battrery compatability flag from charger? upper two bits of byte 4.
}

void i3LIMClass::handle2EF(uint32_t data[2])  //Lim data. Min available voltage from the ccs charger.

{
uint8_t* bytes = (uint8_t*)data;// arrgghhh this converts the two 32bit array into bytes. See comments are useful:)
minV_Avail=((bytes[1]<<8)|(bytes[0]));
minV_Avail=FP_TOINT(FP_DIV(minV_Avail,10));
Param::SetInt(Param::CCS_V_Min,minV_Avail);//minimum available voltage from ccs charger
}

void i3LIMClass::handle272(uint32_t data[2])  //Lim data. CCS contactor state and charge flap open/close status.

{
uint8_t* bytes = (uint8_t*)data;// arrgghhh this converts the two 32bit array into bytes. See comments are useful:)
Cont_stat=bytes[2];
}


void i3LIMClass::Send10msMessages()
{
   V_Batt=Param::GetInt(Param::udc)*10;
   V_Batt2=(Param::GetInt(Param::udc))/4;
   I_Batt=(Param::GetInt(Param::idc)+819)*10;//(Param::GetInt(Param::idc);FP_FROMINT
   //I_Batt=0xa0a0;
   SOC_Local=50;//(Param::GetInt(Param::SOC))*10;
uint8_t bytes[8]; //seems to be from i3 BMS.
bytes[0] = I_Batt & 0xFF;  //Battery current LSB. Scale 0.1 offset 819.2. 16 bit unsigned int
bytes[1] = I_Batt >> 8;  //Battery current MSB. Scale 0.1 offset 819.2.  16 bit unsigned int
bytes[2] = V_Batt & 0xFF;  //Battery voltage LSB. Scale 0.1. 16 bit unsigned int.
bytes[3] = V_Batt >> 8;  //Battery voltage MSB. Scale 0.1. 16 bit unsigned int.
bytes[4] = SOC_Local & 0xFF;;  //Battery SOC LSB. 12 bit unsigned int. Scale 0.1. 0-100%
bytes[5] = SOC_Local >> 8;  //Battery SOC MSB. 12 bit unsigned int. Scale 0.1. 0-100%
bytes[6] = 0x65;  //Low nibble battery status. Seem to need to be 0x5.
bytes[7] = V_Batt2;  //zwischenkreis. Battery voltage. Scale 4. 8 bit unsigned int.

Can::GetInterface(0)->Send(0x112, (uint32_t*)bytes,8); //Send on CAN1


}


void i3LIMClass::Send200msMessages()
{
    Wh_Local=Param::GetInt(Param::BattCap);
 uint16_t CHG_Pwr1=CHG_Pwr/25;
    CHG_Pwr1=(CHG_Pwr1 & 0xFFF);
uint8_t bytes[8]; //Main LIM control message
bytes[0] = Wh_Local & 0xFF;  //Battery Wh lowbyte
bytes[1] = Wh_Local >> 8;  //BAttery Wh high byte
bytes[2] = ((CHG_Status<<4)|(CHG_Req));  //charge status in bits 4-7.goes to 1 then 2.8 secs later to 2. Plug locking???. Charge request in lower nibble. 1 when charging. 0 when not charging.
bytes[3] = (((CHG_Pwr1)<<4)|CHG_Ready);  //charge readiness in bits 0 and 1. 1 = ready to charge.upper nibble is LSB of charge power.
bytes[4] = CHG_Pwr1>>4;   //MSB of charge power.in this case 0x28 = 40x25 = 1000W. Probably net DC power into the Batt.
bytes[5] = 0x00;   //LSB of the DC ccs current command
bytes[6] = 0x00;   //bits 0 and 1 MSB of the DC ccs current command.Upper nibble is DC ccs contactor control. Observed in DC fc logs only.
                    //transitions from 0 to 2 and start of charge but 2 to 1 to 0 at end. Status and Ready operate the same as in AC logs.
bytes[7] = EOC_Time;    // end of charge timer.


Can::GetInterface(0)->Send(0x3E9, (uint32_t*)bytes,8); //Send on CAN1

                //LIM needs to see this but doesnt control anything...
bytes[0] = 0xca;
bytes[1] = 0xff;
bytes[2] = 0x0b;
bytes[3] = 0x02;
bytes[4] = 0x69;
bytes[5] = 0x26;
bytes[6] = 0xf3;
bytes[7] = 0x4b;
Can::GetInterface(0)->Send(0x431, (uint32_t*)bytes,8); //Send on CAN1

                //LIM needs to see this but doesnt control anything...
bytes[0] = 0x00;
bytes[1] = 0x00;
bytes[2] = 0x00;
bytes[3] = 0x00;
bytes[4] = 0xfe;
bytes[5] = 0x00;
bytes[6] = 0x00;
bytes[7] = 0x60;
Can::GetInterface(0)->Send(0x560, (uint32_t*)bytes,8); //Send on CAN1

}

void i3LIMClass::Send100msMessages()
{
uint8_t bytes[8]; //Wake up message.
bytes[0] = 0xf5;
bytes[1] = 0x28;
bytes[2] = 0x88;
bytes[3] = 0x1d;
bytes[4] = 0xf1;
bytes[5] = 0x35;
bytes[6] = 0x30;
bytes[7] = 0x80;

Can::GetInterface(0)->Send(0x12f, (uint32_t*)bytes,8); //Send on CAN1

                //central locking status message.
bytes[0] = 0x81;
bytes[1] = 0x00;
bytes[2] = 0x04;
bytes[3] = 0xff;
bytes[4] = 0xff;
bytes[5] = 0xff;
bytes[6] = 0xff;
bytes[7] = 0xff;
Can::GetInterface(0)->Send(0x2fc, (uint32_t*)bytes,8); //Send on CAN1

                //Lim command 2. Used in DC mode

bytes[0] = 0xA2;  //Charge voltage limit LSB. 14 bit signed int.scale 0.1 0xfa2=4002*.1=400.2Volts
bytes[1] = 0x0f;  //Charge voltage limit MSB. 14 bit signed int.scale 0.1
bytes[2] = 0x00;  //Fast charge current limit. Not used in logs from 2014-15 vehicle so far. 8 bit unsigned int. scale 1.so max 254amps in theory...
bytes[3] = 0x18;  //time remaining in seconds to hit soc target from byte 7 in AC mode. LSB. 16 bit unsigned int. scale 10.
bytes[4] = 0x1B;  //time remaining in seconds to hit soc target from byte 7 in AC mode. MSB. 16 bit unsigned int. scale 10.
bytes[5] = 0xFB;  //time remaining in seconds to hit soc target from byte 7 in ccs mode. LSB. 16 bit unsigned int. scale 10.
bytes[6] = 0x06;  //time remaining in seconds to hit soc target from byte 7 in ccs mode. MSB. 16 bit unsigned int. scale 10.
bytes[7] = 0xA0;  //Fast charge SOC target. 8 bit unsigned int. scale 0.5. 0xA0=160*0.5=80%

Can::GetInterface(0)->Send(0x2f1, (uint32_t*)bytes,8); //Send on CAN1

                //Lim command 3. Used in DC mode

bytes[0] = 0x84; //Time to go in minutes LSB. 16 bit unsigned int. scale 1. May be used for the ccs station display of charge remaining time...
bytes[1] = 0x04; //Time to go in minutes MSB. 16 bit unsigned int. scale 1. May be used for the ccs station display of charge remaining time...
bytes[2] = 0x00;  //upper nibble seems to be a mode command to the ccs station. 0 when off, 9 when in constant current phase of cycle.
                    //more investigation needed here...
                   //Lower nibble seems to be intended for two end charge commands each of 2 bits.
bytes[3] = 0xff;
bytes[4] = 0xff;
bytes[5] = 0xff;
bytes[6] = 0xff;
bytes[7] = 0xff;
Can::GetInterface(0)->Send(0x2fa, (uint32_t*)bytes,8); //Send on CAN1

}

uint8_t i3LIMClass::Control_Charge()
{
    int opmode = Param::GetInt(Param::opmode);
    if (opmode != MOD_RUN)
    {
if (Param::GetBool(Param::PlugDet)&&(!Param::GetBool(Param::Chgctrl))&&(CP_Mode==0x9||CP_Mode==0xA))  //if we have an enable and a plug in and a std ac pilot lets go AC charge mode.
{

  EOC_Time=0xFE;
  CHG_Status=Status_Rdy;
  CHG_Req=Req_Charge;
  CHG_Ready=Chg_Rdy;
  CHG_Pwr=FP_FROMFLT(Param::GetInt(Param::udc));
    return AC_Chg;

}


if (Param::GetBool(Param::PlugDet)&&(!Param::GetBool(Param::Chgctrl))&&(CP_Mode==0xC||CP_Mode==0xD))  //if we have an enable and a plug in and a 5% pilot lets go DC charge mode.
{

  EOC_Time=0xFE;
  CHG_Status=Status_Rdy;
  CHG_Req=Req_Charge;
  CHG_Ready=Chg_Rdy;
  CHG_Pwr=FP_FROMFLT(Param::GetInt(Param::udc));
    return DC_Chg;

}


if (!Param::GetBool(Param::PlugDet)||(Param::GetBool(Param::Chgctrl)))  //if we a disable or plug remove shut down
{
    uint16_t test1=Param::GetInt(Param::udc);
  EOC_Time=0x00;
  CHG_Status=Status_NotRdy;
  CHG_Req=Req_EndCharge;
  CHG_Ready=Chg_NotRdy;
  CHG_Pwr=test1;
    return No_Chg;
}
}

}
