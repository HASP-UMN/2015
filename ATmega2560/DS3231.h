#define DS3231ADDR 0x68       // RTC defined address    1101000    
uint32_t timeStamp = 0;

void RTC_INIT(){
  // Initializes RTC
  Wire.begin(DS3231ADDR);
}

// USE THIS TO GET TIMESTAMP
uint32_t RTC_GET_TIMESTAMP(){
  Wire.beginTransmission(DS3231ADDR);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS3231ADDR, 3);
  uint32_t Seconds = Wire.read();
  uint32_t Minutes = Wire.read();
  uint32_t Hours   = Wire.read();
  timeStamp =  Hours<<16 | Minutes<<8 | Seconds;
  return timeStamp;
}

void RTC_RESET_TIME(){
  Wire.beginTransmission(DS3231ADDR);
  Wire.write(0);
  Wire.write(B00000000);  // Second 0-59
  Wire.write(B00000000);  // Minute 0-59
  Wire.write(B00000000);  // Hour 0-23 
  Wire.write(B00000001);  // Weekday 1-7
  Wire.write(B00000001);  // Date 1-31 + Century
  Wire.write(B00000001);  // Month 1-12
  Wire.write(B00010101); // Year 00-99
  Wire.write(byte(0));
  Wire.endTransmission();
}

void RTC_PRINT_TIME(){// For Debugging Purposes
  Wire.beginTransmission(DS3231ADDR);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS3231ADDR, 3);
  byte Seconds = Wire.read();
  byte Minutes = Wire.read();
  byte Hours   = Wire.read();
  Serial.print(Hours,HEX);
  Serial.print(":");
  Serial.print(Minutes,HEX);
  Serial.print(":");
  Serial.println(Seconds,HEX);
  }

// Conversion Functions
byte decToBcd(byte val){
  // Convert normal decimal numbers to binary coded decimal
  return ( (val/10*16) + (val%10) );}
byte bcdToDec(byte val){
  // Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );}
