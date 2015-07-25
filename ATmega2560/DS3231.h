#define DS3231 0x68       // RTC defined address    1101000    

volatile unsigned long uSec = 0;
volatile unsigned long uSecOffset = 0;

unsigned long RTC_GET_USEC(){
  uSec = micros() - uSecOffset;
  return uSec;
}

// USE THIS TO GET TIME
unsigned long RTC_GET_TIME(){
  Wire.beginTransmission(DS3231);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS3231, 3);
  unsigned long Seconds = Wire.read();
  unsigned long Minutes = Wire.read();
  unsigned long Hours   = Wire.read();
  unsigned long rtcTime =  Hours<<16 | Minutes<<8 | Seconds;
  return rtcTime;
}

// PRINT_TIME SHOULD ONLY BE USED FOR DEBUGGING
void RTC_PRINT_TIME(){
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

/*
// NOT IN USE / NO SQW HOOKED UP TO 2560
void RTC_INIT_SQW(){
  // Start 1Hz square wave
  Wire.write(0);
  Wire.write(0x0E); // move pointer to SQW address 
  Wire.endTransmission();  
  Wire.requestFrom(DS3231,1);
  uint8_t REG_0E = Wire.read();
   // Set bits 4 and 3 for 1Hz
   REG_0E &= ~(1 << 4);
   REG_0E &= ~(1 << 3);
  Wire.beginTransmission(DS3231);
  Wire.write(0x0E);
  Wire.write(0x10);
  Wire.endTransmission();
}
*/

