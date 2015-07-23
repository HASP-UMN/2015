#define DS3231 0x68       // RTC defined address    1101000    

uint32_t timeStamp = 0;
volatile uint32_t usec = 0;
volatile uint32_t usec_offset = 0;

void RTC_INIT(){
  // Initializes RTC
  Wire.begin(DS3231);

  // Start 1Hz square wave
  Wire.write(0);
  Wire.write(0x07); // move pointer to SQW address 
  Wire.write(0x10); // sends 0x10(hex) : 1Hz Square Wave 
  Wire.endTransmission();  
}

uint16_t RTC_GET_USEC(){
  usec = micros() - usec_offset;
  return round(usec/10);
}

// USE THIS TO GET TIMESTAMP
uint32_t RTC_GET_TIMESTAMP(){
  Wire.beginTransmission(DS3231);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS3231, 3);
  uint32_t Seconds = Wire.read();
  uint32_t Minutes = Wire.read();
  uint32_t Hours   = Wire.read();
  timeStamp =  Hours<<16 | Minutes<<8 | Seconds;
  return timeStamp;
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

// Conversion Functions
byte decToBcd(byte val){
  // Convert normal decimal numbers to binary coded decimal
  return ( (val/10*16) + (val%10) );}
byte bcdToDec(byte val){
  // Convert binary coded decimal to normal decimal numbers
  return ( (val/16*10) + (val%16) );}
