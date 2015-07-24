#define DS3231 0x68       // RTC defined address    1101000    

volatile uint32_t rtcTime = 0;
volatile uint32_t usec = 0;
volatile uint32_t usec_offset = 0;

uint16_t RTC_GET_USEC(){
  usec = micros() - usec_offset;
  return round(usec/10);
}

// USE THIS TO GET TIME
uint32_t RTC_GET_TIME(){
  Wire.beginTransmission(DS3231);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS3231, 3);
  uint32_t Seconds = Wire.read();
  uint32_t Minutes = Wire.read();
  uint32_t Hours   = Wire.read();
  rtcTime =  Hours<<16 | Minutes<<8 | Seconds;
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
  Serial.println(Seconds,HEX);
  }

/* NOT IN USE / NO SQW HOOKED UP TO 2560
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

// Conversion Functions
byte decToBcd(byte val){
  // Convert normal decimal numbers to binary coded decimal
  return ( (val/10*16) + (val%10) );}
byte bcdToDec(byte val){
  // Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );}
