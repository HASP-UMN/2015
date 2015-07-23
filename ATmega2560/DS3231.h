#define DS3231ADDR 0x68       // RTC defined address    1101000    

uint32_t timeStamp;
volatile uint32_t usec;
volatile uint32_t usec_offset;

void RTC_INIT(){
  // Initializes RTC
  Wire.begin(DS3231ADDR);

/* Commented out; square wave pin not connected on current setup.
  // Start 1Hz square wave
  Wire.write(0);
  Wire.write(0x07); // move pointer to SQW address 
  Wire.write(0x10); // sends 0x10(hex) : 1Hz Square Wave 
  Wire.endTransmission();  
*/
}

uint16_t RTC_GET_USEC(){
  usec = micros() - usec_offset;
  return round(usec/10);
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
