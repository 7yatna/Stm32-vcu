#include <teslaCharger.h>

static bool HVreq=false;
static bool ChRun=false;


void teslaCharger::SetCanInterface(CanHardware* c)
{
   can = c;
   can->RegisterUserMessage(0x109);

}

void teslaCharger::DecodeCAN(int id, uint32_t data[2])
{
   uint8_t* bytes = (uint8_t*)data;
   
 if (id == 0x109)
   {
   if(bytes[5]==0x05) HVreq=true;
   if(bytes[5]==0x00) HVreq=false;
    
  }
}

void teslaCharger::Task100Ms()
{
   uint8_t bytes[8];
  //uint16_t HVvolts=Param::GetInt(Param::udc);
   uint16_t HVvoltspnt=Param::GetInt(Param::Voltspnt);
   uint16_t Pilot_I=Param::GetInt(Param::PilotLim);
   uint8_t SOC = Param::GetInt(Param::SOC);
  // uint8_t EFF = Param::GetInt(Param::ChgEff);
  // int AC_Volt = 110;
  // if (Pilot_I > 16)AC_Volt = 220; 
   //uint8_t HV_I=((AC_Volt*Pilot_I*EFF)/(100*HVvolts));
  //DC_current = fixed_AC_voltage * CP_PP_current_limit * phase_count * charger_efficiency / DC_voltage
/*  
  bytes[0] = 0x00;
   bytes[1] = ((HVvoltspnt&0xFF00)>>8);//HV voltage highbyte
   bytes[2] = (HVvoltspnt&0xFF);//HV voltage lowbyte
   bytes[3] = (HV_I&0xFF);//HV Current setpoint lowbyte
   bytes[4] = 0x00;
   if(ChRun)bytes[5] = 0x01;  //send Chg enable
   if(!ChRun)bytes[5] = 0x00;      //send Chg disable
   bytes[6] = 0x50;
   bytes[7] = 0x00;
   //original OI Code
  // original Karim tesla Code 
   bytes[0] = 0x00;
   bytes[1] = ((HVvoltspnt&0xFF00)>>8);//HV voltage highbyte
   bytes[2] = (HVvoltspnt&0xFF);//HV voltage lowbyte
   bytes[3] = ((HV_I&0xFF00)>>8);//HV Current setpoint highbyte
   bytes[4] = (HV_I&0xFF);//HV Current setpoint lowbyte
   if(ChRun)bytes[5] = 0x01;  //send Chg enable
   if(!ChRun)bytes[5] = 0x00;      //send Chg disable
   bytes[6] = 0x50;
   bytes[7] = 0x00;
   can->Send(0x102, (uint32_t*)bytes,8);
*/
   bytes[0] = 0x00;
   bytes[1] = (HVvoltspnt&0xFF);//HV voltage lowbyte
   bytes[2] = ((HVvoltspnt&0xFF00)>>8);//HV voltage highbyte
   bytes[3] = 0x2D; //Max DC current at 45A
   bytes[4] = Pilot_I; // In Karim Tesla STM_charger loop control is by AC current which equal the Piulot Current. 
   //bytes[4] = 0x00;
   if(ChRun)bytes[5] = 0x01;  //send Chg enable
   if(!ChRun)bytes[5] = 0x00; //send Chg disable
   bytes[6] = SOC;
   bytes[7] = 0x00;
   can->Send(0x102, (uint32_t*)bytes,8);

}


bool teslaCharger::ControlCharge(bool RunCh, bool ACReq)
{
   //bool dummy=RunCh;
   //dummy=dummy;
   //ChRun=ACReq;
    if(RunCh && ACReq)
        {
            //enable charger digital line.
            IOMatrix::GetPin(IOMatrix::OBCENABLE)->Set();
			ChRun = true;
        }
    else
        {
            //disable charger digital line when requested by timer or webui.
            IOMatrix::GetPin(IOMatrix::OBCENABLE)->Clear();
			ChRun = false;
			HVreq=false;
        }
   
   if(HVreq) return true;
   if(!HVreq) return false;
   return false;

}