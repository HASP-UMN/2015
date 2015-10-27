/*//////////////////////////////////////////////////////////////////////

                        UNIVERSITY OF MINNESOTA
  * 2015 HASP TIMING
  * Authored - Charles Denis 7/15/2015
    * UPDATES:
      * Updated to contain all timing declarations and functions- CD 7/28/2015 
      * 

*///////////////////////////////////////////////////////////////////////

// DEFINITIONS:
#define DS3231 0x68       // RTC defined address    1101000    

// VARIABLES:
volatile unsigned int ticCount = 0;
volatile unsigned long uSec = 0;
volatile unsigned long uSecOffset = 0;

// FUNCTIONS:
unsigned long RTC_GET_USEC(){
  uSec = micros() - uSecOffset;
  return uSec;
}
unsigned long RTC_GET_TIME(){
  Wire.beginTransmission(DS3231);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS3231, 3);
  unsigned long Seconds = Wire.read();
  unsigned long Minutes = Wire.read();
  unsigned long Hours   = Wire.read();
  unsigned long rtcTime =  Hours<<14 | Minutes<<7 | Seconds;
  return rtcTime;
}
int RTC_GET_TEMP(){
  Wire.beginTransmission(DS3231);
  Wire.write(0x11);
  Wire.endTransmission();
  Wire.requestFrom(DS3231, 2);
  int MSB = Wire.read();
  unsigned int LSB = Wire.read();
  int rtcTemp =  MSB<<8 | LSB>>1;
  return rtcTemp;
}
void RTC_PRINT_TIME(){// !!! PRINT_TIME SHOULD ONLY BE USED FOR DEBUGGING !!!
  Wire.beginTransmission(DS3231);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS3231, 3);
  byte Seconds = Wire.read();
  byte Minutes = Wire.read();
  byte Hours   = Wire.read();
  Serial.print(Hours,HEX);
  Serial.print(":");
  Serial.print(Minutes,HEX);
  Serial.print(":");
  Serial.print(Seconds,HEX);
}

