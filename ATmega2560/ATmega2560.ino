#include <SPI.h>
#include <Wire.h>
#include "atmega2560.h"
#include <avr/interrupt.h>

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

// FIFO full flag. Pin 88 on the ATmega2560
#define FIFO_FF A9
volatile bool FIFO_full_flag = false;

//uint16_t tempRaw = 0; // A/D converter's internal temp sensor
//uint8_t  channel = 0; // Channel no. [1-4]
volatile bool newEventCH1 = false; // New event flag for channel 1
volatile bool newEventCH2 = false; // New event flag for channel 2
volatile bool newEventCH3 = false; // New event flag for channel 3
volatile bool newEventCH4 = false; // New event flag for channel 4
//uint16_t peakCH1 = 0; // Peak value for channel 1
//uint16_t peakCH2 = 0; // Peak value for channel 2
//uint16_t peakCH3 = 0; // Peak value for channel 3
//uint16_t peakCH4 = 0; // Peak value for channel 4


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

// Declarations for TIMING
unsigned long timeMs = 0; // Time in milliseconds
#define gpsPV 47              // GPS Position Valid     Port D Pin 4
#define gpsPulse 46           // GPS Pulse Per Second   Port D Pin 3
#define rtcPulse 45           // RTC Square Wave Pulse  Port D Pin 2
#define DS3231addr 0x68       // RTC defined address    1101000    
void realTimeISR(){usec_offset = micros();} // While GPS position NOT VALID rtc 1hz pulse drives time
void gpsPulseISR(){usec_offset = micros();} // While GPS position VALID, gps 1hz pulse drives time
void gpsPVonISR(){detachInterrupt(rtcPulse); attachInterrupt(gpsPulse, gpsPulseISR, RISING);} // Switches ownership of time to gpsPulse
void gpsPVoffISR(){detachInterrupt(gpsPulse); attachInterrupt(rtcPulse, realTimeISR, FALLING);} // Switches ownership of time to rtcPulse
volatile uint32_t usec;
volatile uint32_t usec_offset;
volatile uint32_t time_hm;
volatile uint16_t time_uSec;



void setup() {
  
  // Set up Port H on the ATmega2560 as an output port. DDRH is the direction register for Port H.
  DDRH = DDRH | B01111111;
  // Set up Port F on the ATmega2560 as an output port. DDRF is the direction register for Port F.
  DDRF = DDRF | B11111111;
  
  
  // Initialize the digital outputs. PORTH is the register for the state of the outputs.
  PORTH = B01000011;
  // Initialize the digital outputs on Port F to low. PORTF is the register for the state of the outputs.
  PORTF = B00000000;


  // Reset the FIFO.
  PORTH |= FIFO_WR; // set FIFO_WR high before resetting
  PORTH &= ~FIFO_RST; // Toggle FIFO_RST pin from HIGH to LOW
  PORTH = PORTH |  FIFO_RST; // Toggle FIFO_RST pin from LOW to HIGH  
  
    // Configure interrupts for all four threshhold discriminators
  EICRB = 0xFF; // Set INT[4-7] to be on their rising edges
  EIMSK = 0xF0; // Enable INT[4-7]
  
  // Configure interrupts for the FIFO FF
  //etc.
  //etc.
  
    // Write A/D configuration register to enable internal Vref and temperature sensor 
  PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
  delayMicroseconds(5);
  SPI.transfer(CONFIG_ADDR << 1); // Must shift address 1 bit left for write bit
  SPI.transfer(ADC_CONFIG);
  PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH
  delay(100);

  // Initialize data structs
  data_ch1.channel = 1;
  data_ch2.channel = 2;
  data_ch3.channel = 3;
  data_ch4.channel = 4;
  
  // Initialize and configure SPI bus for A/D communications
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2);   // 8 MHz SPI clock
  SPI.setBitOrder(MSBFIRST);             // Most-significant bit first
  SPI.setDataMode(SPI_MODE0);            // Clock polarity = 0, clock phase = 0
  delay(100);

  // Initialize and configure RTC
  Wire.begin(DS1307addr);
  Wire.beginTransmission(DS1307addr);  // Initialize Square Wave Oscillator
  Wire.write(0);
  Wire.write(0x07); // move pointer to SQW address 
  Wire.write(0x10); // sends 0x10(hex) : 1Hz Square Wave 
  Wire.endTransmission();
  attachInterrupt(rtcPulse, realTimeISR, FALLING); 
  attachInterrupt(gpsPV, gpsPVonISR, RISING);
  attachInterrupt(gpsPV, gpsPVoffISR,FALLING);  
  
  // Configure interrupts for all four threshhold discriminators
  EICRB = 0xFF; // Set INT[4-7] to be on their rising edges
  EIMSK = 0xF0; // Enable INT[4-7]
  
  //initialize data structs
  data_ch1.channel = 1;
  data_ch2.channel = 2;
  data_ch3.channel = 3;
  data_ch4.channel = 4;
  
//  attachInterrupt(DISCRIMINATOR1, eventISR_CH1, RISING); //INT7
//  attachInterrupt(DISCRIMINATOR2, eventISR_CH2, RISING); //INT6
//  attachInterrupt(DISCRIMINATOR3, eventISR_CH3, RISING); //INT5
//  attachInterrupt(DISCRIMINATOR4, eventISR_CH4, RISING); //INT4


  // Configure interrupts for the FIFO FF
  //etc.
  //etc.


  // Print data header
  Serial.print("channel"); Serial.print(',');
  Serial.print("timeMs");  Serial.print(',');
  Serial.print("peak");    Serial.print(',');
  Serial.println("temperature");
   
  delay(100);

  // Open serial port
  Serial.begin(115200); Serial.flush();
  Serial.println("channel,timeMs,peak,temperature");
  Serial.println("=============================");
  
}

void loop() {
  
  if (FIFO_full_flag) {
     FIFO_full_flag = false;
     // if the FF flag is set in the beginning of the loop (i.e. at boot or after a reset), unset it
     // this really should never happen and if it does we should send an error code 
  }
  
  
  
  
  if (newEventCH1) {
    
    timeMs = millis();  // Get timestamp in milliseconds

    Serial.println("Entering newEventCH1()!");
    data_ch1 = get_data(data_ch1);
    //send_data(data_ch1.channel, timeMs, data_ch1.peak_val, data_ch1.tempRaw);
    
    //debugging print statements in function below
    print_debug(data_ch1, "1", timeMs);
    
    // Reset peak value and interrupt flag for CH1
    newEventCH1 = false;
    delay(1000);
    
  }




  if (newEventCH2) {
    
    timeMs = millis();  // Get timestamp in milliseconds

    Serial.println("Entering newEventCH2()!");    
    data_ch2 = get_data(data_ch2);
    //send_data(data_ch2.channel, timeMs, data_ch2.peak_val, data_ch2.tempRaw);
    
    //debugging print statements in function below
    print_debug(data_ch2, "2", timeMs); 

    // Reset peak value and interrupt flag for CH2
    newEventCH2 = false;
    delay(1000);
    
  }




  if (newEventCH3) {
    
    timeMs = millis();  // Get timestamp in milliseconds

    Serial.println("Entering newEventCH3()!");
    data_ch3 = get_data(data_ch3);
    //send_data(data_ch3.channel, timeMs, data_ch3.peak_val, data_ch3.tempRaw);    
    
    //debugging print statements in function below
    print_debug(data_ch3, "3", timeMs);   
    
    // Reset peak value and interrupt flag for CH3
    newEventCH3 = false;
    delay(1000);
    
  }



  if (newEventCH4) {
    
    timeMs = millis();  // Get timestamp in milliseconds

    Serial.println("Entering newEventCH4()!");
    data_ch4 = get_data(data_ch4);  
    //send_data(data_ch4.channel, timeMs, data_ch4.peak_val, data_ch4.tempRaw);    
    
    //debugging print statements in function below
    print_debug(data_ch4, "4", timeMs);

    // Reset peak value and interrupt flag for CH4
    newEventCH4 = false;
    delay(1000);
  }
 
}


ADC_data get_data(ADC_data data) {
    
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(data.channel);
    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH
      
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
    data.tempRaw = SPI.transfer(0) & 0x0F;
    data.tempRaw = data.tempRaw << 8;
    data.tempRaw += SPI.transfer(0);
    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH

    // Reset the peak detector. PH3 (the 4th bit of Port H) is the CH3 reset signal.
    PORTH = PORTH | PK_RST3; // Toggle PK_RST3 HIGH
    PORTH = PORTH & ~PK_RST3; // Toggle PK_RST3 LOW
    return data;
    
}


void send_data(uint8_t channel, unsigned long timeMs, uint16_t peak, uint16_t tempRaw) {
 
    PORTF = (channel & 0xFF);              //1st byte
    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state

    PORTF = (timeMs  & 0xFF);              //2nd byte        
    PORTH = PORTH & ~FIFO_WR; 
    PORTH = PORTH |  FIFO_WR; 
  
    PORTF = (timeMs  & 0xFF00)>>8;         //3rd byte        
    PORTH = PORTH & ~FIFO_WR;
    PORTH = PORTH |  FIFO_WR;

    PORTF = (timeMs  & 0xFF0000)>>16;      //4th byte    
    PORTH = PORTH & ~FIFO_WR;
    PORTH = PORTH |  FIFO_WR;
    
    PORTF = (timeMs  & 0xFF000000)>>24;    //5th byte       
    PORTH = PORTH & ~FIFO_WR;
    PORTH = PORTH |  FIFO_WR;
    
    PORTF = (peak & 0xFF);                 //6th byte    
    PORTH = PORTH & ~FIFO_WR;
    PORTH = PORTH |  FIFO_WR;
    
    PORTF = (peak & 0xFF00)>>8;            //7th byte        
    PORTH = PORTH & ~FIFO_WR;
    PORTH = PORTH |  FIFO_WR;
    
    PORTF = (tempRaw & 0xFF);              //8th byte    
    PORTH = PORTH & ~FIFO_WR;
    PORTH = PORTH |  FIFO_WR;
    
    PORTF = (tempRaw & 0xFF00)>>8;         //9th byte    
    PORTH = PORTH & ~FIFO_WR;
    PORTH = PORTH |  FIFO_WR;
    
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

void print_debug(ADC_data data, char* channel_char, unsigned long timeMs){
  uint8_t  channel = data.channel;
  uint16_t peak_val = data.peak_val;
  uint16_t tempRaw = data.tempRaw;
  
    Serial.print(channel_char);    Serial.print(',');
    Serial.print(timeMs); Serial.print(',');
    Serial.println(tempRaw); Serial.println(" ");

//    Serial.println((channel & 0xFF),            BIN); // [1st byte]
//    Serial.println((timeMs  & 0xFF),            BIN); // [2nd byte]
//    Serial.println((timeMs  & 0xFF00)>>8,       BIN); // [3rd byte]
//    Serial.println((timeMs  & 0xFF0000)>>16,    BIN); // [4th byte]
//    Serial.println((timeMs  & 0xFF000000)>>24,  BIN); // [5th byte]
//    Serial.println((peak_val & 0xFF),            BIN); // [6th byte]
//    Serial.println((peak_val & 0xFF00)>>8,       BIN); // [7th byte]
//    Serial.println((tempRaw & 0xFF),            BIN); // [8th byte]
//    Serial.println((tempRaw & 0xFF00)>>8,       BIN); // [9th byte]
//    Serial.println("---------------------------------------------");  
  
}
