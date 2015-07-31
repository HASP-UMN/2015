#include <SPI.h>
#include <Wire.h>
#include "atmega2560.h"
#include "TIMING.h"

// Addresses and commands for A/D operation (SPI interface). See ADS8634 datasheet.
#define ADC_CONFIG       0B00000110 // Enable internal Vref and temperature sensor.
#define READ_CH1         0B01001010 // Read A/D channel 2 (MCA ch.1) with 0 to 10V range.
#define READ_CH2         0B01101010 // Read A/D channel 3 (MCA ch.2) with 0 to 10V range.
#define READ_CH3         0B00001010 // Read A/D channel 0 (MCA ch.3) with 0 to 10V range.
#define READ_CH4         0B00101010 // Read A/D channel 1 (MCA ch.4) with 0 to 10V range.
#define READ_TEMP        0B00000001 // Read temperature sensor.
#define CONFIG_ADDR      0x06 // 7-bit internal control register address
#define MANUAL_READ_ADDR 0x04 // 7-bit address for manual mode read register

// Peak threshhold discriminators. INT[7-4] on the ATmega2560 (inputs).
#define DISCRIMINATOR1 7 // INT7: Channel 1 discrimintor.
#define DISCRIMINATOR2 6 // INT6: Channel 2 discrimintor.
#define DISCRIMINATOR3 5 // INT5: Channel 3 discrimintor.
#define DISCRIMINATOR4 4 // INT4: Channel 4 discrimintor.

  // Port masks for Port H
#define ADC_CS   0B01000000
#define PK_RST1  0B00100000
#define PK_RST2  0B00010000
#define PK_RST3  0B00001000
#define PK_RST4  0B00000100
#define FIFO_RST 0B00000010
#define FIFO_WR  0B00000001

// Checksum definitions
#define H3 36344967696
#define H4 4841987667533046032
unsigned int checksum = 0;
byte startByte = 0x00;

// Time Data definitions
unsigned int ticStamp  = 0; // used for passing current  sec to data stream.
unsigned long uSecStamp = 0; // used for passing current usec to data stream.

// FIFO full flag. Pin 88 on the ATmega2560
volatile bool FIFO_full_flag = false;
//uint16_t tempRaw = 0; // A/D converter's internal temp sensor
//uint8_t  channel = 0; // Channel no. [1-4]
volatile bool newEventCH1 = false; // New event flag for channel 1
volatile bool newEventCH2 = false; // New event flag for channel 2
volatile bool newEventCH3 = false; // New event flag for channel 3
volatile bool newEventCH4 = false; // New event flag for channel 4

//declarations of data structs for each channel; see atmega2560.h
ADC_data data_ch1;
ADC_data data_ch2;
ADC_data data_ch3;
ADC_data data_ch4;

// FIFO interrupt service routine for the FF signal
void FIFO_FF_ISR() {
  FIFO_full_flag = true;
  // The Full Flag (FF) will go LOW, inhibiting further write operation,
  // when the write pointer is one location less than the read pointer,
  // indicating that the device is full. If the read pointer is not moved
  // after Reset (RS), the Full-Flag (FF) will go LOW after 256 writes for
  // IDT7200, 512 writes for the IDT7201A and 1,024 writes for the IDT7202A.
}

// Ch[1-4] interrupt service routines for threshhold discriminator signals
ISR(INT7_vect) {
  if (!newEventCH1 && !newEventCH2 && !newEventCH3 && !newEventCH4)
    newEventCH1 = true;
  // Ignore new events if another event (on any channel) is currently being processed
}
ISR(INT6_vect) {
  if (!newEventCH1 && !newEventCH2 && !newEventCH3 && !newEventCH4)
    newEventCH2 = true;
  // Ignore new events if another event (on any channel) is currently being processed
}
ISR(INT5_vect) {
  if (!newEventCH1 && !newEventCH2 && !newEventCH3 && !newEventCH4)
    newEventCH3 = true;
  // Ignore new events if another event (on any channel) is currently being processed
}
ISR(INT4_vect) {
  if (!newEventCH1 && !newEventCH2 && !newEventCH3 && !newEventCH4)
    newEventCH4 = true;
  // Ignore new events if another event (on any channel) is currently being processed
}

// Interrupt Service Routines for GPS_PPS
ISR(INT3_vect) {
  uSecOffset = micros(); 
  ticCount++;
  }  

void setup() {  
  // Initialize and configure SPI bus for A/D communications
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2);  // 8 Mhz SPI clock
  SPI.setBitOrder(MSBFIRST);            // Most-significant bit first
  SPI.setDataMode(SPI_MODE0);           // Clock Polarity = 0; clock phase = 0
  
  // Open serial port
  Serial.begin(115200); Serial.flush();
  Serial.println("Channel, PeakVal, TempRaw, Seconds, uSeconds, rtcTime");
  Serial.println("=====================================================");

  cli(); // HOLD interrupts while new interrupts are set.
  
  // Set up Port H on the ATmega2560 as an output port. DDRH is the direction register for Port H.
  DDRH = DDRH | B01111111;
  // Initialize the digital outputs. PORTH is the register for the state of the outputs.
  PORTH = B01000011;
  
  // PORT F (digital output)
  // Set up Port F on the ATmega2560 as an output port. DDRF is the direction register for Port F.
  DDRF = DDRF | B11111111;
  // Initialize the digital outputs on Port F to low. PORTF is the register for the state of the outputs.
  PORTF = B00000000;
 
  // PORT K (FIFO full flag on PK1)
  DDRK   = DDRK   & ~B00000010;    // Set PK1 as input
  PORTK  = PORTK  |  B00000010;    // Activate PULL UP resistor
  PCMSK1 = PCMSK1 |  B00000010;    // Enable PCINT17 on PK1
  PCICR  = PCICR  | (1<<PCIE2);    // Activate interrupt on enabled PCINT23-16
 
  // PORT E (CH[1-4] threshhold discriminators)
  DDRE  = DDRE  & ~B11110000;  /* Set PE7 (DISCRIMINATOR1) as input */
  PORTE = PORTE & ~B11110000;  /* Activate PULL DOWN resistors */
  EICRB = B11111111; // Set INT[4-7] to be on their rising edges
  EIMSK = B11110000; // Enable INT[4-7] 
  
  // PORT D (GPS_PPS as Input)
  DDRD = DDRD | B00000000;         // Sets PD3 as input
  EICRA = B11000000;               // Set INT3 to be rising edge
  EIMSK = EIMSK | B00001000;       // Enables INT[3]
   
  sei(); // RE-ENABLE Interrupts after new interrupts are set.
   
  // Reset the FIFO.
  PORTH |= FIFO_WR; // set FIFO_WR high before resetting
  PORTH &= ~FIFO_RST; // Toggle FIFO_RST pin from HIGH to LOW
  PORTH = PORTH |  FIFO_RST; // Toggle FIFO_RST pin from LOW to HIGH  

  // Write A/D configuration register to enable internal Vref and temperature sensor 
  PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
  delayMicroseconds(5);
  SPI.transfer(CONFIG_ADDR << 1); // Must shift address 1 bit left for write bit
  SPI.transfer(ADC_CONFIG);
  PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH

  // Initialize data structs
  data_ch1.read_channel = READ_CH1;
  data_ch2.read_channel = READ_CH2;
  data_ch3.read_channel = READ_CH3;
  data_ch4.read_channel = READ_CH4;
  
  data_ch1.send_channel = 1;
  data_ch2.send_channel = 2;
  data_ch3.send_channel = 3;
  data_ch4.send_channel = 4;
  
  data_ch1.reset = PK_RST1;
  data_ch2.reset = PK_RST2;
  data_ch3.reset = PK_RST3;
  data_ch4.reset = PK_RST4;

  delay(120000); // Allows TOMCAT to boot and start data collection before sending data.

  // Start RTC and send System Start Time to FIFO
  Wire.begin(DS3231);   // Initializes RTC 
  unsigned int init_time1 = 0;
  unsigned int init_time2 = 0;
  unsigned long start_time = 0;
  unsigned long start_usec_offset = 0;
  bool sec_change = false;
    Wire.beginTransmission(DS3231);
    Wire.write(0);
    Wire.endTransmission();
    Wire.requestFrom(DS3231, 1);
    init_time2 = Wire.read();
  // while loop must poll RTC to get most accurate second change nearest to first gpsPPS tic
  while(sec_change == false){ 
    init_time1 = init_time2;
    start_usec_offset = micros();
    Wire.beginTransmission(DS3231);
    Wire.write(0);
    Wire.endTransmission();
    Wire.requestFrom(DS3231, 1);
    init_time2 = Wire.read();
    sec_change = (init_time1 != init_time2);
    ticCount = 0;
    }
  while(ticCount == 0){
    /*
     This while holds the program until the gpsPPS starts a tic count.
     After the first incrementation of ticCount, the offset from RTC
     second/tic will be calculated.
     */
    }
    start_usec_offset = micros() - start_usec_offset;
    Wire.beginTransmission(DS3231);
    Wire.write(0x01);
    Wire.endTransmission();
    Wire.requestFrom(DS3231, 2);
    unsigned int Minutes = Wire.read();
    byte Hours           = Wire.read();
    start_time =  Minutes<<8 | init_time2;
    send_data(0x00, Hours, start_time, start_usec_offset, 0xFFFF, 0xFFFF, 0xFFFF);

    // Debugging print statements below.
    Serial.print("TIMEPACKET,         "); Serial.print(Hours,BIN); Serial.print(start_time,BIN); Serial.print(", "); Serial.println(start_usec_offset,BIN);
    Serial.print("TIMEPACKET DECODED, "); Serial.print(Hours,HEX); Serial.print(start_time,HEX); Serial.print(", "); Serial.println(start_usec_offset,HEX);
    Serial.print("TIMEPACKET DECODED, "); Serial.print(Hours,DEC); Serial.print(start_time,DEC); Serial.print(", "); Serial.println(start_usec_offset,DEC);

} // end SETUP

void loop() {

  if (FIFO_full_flag) {
     FIFO_full_flag = false;
     // if the FF flag is set in the beginning of the loop (i.e. at boot or after a reset), unset it
     // this really should never happen and if it does we should send an error code 
  }
  
  if (newEventCH1) {
    
    uSecStamp = RTC_GET_USEC();  // Get uSeconds from start of most recent gpsPPS (tic).
    ticStamp  = ticCount;        // Get current gpsPPS number of seconds since data begin (ticCount).
    
    data_ch1 = get_data(data_ch1);
    
    checksum =  getChecksum(data_ch1.send_channel);
    checksum += getChecksum(uSecStamp);
    checksum += getChecksum(ticStamp);
    checksum += getChecksum(data_ch1.peak_val);
    checksum += getChecksum(data_ch1.tempRaw);
    
    send_data(startByte, data_ch1.send_channel, ticStamp, uSecStamp, data_ch1.peak_val, data_ch1.tempRaw, checksum);

    //debugging print statements in function below
    print_debug(data_ch1, "1", ticStamp, uSecStamp, checksum);

    // Reset peak value and interrupt flag for CH1
    newEventCH1 = false;
    data_ch1.peak_val = 0;
    delay(100);
  }

  if (newEventCH2) {
    
    uSecStamp = RTC_GET_USEC();  // Get uSeconds from start of most recent gpsPPS (tic).
    ticStamp  = ticCount;        // Get current gpsPPS number of seconds since data begin (ticCount).
    
    data_ch2 = get_data(data_ch2);

    checksum = getChecksum(data_ch2.send_channel);
    checksum += getChecksum(uSecStamp);
    checksum += getChecksum(ticStamp);
    checksum += getChecksum(data_ch2.peak_val);
    checksum += getChecksum(data_ch2.tempRaw);    
    
    send_data(startByte, data_ch2.send_channel, ticStamp, uSecStamp, data_ch2.peak_val, data_ch2.tempRaw, checksum);
    
    //debugging print statements in function below
    print_debug(data_ch2, "2", ticStamp, uSecStamp, checksum);
    
    // Reset peak value and interrupt flag for CH2
    newEventCH2 = false;
    data_ch2.peak_val = 0;
    delay(100);
  }

  if (newEventCH3) {
    
    uSecStamp = RTC_GET_USEC();  // Get uSeconds from start of most recent gpsPPS (tic).
    ticStamp  = ticCount;        // Get current gpsPPS number of seconds since data begin (ticCount).
    
    data_ch3 = get_data(data_ch3);
    
    checksum = getChecksum(data_ch3.send_channel);
    checksum += getChecksum(uSecStamp);
    checksum += getChecksum(ticStamp);
    checksum += getChecksum(data_ch3.peak_val);
    checksum += getChecksum(data_ch3.tempRaw);    
    
    send_data(startByte, data_ch3.send_channel, ticStamp, uSecStamp, data_ch3.peak_val, data_ch3.tempRaw, checksum);    

    //debugging print statements in function below
    print_debug(data_ch3, "3", ticStamp, uSecStamp, checksum);
    
    // Reset peak value and interrupt flag for CH3
    newEventCH3 = false;
    data_ch3.peak_val = 0;
    delay(100);    
  }

  if (newEventCH4) {
    
    uSecStamp = RTC_GET_USEC();  // Get uSeconds from start of most recent gpsPPS (tic).
    ticStamp  = ticCount;        // Get current gpsPPS number of seconds since data begin (ticCount).
    
    data_ch4 = get_data(data_ch4);

    checksum = getChecksum(data_ch4.send_channel);
    checksum += getChecksum(uSecStamp);
    checksum += getChecksum(ticStamp);
    checksum += getChecksum(data_ch4.peak_val);
    checksum += getChecksum(data_ch4.tempRaw);

    send_data(startByte, data_ch4.send_channel, ticStamp, uSecStamp, data_ch4.peak_val, data_ch4.tempRaw, checksum);

    //debugging print statements in function below
    print_debug(data_ch4, "4", ticStamp, uSecStamp, checksum);

    // Reset peak value and interrupt flag for CH4
    newEventCH4 = false;
    data_ch4.peak_val = 0;
    delay(100);
  }
}

ADC_data get_data(ADC_data data) {
    
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(data.read_channel);
    PORTH = PORTH | ADC_CS; // Toggle ADC_CS HIGH
      
    // Read temp sensor
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(READ_TEMP);
    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH

    // Get channel data
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    data.peak_val = SPI.transfer(0) & 0x0F;
    data.peak_val = data.peak_val << 8;
    data.peak_val += SPI.transfer(0);
    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH
    
    // Get temp sensor data
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    data.tempRaw = SPI.transfer(0) & 0xFF;
    data.tempRaw = data.tempRaw << 8;
    data.tempRaw += SPI.transfer(0);
    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH

    // Reset the peak detector. PH3 (the 4th bit of Port H) is the CH3 reset signal.
    PORTH = PORTH | data.reset; // Toggle PK_RST3 HIGH
    PORTH = PORTH & ~data.reset; // Toggle PK_RST3 LOW
    return data;       
}

void send_data(byte startByte, byte channel, unsigned int ticStamp, 
               unsigned long uSecStamp, uint16_t peak, uint16_t tempRaw, uint16_t checksum){

    PORTF = (startByte | channel);               //1rst byte        
    PORTH = PORTH & ~FIFO_WR; 
    PORTH = PORTH |  FIFO_WR; 
  
    PORTF = (ticStamp  & 0xFF);              //2nd byte        
    PORTH = PORTH & ~FIFO_WR; 
    PORTH = PORTH |  FIFO_WR; 
  
    PORTF = (ticStamp  & 0xFF00)>>8;         //3rd byte        
    PORTH = PORTH & ~FIFO_WR;
    PORTH = PORTH |  FIFO_WR;
        
    PORTF = (uSecStamp  & 0xFF);              //4th byte        
    PORTH = PORTH & ~FIFO_WR; 
    PORTH = PORTH |  FIFO_WR; 
  
    PORTF = (uSecStamp  & 0xFF00)>>8;         //5th byte        
    PORTH = PORTH & ~FIFO_WR;
    PORTH = PORTH |  FIFO_WR;

    PORTF = (uSecStamp  & 0xFF0000)>>16;      //6th byte    
    PORTH = PORTH & ~FIFO_WR;
    PORTH = PORTH |  FIFO_WR;
    
    PORTF = (peak & 0xFF);                    //7th byte    
    PORTH = PORTH & ~FIFO_WR;
    PORTH = PORTH |  FIFO_WR;
    
    PORTF = (peak & 0xFF00)>>8;               //8th byte        
    PORTH = PORTH & ~FIFO_WR;
    PORTH = PORTH |  FIFO_WR;
    
    PORTF = (tempRaw & 0xFF);                 //9th byte    
    PORTH = PORTH & ~FIFO_WR;
    PORTH = PORTH |  FIFO_WR;

    PORTF = (tempRaw & 0xFF00)>>8;            //10th byte    
    PORTH = PORTH & ~FIFO_WR;
    PORTH = PORTH |  FIFO_WR;
    
    PORTF = (checksum & 0xFF);                //11th byte    
    PORTH = PORTH & ~FIFO_WR;
    PORTH = PORTH |  FIFO_WR;
    
    PORTF = (checksum & 0xFF00)>>8;           //12th byte    
    PORTH = PORTH & ~FIFO_WR;
    PORTH = PORTH |  FIFO_WR;
    
}

void print_debug(ADC_data data, char* channel_char, unsigned long ticStamp, unsigned long uSecStamp, uint16_t checksum) {
  
    uint8_t  channel = data.read_channel;
    uint16_t peak_val = data.peak_val;
    uint16_t tempRaw = data.tempRaw;

/*
byte startByte, byte channel, unsigned int ticStamp, unsigned long uSecStamp, uint16_t peak, uint16_t tempRaw, uint16_t checksum
 */


    Serial.print("FF");Serial.print(channel_char); Serial.print(", ");
    Serial.print(ticStamp,HEX); Serial.print(" ");
    Serial.print(uSecStamp,HEX); Serial.print(" ");
    Serial.print(peak_val,HEX); Serial.print(", ");
    Serial.print(tempRaw,HEX); Serial.print(", ");
    Serial.print(checksum,HEX); Serial.print(",");
    
    
    Serial.print("Raw rtc Time:");
    RTC_PRINT_TIME();
    Serial.println();
}

unsigned int getChecksum(unsigned int value) {
    unsigned int h1, h2, h3;
    unsigned int chksum;
    
    h1 = 0; h2 = 0;
    h1 = (value & 0xF) << 2;
    h2 = H4 >> h1;
    h1 = h2 & 0xF;
    h2 = h1 << 2;
    h1 = H3 >> h2;
    h3 = 0; chksum = 0;
    h3 = (value & 0xF0) >> 2;
    h2 = H4 >> h3 & 0xF;
    h3 = h2 << 2;

    chksum = (h1 >> h3) & 0xF;
    return chksum;
}
