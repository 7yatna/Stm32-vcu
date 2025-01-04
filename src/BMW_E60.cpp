#include <BMW_E60.h>
#include "stm32_can.h"
#include "params.h"


uint8_t Gcount = 0x0C; //gear display counter byte - Karim Change ??
uint8_t Gearcount = 0xF1;
uint8_t Engine_count1 = 0x00;
uint8_t Engine_count2 = 0x00;
uint8_t A91=0x00;//0x0A9 second counter byte
uint8_t BA6=0x80;//0x0BA second counter byte(byte 6)
int C_Count1 = 0;
int C_Count2 = 0;
uint8_t shiftPos = 0xe1; //contains byte to display gear position on dash.default to park
uint8_t MPos = 0x0f;
uint8_t SPos = 0xf0;
int Sport = 0;
uint8_t gear_BA = 0x03; //set to park as initial condition//
int B1_Count1 = 0x40;
int B1_Count2 =0x70;
int once = 0;
int once2 = 0;
uint8_t Wake_UP = 0;
int Cruise_ON = 0;
int IS_CC_ON = 0;
int IS_CC_NONE = 0;
int IS_CC_SET = 0;
int IS_CC_RESUME = 0;
int IS_CC_CANCEL = 0;
//float ThrotRamp = 0.2f;



void BMW_E60::SetCanInterface(CanHardware* c)
{
   can = c;
   can->RegisterUserMessage(0x130);//E60 CAS
   can->RegisterUserMessage(0x194);//Cruise Control Shifter
   can->RegisterUserMessage(0x198);//E60 Shifter
   can->RegisterUserMessage(0x19E);//E60 DSC
   
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
///////Handle incomming pt can messages from the car here
////////////////////////////////////////////////////////////////////////////////////////////////////
void BMW_E60::DecodeCAN(int id, uint32_t* data)
{
   uint8_t* bytes = (uint8_t*)data;

   switch (id)
   {
	case 0x130:
		if (bytes[0] == 0x45)
		{
			switch (once)
			{
		      case 0:
				Check_Engine_On();
				terminal15On = false;
				this->gear = PARK;
				gear_BA = 0x03;
                shiftPos = 0xe1;
			    MPos = 0x0f;
				SPos = 0xf0;
				Sport = 0;
				Reset_Cruise();
				//ThrotRamp = GetFloat(Param::throtramp);
               break;
			  case 1:
		       Check_Engine_Off();
			   terminal15On = true;
			  break;
			}
		}
		
		else if (bytes[0] == 0x55)
		{
			once = 1;
			once2 = 3;
		}
		
		else
		{
			once = 0;
			switch(once2)
			{	case 0:
				 break;
				case 2:
				 Engine_count1 = 0x00;
				 once2 = 0;
				break;
			}
			terminal15On = false;
			this->gear = PARK;
			gear_BA = 0x03;
            shiftPos = 0xe1;
			MPos = 0x0f;
			SPos = 0xf0;
			Sport = 0;
			Reset_Cruise();
			Charge_Batt_Off();
		}
		break;   
	  
	  case 0x194:
	   {
		uint32_t CCLeaver =  data[0] & 0xFFFF0000;   //unsigned int to contain result of message 0x194. Cruise control lever position      
		switch (CCLeaver)
         {
			   case 0xFC800000:  //Stalk not pressed.
			   IS_CC_SET = 0;
			   IS_CC_RESUME = 0;
			   IS_CC_CANCEL = 0;
			   break;
            case 0xFC900000: //Stalk at OFF.
			   Cruise_ON = 0;
			   IS_CC_SET = 0;
			   IS_CC_RESUME = 0;
			   IS_CC_CANCEL = 1;			   
			   break;
			case 0xFC840000:  //Stalk at Decrease 1 MPH.
			   //Cruise_ON = 0;
			   //this->cruise = CC_CANCEL;
			   break;
			case 0xFC880000:  ////Stalk at Decrease 5 MPH.
			   //Cruise_ON = 0;
			   //this->cruise = CC_CANCEL;
			   break;
			case 0xFC810000:  ////Stalk at increaase 1 MPH.
			   Cruise_ON = 1;
			   IS_CC_SET = 0;
			   IS_CC_RESUME = 0;
			   IS_CC_CANCEL = 0;			   //this->cruise = CC_ON;
			   break;
			case 0xFC820000:  ////Stalk at increase 5 MPH.
			   Cruise_ON = 1;
			   IS_CC_SET = 1;
			   IS_CC_RESUME = 0;
			   IS_CC_CANCEL = 0;			   //this->cruise = CC_SET;
			   break;
			case 0xFCC00000:  ////Stalk at Resume.
			   IS_CC_SET = 0;
			   IS_CC_RESUME = 1;
			   IS_CC_CANCEL = 0;			   //this->cruise = CC_RESUME;
			   break;

		 }
	   }
	  break; 

	  case 0x198:
	  {
	  uint32_t GLeaver = data[0] & 0xFFFFFF00;   //unsigned int to contain result of message 0x198. Gear selector lever position        
	  switch (GLeaver)
         {
            case 0xC5CF0000:  //park button pressed
               this->gear = PARK;
               gear_BA = 0x03;
               shiftPos = 0xe1;
			   MPos = 0x0f;
			   SPos = 0xf0;
			   Sport = 0;
               break;
			case 0xC5CD2000: //R+ Park position
			   Param::SetInt(Param::Gear, 0);
			   this->gear = REVERSE;
               gear_BA = 0x02;
               shiftPos = 0xd2;
               MPos = 0x0f;
			   SPos = 0xf0;
			   Sport = 0;
			   break;
            case 0xC0CD2000: //R+ position
			   Param::SetInt(Param::Gear, 0);
			   this->gear = REVERSE;
               gear_BA = 0x02;
               shiftPos = 0xd2;
               MPos = 0x0f;
			   SPos = 0xf0;
			   Sport = 0;
			   break;
			case 0xC0CC3000: //R to Neutral
               this->gear = PARK;
               gear_BA = 0x01;
               shiftPos = 0xb4;
               MPos = 0x0f;
			   SPos = 0xf0;
			   Sport = 0;
			   break;  			
			case 0xC0CF0000:  //not pressed
				Param::SetInt(Param::Gear, 2);
				//Param::SetFloat(Param::throtramp, ThrotRamp);
				if ((Param::GetInt(Param::dir)) == -1)
					{
						Param::SetInt(Param::Gear, 0);
					}
				MPos = 0x0f;
				SPos = 0xf0;
				Sport = 0;
				break;
			case 0xC0CE1000: //D to Neutral
               this->gear = PARK;
               gear_BA = 0x01;
               shiftPos = 0xb4;
               MPos = 0x0f;
			   SPos = 0xf0;
			   Sport = 0;
			   break;   
            case 0xC0CB4000:  //D+ pressed
			   Param::SetInt(Param::Gear, 2);
			   this->gear = DRIVE;
               gear_BA = 0x08;
               shiftPos = 0x78;
               MPos = 0x0f;
			   SPos = 0xf0;
			   Sport = 0;
			   break;
			case 0xC5CB4000:  //D+ Park pressed
			   Param::SetInt(Param::Gear, 2);
			   this->gear = DRIVE;
               gear_BA = 0x08;
               shiftPos = 0x78;
               MPos = 0x0f;
			   SPos = 0xf0;
			   Sport = 0;
			   break;
			case 0xC0CA5000:  //Sport Minus
			   Param::SetInt(Param::Gear, 0);
			   //Param::SetInt(Param::throtramp, 12);
			   this->gear = DRIVE;
			   gear_BA = 0x05;
               shiftPos = 0x78;
			   MPos = 0x5f;
			   SPos = 0xf2;
			   break;
			case 0xC0C96000:  //Sport Plus
			   Param::SetInt(Param::Gear, 1);
			   //Param::SetInt(Param::throtramp, 12);
			   this->gear = DRIVE;
			   gear_BA = 0x06;
               shiftPos = 0x78;
			   MPos = 0x6f;
			   SPos = 0xf2;
			   break;
			case 0xC0C87000:  //  S-M-D button pressed
			   switch(Sport)
			   {
				   case 0:
					   Param::SetInt(Param::Gear, 2);
					   this->gear = DRIVE;
					   gear_BA = 0x08;
					   shiftPos = 0x78;
					   MPos = 0x0f;
					   SPos = 0xf1;
					   Sport = 1;
					   //Param::SetInt(Param::throtramp, 12);
					   break;
				   case 1:
					   break;
			   }
               break;
		 }    
		}
		break;
		case 0x19E:
		  {
			 Param::SetInt(Param::brakepedal, bytes[6]);
		  }
		break;
   }
}



void BMW_E60::Batt_Voltage_3B3()
{
	uint8_t bytes[8];
	bytes[0] =0xF1;
    bytes[1] =0xC8;
	if (Ready())
	{
		bytes[2] =0x00;
		bytes[3] =0x00;
		bytes[4] =0x00;
		bytes[5] =0xF0;
	}
	else
	{
		bytes[2] =0xFF;
		bytes[3] =0x7F;
		bytes[4] =0x00;
		bytes[5] =0xF1;
	}
   
    can->Send(0x3B3,bytes,6); //Send on CAN2
}

void BMW_E60::Batt_Voltage_3B4()
{
	uint8_t bytes[8];
	bytes[0] =0xCD;
    bytes[1] =0xF3;
	if (Ready()) bytes[2] = 0x00;
	else bytes[2] =0x09;
	bytes[3] =0xFF;
    bytes[4] =0xFF;
    bytes[5] =0xFF;
    bytes[6] =0xFF;
    bytes[7] =0xFF;
    can->Send(0x3B4,bytes,8); //Send on CAN2
}

void BMW_E60::Send_RPM()
{
	uint8_t bytes[8];
	uint16_t rpm = MAX(997, revCounter) * 4; // rpm value for E60
	rpm = rpm / 1.5;
	uint16_t potnom = GetInt(Param::potnom);
	potnom = potnom*65064;
	potnom = potnom / 100;
	
	uint16_t Byte1 = B1_Count1;  
	uint16_t Byte2 = 0xFB;
	uint16_t Byte3 = 0x00;
	uint16_t Byte4 = 0x00;
	uint16_t Byte5 = 0x00;
	uint16_t Byte6 = 0xF4;
	uint16_t Byte7 = 0x00;
	
	if (Ready())
	{
		Byte2 = lo8(potnom);
		Byte3 = hi8(potnom);
		Byte4 = lo8(rpm);
		Byte5 = hi8(rpm); 
		
		if (potnom > 4)
		{
			Byte6 =0x94;
		}
		else 
		{
			Byte6 =0x80;
		}
		Byte7 = 0xFE; 
	}	
	
	int16_t check_AA;
	check_AA = (Byte1+Byte2+Byte3+Byte4+Byte5+0xAA+Byte6+Byte7);
    check_AA = (check_AA / 0x100)+ (check_AA & 0xFF);
    check_AA = check_AA & 0xFF;
		
	bytes[0]=check_AA;
    bytes[1]=Byte1;
	bytes[2]=Byte2;
	bytes[3]=Byte3;
    bytes[4]=Byte4;
    bytes[5]=Byte5;
    bytes[6]=Byte6;
    bytes[7]=Byte7;
	
    can->Send(0x0AA,bytes,8); //Send on CAN2  
	
	B1_Count1++;
	
	if (B1_Count1== 0x4F)
	{
		B1_Count1 = 0x40;
	}
}

void BMW_E60::Heatflow()
{
	uint8_t bytes[8];

	bytes[0] =0xFF;
    bytes[1] =0xFF;
	bytes[2] =0x3C;
	if (Ready())
	{
		bytes[3] =0x02;
		bytes[4] =0x96;
		bytes[5] =0xF0;
		bytes[6] =0x26;
	}
	else
	{
		bytes[3] =0x00;
		bytes[4] =0x00;
		bytes[5] =0xFC;
		bytes[6] =0x0F;
	}
 
    can->Send(0x1B6,bytes,7); //Send on CAN2  
}

void BMW_E60::Task10Ms()
{
   Wake_UP = GetInt(Param::din_WAKE_UP);
   
   if (Ready())
   {
      
   }
   if (Wake_UP)
   {
	   Send_RPM();
	   SendAbsDscMessages(Param::GetBool(Param::din_brake));
   }
   if (!Wake_UP)
   {
	   once = 0;
			switch(once2)
			{	case 0:
				 break;
				case 2:
				 Engine_count1 = 0x00;
				 once2 = 0;
				break;
			}
			terminal15On = false;
			gear_BA = 0x03;
            shiftPos = 0xe1;
			MPos = 0x0f;
   }
}


void BMW_E60::Task100Ms()
{
	if (Wake_UP)
   {
		Heatflow();
   }

   if (!this->dashInit)
   {
      
	  this->dashInit=true;
   }
    if (Ready())
   {
	 AC_Comp();
	 
   }
   
}

void BMW_E60::Task200Ms()
{
   if (Wake_UP)
   {
	   Gear_Display();
	   Gear2();
	   Temp_Gauge();
	   DME_Alive();
	   Cruise_Control();
	   Engine_Data();
	   Batt_Voltage_3B3();
	   Batt_Voltage_3B4();
	   int Batt = GetInt(Param::SOC);
	   if (Batt < 15)
		   {
			   Charge_Batt_On();
		   }
		
		else
		   {
				Charge_Batt_Off();
		   }
		   
   }
   
  if (Ready())
   {
	  Dodge_PS();
   }
}

void BMW_E60::Dodge_PS()
{
  uint8_t bytes[8];
  bytes[0]=0x54;
  bytes[1]=0x01;
  bytes[2]=0x40;
  bytes[3]=0x00;
  bytes[4]=0x00;
  bytes[5]=0x00;
  bytes[6]=0x00;
  bytes[7]=0x00;
  can->Send(0x308, bytes, 8);
}

void BMW_E60::AC_Comp()
{
  uint8_t bytes[8];
  uint8_t AC_Comp = GetInt(Param::AC_Comp_Req);
  bytes[0]=0x90;
  bytes[1]=0x01;
  bytes[2]=0xB8;
  bytes[3]=0x0B;
  bytes[4]=0x00;
  bytes[5]=0x00;
  if (AC_Comp) bytes[5]=0x01;
  bytes[6]=0x00;
  bytes[7]=0x00;
  can->Send(0x28A, bytes, 8);
}

void BMW_E60::Temp_Gauge()
{
   uint8_t bytes[2];
   float tmphs = Param::GetFloat(Param::tmphs);
   float tmp = MAX(10, tmphs);
   float RPM_thing = map(tmp, 10, 80, -6500, -5500);
   RPM_thing = RPM_thing * -1;
   RPM_thing = RPM_thing / 50;
   bytes[0] = RPM_thing; //sets max rpm on tach (temp thing)
   bytes[1] = 0x82;
   can->Send(0x332, bytes, 2); //Send on CAN2
   
   if (tmp > 60)
   {
		Over_heat_On();
		Check_Engine_On();
   }
   
}

void BMW_E60::Engine_Data()
{	
   uint8_t bytes[8];
   float Curr = Param::GetFloat(Param::idc);
   Curr = Curr + 100;
   float Temp = map(Curr, 0, 500, 50, 143);
   Temp = Temp + 48;
   if (Curr > 525)
	{
	  Temp = 203;
	}
  
   bytes[0]=0x8E;
   bytes[1]=Temp;
   bytes[2]=Engine_count1;
   bytes[3]=0xCE;
   bytes[4]=0xFF;
   bytes[5]=Engine_count2;
   bytes[6]=0xFD;
   bytes[7]=0x84;
   
   can->Send(0x1D0,bytes,8); //Send on CAN2
    
	if (once2 == 3)
	{
		Engine_count1 = 0x60;
		once2 = 2;
	}
	
	switch (C_Count1)
         {
            case 0:  
               Engine_count1++;
               C_Count1++;
               break;
            case 1:
               C_Count1=0;
               break;   
          }
 
   switch (C_Count2)
         {
            case 0:  
               Engine_count2++;
               C_Count2++;
               break;
             case 1:
             C_Count2++;
               break;
             case 2:
             C_Count2++;
               break;
			 case 3:
             C_Count2++;
               break;
             case 4:
            C_Count2=0;
               break;
         }
		 
 if (Engine_count1 == 0x6F)	
	{
		Engine_count1 = 0x60;
	}
	
else if (Engine_count1 == 0x0F)
	{
		Engine_count1 = 0x00;
	}
	
 if (Engine_count2 == 0xFF)
   {
      Engine_count2=0x00;
   }
 
}

void BMW_E60::DME_Alive()
{
   uint8_t bytes[8];
   bytes[0]=0x16;
   bytes[1]=0x42;
   bytes[2]=0xFF;
   bytes[3]=0xFF;
   bytes[4]=0xFF;
   bytes[5]=0xFF;
   bytes[6]=0xFF;
   bytes[7]=0xFF;
   can->Send(0x492,bytes,8);

   bytes[0]=0x22;
   bytes[1]=0x42;
   bytes[2]=0xFF;
   bytes[3]=0xFF;
   bytes[4]=0xFF;
   bytes[5]=0xFF;
   bytes[6]=0xFF;
   bytes[7]=0xFF;
   can->Send(0x498,bytes,8);
}


void BMW_E60::Gear_Display()
{	
   uint8_t bytes[6];
///////////////////////////////////////////////////////////////////////////////////////////////////
   bytes[0]=shiftPos;  //e1=P  78=D  d2=R  b4=N
   bytes[1]=MPos;
   bytes[2]=0xFF;
   bytes[3]=Gcount;
   bytes[4]=SPos;
   bytes[5]=0xFF;
   
   can->Send(0x1D2, (uint32_t*)bytes,6); //Send on CAN2
   ///////////////////////////
   //Byte 3 is a counter running from 0C through to EC and then back to 0C///
   //////////////////////////////////////////////
   
   Gcount=Gcount+0x10;
   
   if (Gcount==0xEC)
   {
      Gcount=0x0C;
   }
}

//E60 Shifter Pulse Routine 
void BMW_E60::Gear2()
{	
   uint8_t bytes[8];
   bytes[0]=0x00;
   bytes[1]=0x42;
   bytes[2]=0xFD;
   bytes[3]=0x01;
   bytes[4]=0xFF;
   bytes[5]=0xFF;
   bytes[6]=0xFF;
   bytes[7]=0xFF;
   
   
   can->Send(0x4DE,bytes,8); //Send on CAN2
}
//End E60 Shifter Pulse Routine

//E60 Check Engnie Light 
void BMW_E60::Check_Engine_On()
{	
   uint8_t bytes[8];
   bytes[0]=0x40;
   bytes[1]=0x22;
   bytes[2]=0x00;
   bytes[3]=0x31;
   bytes[4]=0xFF;
   bytes[5]=0xFF;
   bytes[6]=0xFF;
   bytes[7]=0xFF;
   
   can->Send(0x592,bytes,8); //Send on CAN2
}
//End E60 Check Engnie Light



//E60 Check Engnie Light 
void BMW_E60::Check_Engine_Off()
{	
   uint8_t bytes[8];
   bytes[0]=0x40;
   bytes[1]=0x22;
   bytes[2]=0x00;
   bytes[3]=0x30;
   bytes[4]=0xFF;
   bytes[5]=0xFF;
   bytes[6]=0xFF;
   bytes[7]=0xFF;
   
   can->Send(0x592,bytes,8); //Send on CAN2
}
//End E60 Check Engnie Light

void BMW_E60::Charge_Batt_On()
{	
   uint8_t bytes[8];
   bytes[0]=0x40;
   bytes[1]=0xE5;
   bytes[2]=0x00;
   bytes[3]=0x31;
   bytes[4]=0xFF;
   bytes[5]=0xFF;
   bytes[6]=0xFF;
   bytes[7]=0xFF;
   
   can->Send(0x592,bytes,8); //Send on CAN2
}

void BMW_E60::Charge_Batt_Off()
{	
   uint8_t bytes[8];
   bytes[0]=0x40;
   bytes[1]=0x14;
   bytes[2]=0xFF;
   bytes[3]=0x30;
   bytes[4]=0xFF;
   bytes[5]=0xFF;
   bytes[6]=0xFF;
   bytes[7]=0xFF;
   
   can->Send(0x592,bytes,8); //Send on CAN2
}

//E60 Check Engnie Light 
void BMW_E60::Over_heat_On()
{	
   uint8_t bytes[8];
   bytes[0]=0x40;
   bytes[1]=0x27;
   bytes[2]=0x00;
   bytes[3]=0x31;
   bytes[4]=0xFF;
   bytes[5]=0xFF;
   bytes[6]=0xFF;
   bytes[7]=0xFF;
   
   can->Send(0x592,bytes,8); //Send on CAN2
}
//End E60 Check Engnie Light
void BMW_E60::Over_heat_Off()
{	
   uint8_t bytes[8];
   bytes[0]=0x40;
   bytes[1]=0x27;
   bytes[2]=0x00;
   bytes[3]=0x30;
   bytes[4]=0xFF;
   bytes[5]=0xFF;
   bytes[6]=0xFF;
   bytes[7]=0xFF;
   
   can->Send(0x592,bytes,8); //Send on CAN2
}
//End E60 Check Engnie Light
int BMW_E60::GetCruiseState()
{
   //static int prevSel = 0;
   //int cruisesel = Cruise_ON; //todo
   int result = CC_NONE;

   if (IS_CC_RESUME)
   {
      result = CC_RESUME;
   }
   else if (IS_CC_SET)
   {
      result = CC_SET;
   }
   else if (IS_CC_CANCEL)
   {
      result = CC_CANCEL;
   }
  //else if ((cruisesel) && IS_CC_NONE(prevSel)) //todo
   //{
      ccOn = Cruise_ON;
   //}

   //prevSel = cruisesel;
   result |= ccOn ? CC_ON : CC_NONE;
   return result;
}

void BMW_E60::Reset_Cruise()
{
	Cruise_ON = 0;
	IS_CC_ON = 0;
	IS_CC_NONE = 0;
	IS_CC_SET = 0;
	IS_CC_RESUME = 0;
	IS_CC_CANCEL = 0;
}




void BMW_E60::Cruise_Control()
{	
  uint8_t Byte1;
  uint8_t Byte2;
  uint8_t Byte3 = 0xFC;
  Cruise_Speed = Param::GetInt(Param::cruisespeed);
  //Cruise_Speed = 3000;
  uint16_t CCSpeed;
  
  
  switch (Param::GetBool(Param::GearFB))
    {
    case 0:
      //CCSpeed = Cruise_Speed*0.005881507;
	  CCSpeed = Cruise_Speed*0.007949226;
      break;

    case 1:  
      //CCSpeed = Cruise_Speed*0.012072570;
      CCSpeed = Cruise_Speed*0.01631683375;
      break;
    }
  //uint16_t Bytes = map(CCSpeed, 0, 121, 40960, 43513);
  uint16_t Bytes = map(CCSpeed, 0, 163, 40960, 43513);
  Byte1 = (lo8(Bytes));
  Byte2 = (hi8(Bytes));
  
  //switch (Param::GetInt(Param::cruisestt))
  switch (Cruise_ON)
    {
    
	case 0:
      Byte1 = 0x00;
      Byte2 = 0x80;
      Byte3 = 0xFC;
	  //Param::SetInt(Param::cruisestt, 0);
      break;
	case 1:
      Byte3 = 0xFD;
	  //Param::SetInt(Param::cruisestt, 1);
      break;
    
    }
	
   uint8_t bytes[8];
   bytes[0]=Byte1;
   bytes[1]=Byte2;
   bytes[2]=Byte3;
   bytes[3]=0x00;
   bytes[4]=0x00;
   bytes[5]=0x00;
   bytes[6]=0x00;
   bytes[7]=0x00;
   can->Send(0x200,bytes,8); //Send on CAN2
}

void BMW_E60::DashOff()
{
   this->dashInit=false;
}


void BMW_E60::SendAbsDscMessages(bool Brake_In)
{
   uint8_t bytes[8];
   
   int16_t check_BA = (gear_BA+0xFF+0x0F+BA6+0xEF+0xBA);
   check_BA = (check_BA / 0x100) + (check_BA & 0xFF);
   check_BA = check_BA & 0xFF;

   bytes[0]=gear_BA; //was just 0x03
   bytes[1]=0xFF;
   bytes[2]=0x0F;
   bytes[3]=0x00;
   bytes[4]=0x00;
   bytes[5]=check_BA; //BA5; //counter byte 5
   bytes[6]=BA6; //counter byte 6
   bytes[7]=0xEF;

   can->Send(0x0BA, bytes, 8); //Send on CAN2
   BA6++;
   
   if (BA6 == 0x8F)
   {
      BA6 = 0x80;
   }

//////////send abs/dsc messages////////////////////////
	
	uint16_t Byte1 = B1_Count2; 
	uint16_t Byte2 = 0xFE;
	uint16_t Byte3 = 0x70;
	uint16_t Byte4 = 0xFE;
	uint16_t Byte5 = 0x0F;
	uint16_t Byte6 = 0x03;
	uint16_t Byte7 = 0x00;
	
	if(Ready())
	{	
		Byte2 = 0xFF;
		Byte4 = 0xFF;
		
		if(Brake_In)
	   {
		  Byte7=0x62;
		  //Cruise_ON = 0;
		  
	   }

	   else
	   {
		  Byte7=0x02;
	   }
	}
	int16_t check_A8;
	check_A8 = (Byte1+Byte2+Byte3+Byte4+Byte5+Byte6+Byte7+0xA8);
    check_A8 = (check_A8 / 0x100)+ (check_A8 & 0xFF);
    check_A8 = check_A8 & 0xFF;
		
	bytes[0]=check_A8;
    bytes[1]=Byte1;
	bytes[2]=Byte2;
	bytes[3]=Byte3;
    bytes[4]=Byte4;
    bytes[5]=Byte5;
    bytes[6]=Byte6;
    bytes[7]=Byte7;
	
    can->Send(0x0A8,bytes,8); //Send on CAN2  
	
	B1_Count2++;
	if (B1_Count2== 0x7F)
	{
		B1_Count2 = 0x70;
	}
	
   int16_t check_A9 = (A91+0xA3+0x9F+0x19+0xCC+0x3F+0x00+0xA9);
   check_A9 = (check_A9 / 0x100)+ (check_A9 & 0xFF);
   check_A9 = check_A9 & 0xFF;
   
   bytes[0]=check_A9; //first counter byte
   bytes[1]=A91; //second counter byte
   bytes[2]=0xA3;
   bytes[3]=0x9F;
   bytes[4]=0x19;
   bytes[5]=0xCC;
   bytes[6]=0x3F;
   bytes[7]=0x00;
  
   can->Send(0x0A9, bytes, 8); //Send on CAN2
   
   A91++;
   
   if (A91 == 0x0F)
   {
      A91=0x00;
   }
   
}
bool BMW_E60::GetGear(Vehicle::gear& outGear)
{
   outGear = gear;    //send the shifter pos
   return true; //Let caller know we set a valid gear
}

long BMW_E60::map(long x, long in_min, long in_max, long out_min, long out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}