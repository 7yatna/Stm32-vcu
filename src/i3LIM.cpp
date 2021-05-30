#include <i3LIM.h>

static uint8_t CP_Amps=0;
static uint8_t PP_Amps=0;
static uint8_t CP_Mode=0;
static uint8_t CP_Typ=0;
static bool PP=false;

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


}

void i3LIMClass::handle29E(uint32_t data[2])  //Lim data. Available current and voltage from the ccs charger

{
uint8_t* bytes = (uint8_t*)data;// arrgghhh this converts the two 32bit array into bytes. See comments are useful:)

}

void i3LIMClass::handle2B2(uint32_t data[2])  //Lim data. Current and Votage as measured by the ccs charger

{
uint8_t* bytes = (uint8_t*)data;// arrgghhh this converts the two 32bit array into bytes. See comments are useful:)

}

void i3LIMClass::handle2EF(uint32_t data[2])  //Lim data. Min available voltage from the ccs charger.

{
uint8_t* bytes = (uint8_t*)data;// arrgghhh this converts the two 32bit array into bytes. See comments are useful:)

}

void i3LIMClass::handle272(uint32_t data[2])  //Lim data. CCS contactor state and charge flap open/close status.

{
uint8_t* bytes = (uint8_t*)data;// arrgghhh this converts the two 32bit array into bytes. See comments are useful:)

}


void i3LIMClass::Send10msMessages()
{
uint8_t bytes[8]; //Main LIM control message
bytes[0] = 0xf9;  //Battery current LSB. Scale 0.1 offset 819.2. 16 bit unsigned int
bytes[1] = 0x1f;  //Battery current MSB. Scale 0.1 offset 819.2.  16 bit unsigned int
bytes[2] = 0x8b;  //Battery voltage LSB. Scale 0.1. 16 bit unsigned int.
bytes[3] = 0x0e;  //Battery voltage MSB. Scale 0.1. 16 bit unsigned int.
bytes[4] = 0xa6;  //Battery SOC LSB. 12 bit unsigned int. Scale 0.1. 0-100%
bytes[5] = 0x71;  //Battery SOC MSB. 12 bit unsigned int. Scale 0.1. 0-100%
bytes[6] = 0x65;  //Low nibble battery status. Seem to need to be 0x5.
bytes[7] = 0x5d;  //zwischenkreis. Battery voltage. Scale 4. 8 bit unsigned int.

Can::GetInterface(0)->Send(0x112, (uint32_t*)bytes,8); //Send on CAN1


}


void i3LIMClass::Send200msMessages()
{
uint8_t bytes[8]; //Main LIM control message
bytes[0] = 0x01;  //Battery Wh lowbyte
bytes[1] = 0x00;  //BAttery Wh high byte
bytes[2] = 0x00;  //charge status in bits 4-7.goes to 1 then 2.8 secs later to 2. Plug locking???. Charge request in lower nibble. 1 when charging. 0 when not charging.
bytes[3] = 0x00;  //charge readiness in bits 0 and 1. 1 = ready to charge.upper nibble is LSB of charge power.
bytes[4] = 0x00;   //MSB of charge power.in this case 0x28 = 40x25 = 1000W. Probably net DC power into the Batt.
bytes[5] = 0x00;   //LSB of the DC ccs current command
bytes[6] = 0x00;   //bits 0 and 1 MSB of the DC ccs current command.Upper nibble is DC ccs contactor control. Observed in DC fc logs only.
                    //transitions from 0 to 2 and start of charge but 2 to 1 to 0 at end. Status and Ready operate the same as in AC logs.
bytes[7] = 0x00;    // end of charge timer.


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
