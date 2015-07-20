// HASP timing functions
// Testing on Arduino Mega with DS1307 RTC
//  authored: Charles Denis
//  date: 06/29/2015
//    * RTC communication with init, read, reset, 
//      and conversion functions.
//
//  last updated: 7/14/15 -chd
//    * configured gps interrupts and scheduling 
//    * configured output of timeStamp format
//      * size constraints dictate time must me split into two variables
//      * time_hm contains Hours:Minutes:Seconds in Binary Coded Decimal
//      * timeuSec  contains microseconds in Binary Coded Decimal
//********************************************************************//

// Library Includes 
#include <Wire.h>

// Definitions
#define gpsPV 47              // GPS Position Valid     Port D Pin 4
#define gpsPulse 46           // GPS Pulse Per Second   Port D Pin 3
#define rtcPulse 45           // RTC Square Wave Pulse  Port D Pin 2
#define DS3231addr 0x68       // RTC defined address    1101000    

// Variables
volatile uint32_t usec;
volatile uint32_t usec_offset;
volatile uint32_t time_hm;
volatile uint16_t time_uSec;

// Interrupts
void realTimeISR(){usec_offset = micros();} // While GPS position NOT VALID rtc 1hz pulse drives time
void gpsPulseISR(){usec_offset = micros();} // While GPS position VALID, gps 1hz pulse drives time
void gpsPVonISR(){detachInterrupt(rtcPulse); attachInterrupt(gpsPulse, gpsPulseISR, RISING);} // Switches ownership of time to gpsPulse
void gpsPVoffISR(){detachInterrupt(gpsPulse); attachInterrupt(rtcPulse, realTimeISR, FALLING);} // Switches ownership of time to rtcPulse

void setup(){
  Serial.begin(115200);// Sets up SerialMonitor only. Not needed for flight.
  // Starts I2C Communication with RTC
  Wire.begin(DS1307addr);  // Initialize Square Wave Oscillator
  Wire.beginTransmission(DS1307addr);
  Wire.write(0);
  Wire.write(0x07); // move pointer to SQW address 
  Wire.write(0x10); // sends 0x10(hex) : 1Hz Square Wave 
  Wire.endTransmission();  
  attachInterrupt(rtcPulse, realTimeISR, FALLING); 
  attachInterrupt(gpsPV, gpsPVonISR, RISING);
  attachInterrupt(gpsPV, gpsPVoffISR,FALLING);
}

uint32_t get_time_hm(){
  Wire.beginTransmission(DS1307addr);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS1307addr, 3);
  uint32_t Seconds = Wire.read();
  uint32_t Minutes = Wire.read();
  uint32_t Hours   = Wire.read();
  time_hm =  Hours<<16 | Minutes<<8 | Seconds;
  return time_hm;
}

uint16_t get_time_uSec(){
  usec = micros() - usec_offset;
  timeusec =  decToBcd(round(usec/10));
  return time_uSec;
}

void rtcReadTime(){// For Debugging Purposes
  Wire.beginTransmission(DS1307addr);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS1307addr, 3);
  byte Seconds = Wire.read();
  byte Minutes = Wire.read();
  byte Hours   = Wire.read();
  Serial.print(Hours,HEX);
  Serial.print(":");
  Serial.print(Minutes,HEX);
  Serial.print(":");
  Serial.println(Seconds,HEX);
  }

// For RTC setup only.
void reset_RTC(){
  // ::WARNING::
  // ONLY SET THIS TO A MANUAL TYPE (BUTTON) RESET!!!
  // THIS WILL RESET RTC CLOCK TO DEFINED VALUE
  Wire.beginTransmission(DS1307addr);
  Wire.write(0);
  Wire.write(decToBcd(0));
  Wire.write(decToBcd(00));
  Wire.write(decToBcd(00));
  Wire.write(decToBcd(1));
  Wire.write(decToBcd(1));
  Wire.write(decToBcd(1));
  Wire.write(decToBcd(1));
  Wire.endTransmission();
}


//MAIN LOOP
void loop(){
///////////////////////////////////////////////////////////////////////////    
    Serial.print(getTime());
///////////////////////////////////////////////////////////////////////////  
}

// Conversion Functions
byte decToBcd(byte val){
  // Convert normal decimal numbers to binary coded decimal
  return ( (val/10*16) + (val%10) );}
byte bcdToDec(byte val){
  // Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );}
