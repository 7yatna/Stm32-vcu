#ifndef BMW_E60_h
#define BMW_E60_h
#define lo8(x) (uint8_t)((x)&0xff)
#define hi8(x) (uint8_t)(((x)>>8)&0xff)

/*  This library supports the Powertrain CAN messages for the BMW E60 for driving dash gauges, putting out malf lights etc
    Also reads gear lever, brake lights etc

*/

#include <stdint.h>
#include "vehicle.h"
#include "my_math.h"
#include "throttle.h"

class BMW_E60: public Vehicle
{
public:
   BMW_E60() : terminal15On(false), dashInit(false), gear(PARK) { }
   void SetCanInterface(CanHardware*);
   void Send_RPM();
   void Task10Ms();
   void Task100Ms();
   void Task200Ms();
   void Gear_Display();
   void Gear2();
   void Dodge_PS();
   void AC_Comp();
   void Cruise_Control();
   void Reset_Cruise();
   int GetCruiseState();
   void Engine_Data();
   void Heatflow();
   void DME_Alive();
   void Batt_Voltage_3B3();
   void Batt_Voltage_3B4();
   void Temp_Gauge();
   void Check_Engine_On();
   void Check_Engine_Off();
   void Over_heat_On();
   void Over_heat_Off();
   void Charge_Batt_On();
   void Charge_Batt_Off();
   void SetRevCounter(int speed) { revCounter = speed; }
   void SetTemperatureGauge(float temp) { temperature = temp; }
   void DecodeCAN(int, uint32_t* data);
   bool Ready() { return terminal15On; }
   bool Start() { return terminal15On; }
   bool GetGear(Vehicle::gear& outGear);
   void DashOff();
   long map(long, long, long, long, long);


private:
   void SendAbsDscMessages(bool Brake_In);
   bool terminal15On;
   bool dashInit;
   bool lastCruiseSwitchState;
   Vehicle::gear gear;
   Vehicle::cruise cruise;
   int revCounter;
   int Cruise_Speed;
   float temperature;
   uint8_t timer500=0;
   bool ccOn;
};

#endif /* BMW_E60_h */

