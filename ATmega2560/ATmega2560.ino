#include <SPI.h>

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

// Timestamping from GPS pulse-per-second (PPS) and real-time clock (RTC)
unsigned long timeMs = 0; // Time in milliseconds
//etc.
//etc.
//etc.

// Front end A/D converter
//const int ADCchipSelect = 9; // Chip select for A/D converter
uint16_t tempRaw = 0; // A/D converter's internal temp sensor
uint8_t  channel = 0; // Channel no. [1-4]
volatile bool newEventCH1 = false; // New event flag for channel 1
volatile bool newEventCH2 = false; // New event flag for channel 2
volatile bool newEventCH3 = false; // New event flag for channel 3
volatile bool newEventCH4 = false; // New event flag for channel 4
uint16_t peakCH1 = 0; // Peak value for channel 1
uint16_t peakCH2 = 0; // Peak value for channel 2
uint16_t peakCH3 = 0; // Peak value for channel 3
uint16_t peakCH4 = 0; // Peak value for channel 4


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
void eventISR_CH1() {
  if (!newEventCH1 && !newEventCH2 && !newEventCH3 && !newEventCH4)
    newEventCH1 = true;
  // Ignore new events if another event (on any channel) is currently being processed
  
}
void eventISR_CH2() {
  if (!newEventCH1 && !newEventCH2 && !newEventCH3 && !newEventCH4)
    newEventCH2 = true;
  // Ignore new events if another event (on any channel) is currently being processed
  
}
void eventISR_CH3() {
  if (!newEventCH1 && !newEventCH2 && !newEventCH3 && !newEventCH4)
    newEventCH3 = true;
  // Ignore new events if another event (on any channel) is currently being processed
  
}
void eventISR_CH4() {
  if (!newEventCH1 && !newEventCH2 && !newEventCH3 && !newEventCH4)
    newEventCH4 = true;
  // Ignore new events if another event (on any channel) is currently being processed
  
}


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
  //PORTH = PORTH & ~FIFO_RST; // Toggle FIFO_RST pin from HIGH to LOW
  //delayMicroseconds(5);
  //PORTH = PORTH |  FIFO_RST; // Toggle FIFO_RST pin from LOW to HIGH
  
  
  // Open serial port
  Serial.begin(115200); Serial.flush();
  
  // Initialize and configure SPI bus for A/D communications
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2);   // 8 MHz SPI clock
  SPI.setBitOrder(MSBFIRST);             // Most-significant bit first
  SPI.setDataMode(SPI_MODE0);            // Clock polarity = 0, clock phase = 0

  delay(100);

  // Write A/D configuration register to enable internal Vref and temperature sensor
  digitalWrite(9, LOW);
  Serial.println("Write A/D configuration register");
  delay(10000);
  SPI.transfer(CONFIG_ADDR << 1); // Must shift address 1 bit left for write bit
  SPI.transfer(ADC_CONFIG);
  digitalWrite(9, HIGH);
  
  PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
  delayMicroseconds(5);
  SPI.transfer(CONFIG_ADDR << 1); // Must shift address 1 bit left for write bit
  SPI.transfer(ADC_CONFIG);
  PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH
  
  delay(100);

  // Configure interrupts for all four threshhold discriminators and the FIFO FF
//  attachInterrupt(DISCRIMINATOR1, eventISR_CH1, RISING);
//  attachInterrupt(DISCRIMINATOR2, eventISR_CH2, RISING);
//  attachInterrupt(DISCRIMINATOR3, eventISR_CH3, RISING);
//  attachInterrupt(DISCRIMINATOR4, eventISR_CH4, RISING);
  //attachInterrupt(FIFO_FF, FIFO_FF_ISR, FALLING);


  // Print data header
  Serial.print("channel"); Serial.print(',');
  Serial.print("timeMs");  Serial.print(',');
  Serial.print("peak");    Serial.print(',');
  Serial.println("temperature");
   
  delay(100);
  Serial.println("=============================");
  
}

void loop() {
  
  if (FIFO_full_flag) {
     FIFO_full_flag = false;
     // if the FF flag is set in the beginning of the loop (i.e. at boot or after a reset), unset it
  
  }
  
  
  if (newEventCH1) {
    
    timeMs = millis();  // Get timestamp in milliseconds
    channel = 1;        // Assign channel number

    Serial.println("Entering newEventCH1()!");

    // Read channel 1
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(READ_CH1);
    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH
        
    // Read temp sensor
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(READ_TEMP);
    PORTH = PORTH | ADC_CS; // Toggle ADC_CS HIGH

    // Get channel 1 data
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    peakCH1 = SPI.transfer(0) & 0x0F;
    peakCH1 = peakCH1 << 8;
    peakCH1 += SPI.transfer(0);
    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH
  
    // Get temp sensor data
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    tempRaw = SPI.transfer(0) & 0x0F;
    tempRaw = tempRaw << 8;
    tempRaw += SPI.transfer(0);
    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH

    // Reset the peak detector. PH5 (the 6th bit of Port H) is the CH1 reset signal.
    PORTH = PORTH | PK_RST1; // Toggle PK_RST1 HIGH
    delayMicroseconds(5);
    PORTH = PORTH & ~PK_RST1; // Toggle PK_RST1 LOW
    
    // Debugging
    Serial.print("1");    Serial.print(',');
    Serial.print(timeMs); Serial.print(',');
    Serial.println(peakCH1); Serial.println(" ");

//    // Send detector data (byte-by-byte) to the FIFO memory chip
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (channel & 0xFF); // Present byte 1 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF); // Present byte 2 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF00)>>8; // Present byte 3 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF0000)>>16; // Present byte 4 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF000000)>>24; // Present byte 5 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (peakCH1 & 0xFF); // Present byte 6 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (peakCH1 & 0xFF00)>>8; // Present byte 7 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (tempRaw & 0xFF); // Present byte 8 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (tempRaw & 0xFF00)>>8; // Present byte 9 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state

//    Serial.println((channel & 0xFF),            BIN); // [1st byte]
//    Serial.println((timeMs  & 0xFF),            BIN); // [2nd byte]
//    Serial.println((timeMs  & 0xFF00)>>8,       BIN); // [3rd byte]
//    Serial.println((timeMs  & 0xFF0000)>>16,    BIN); // [4th byte]
//    Serial.println((timeMs  & 0xFF000000)>>24,  BIN); // [5th byte]
//    Serial.println((peakCH1 & 0xFF),            BIN); // [6th byte]
//    Serial.println((peakCH1 & 0xFF00)>>8,       BIN); // [7th byte]
//    Serial.println((tempRaw & 0xFF),            BIN); // [8th byte]
//    Serial.println((tempRaw & 0xFF00)>>8,       BIN); // [9th byte]
//    Serial.println("---------------------------------------------");
    
    // Reset peak value and interrupt flag for CH1
    peakCH1 = 0; newEventCH1 = false;
    delay(1000);
  }


  if (newEventCH2) {
    
    timeMs = millis();  // Get timestamp in milliseconds
    channel = 2;        // Assign channel number

    Serial.println("Entering newEventCH2()!");

    // Read channel 2
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(READ_CH2);
    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH
        
    // Read temp sensor
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(READ_TEMP);
    PORTH = PORTH | ADC_CS; // Toggle ADC_CS HIGH

    // Get channel 2 data
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    peakCH2 = SPI.transfer(0) & 0x0F;
    peakCH2 = peakCH2 << 8;
    peakCH2 += SPI.transfer(0);
    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH
  
    // Get temp sensor data
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    tempRaw = SPI.transfer(0) & 0x0F;
    tempRaw = tempRaw << 8;
    tempRaw += SPI.transfer(0);
    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH

    // Reset the peak detector. PH4 (the 5th bit of Port H) is the CH2 reset signal.
    PORTH = PORTH | PK_RST2; // Toggle PK_RST2 HIGH
    delayMicroseconds(5);
    PORTH = PORTH & ~PK_RST2; // Toggle PK_RST2 LOW
    
    // Debugging
    Serial.print("2");    Serial.print(',');
    Serial.print(timeMs); Serial.print(',');
    Serial.println(peakCH2); Serial.println(" ");

//    // Send detector data (byte-by-byte) to the FIFO memory chip
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (channel & 0xFF); // Present byte 1 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF); // Present byte 2 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF00)>>8; // Present byte 3 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF0000)>>16; // Present byte 4 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF000000)>>24; // Present byte 5 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (peakCH2 & 0xFF); // Present byte 6 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (peakCH2 & 0xFF00)>>8; // Present byte 7 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (tempRaw & 0xFF); // Present byte 8 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (tempRaw & 0xFF00)>>8; // Present byte 9 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state

//    Serial.println((channel & 0xFF),            BIN); // [1st byte]
//    Serial.println((timeMs  & 0xFF),            BIN); // [2nd byte]
//    Serial.println((timeMs  & 0xFF00)>>8,       BIN); // [3rd byte]
//    Serial.println((timeMs  & 0xFF0000)>>16,    BIN); // [4th byte]
//    Serial.println((timeMs  & 0xFF000000)>>24,  BIN); // [5th byte]
//    Serial.println((peakCH2 & 0xFF),            BIN); // [6th byte]
//    Serial.println((peakCH2 & 0xFF00)>>8,       BIN); // [7th byte]
//    Serial.println((tempRaw & 0xFF),            BIN); // [8th byte]
//    Serial.println((tempRaw & 0xFF00)>>8,       BIN); // [9th byte]
//    Serial.println("---------------------------------------------");
    
    // Reset peak value and interrupt flag for CH2
    peakCH2 = 0; newEventCH2 = false;
    delay(1000);
  }


  if (newEventCH3) {
    
    timeMs = millis();  // Get timestamp in milliseconds
    channel = 3;        // Assign channel number

    Serial.println("Entering newEventCH3()!");

    // Read channel 3
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(READ_CH3);
    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH
        
    // Read temp sensor
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(READ_TEMP);
    PORTH = PORTH | ADC_CS; // Toggle ADC_CS HIGH

    // Get channel 3 data
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    peakCH3 = SPI.transfer(0) & 0x0F;
    peakCH3 = peakCH3 << 8;
    peakCH3 += SPI.transfer(0);
    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH
  
    // Get temp sensor data
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    tempRaw = SPI.transfer(0) & 0x0F;
    tempRaw = tempRaw << 8;
    tempRaw += SPI.transfer(0);
    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH

    // Reset the peak detector. PH3 (the 4th bit of Port H) is the CH3 reset signal.
    PORTH = PORTH | PK_RST3; // Toggle PK_RST3 HIGH
    delayMicroseconds(5);
    PORTH = PORTH & ~PK_RST3; // Toggle PK_RST3 LOW
    
    // Debugging
    Serial.print("3");    Serial.print(',');
    Serial.print(timeMs); Serial.print(',');
    Serial.println(peakCH3); Serial.println(" ");

//    // Send detector data (byte-by-byte) to the FIFO memory chip
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (channel & 0xFF); // Present byte 1 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF); // Present byte 2 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF00)>>8; // Present byte 3 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF0000)>>16; // Present byte 4 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF000000)>>24; // Present byte 5 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (peakCH3 & 0xFF); // Present byte 6 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (peakCH3 & 0xFF00)>>8; // Present byte 7 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (tempRaw & 0xFF); // Present byte 8 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (tempRaw & 0xFF00)>>8; // Present byte 9 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state

//    Serial.println((channel & 0xFF),            BIN); // [1st byte]
//    Serial.println((timeMs  & 0xFF),            BIN); // [2nd byte]
//    Serial.println((timeMs  & 0xFF00)>>8,       BIN); // [3rd byte]
//    Serial.println((timeMs  & 0xFF0000)>>16,    BIN); // [4th byte]
//    Serial.println((timeMs  & 0xFF000000)>>24,  BIN); // [5th byte]
//    Serial.println((peakCH3 & 0xFF),            BIN); // [6th byte]
//    Serial.println((peakCH3 & 0xFF00)>>8,       BIN); // [7th byte]
//    Serial.println((tempRaw & 0xFF),            BIN); // [8th byte]
//    Serial.println((tempRaw & 0xFF00)>>8,       BIN); // [9th byte]
//    Serial.println("---------------------------------------------");
    
    // Reset peak value and interrupt flag for CH3
    peakCH3 = 0; newEventCH3 = false;
    delay(1000);
  }

  if (newEventCH4) {
    
    timeMs = millis();  // Get timestamp in milliseconds
    channel = 4;        // Assign channel number

    Serial.println("Entering newEventCH4()!");

    // Read channel 4
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(READ_CH4);
    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH
        
    // Read temp sensor
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(READ_TEMP);
    PORTH = PORTH | ADC_CS; // Toggle ADC_CS HIGH

    // Get channel 4 data
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    peakCH4 = SPI.transfer(0) & 0x0F;
    peakCH4 = peakCH4 << 8;
    peakCH4 += SPI.transfer(0);
    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH
  
    // Get temp sensor data
    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
    tempRaw = SPI.transfer(0) & 0x0F;
    tempRaw = tempRaw << 8;
    tempRaw += SPI.transfer(0);
    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH

    // Reset the peak detector. PH2 (the 3rd bit of Port H) is the CH4 reset signal.
    PORTH = PORTH | PK_RST4; // Toggle PK_RST2 HIGH
    delayMicroseconds(5);
    PORTH = PORTH & ~PK_RST4; // Toggle PK_RST2 LOW
    
    // Debugging
    Serial.print("4");    Serial.print(',');
    Serial.print(timeMs); Serial.print(',');
    Serial.println(peakCH4); Serial.println(" ");

//    // Send detector data (byte-by-byte) to the FIFO memory chip
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (channel & 0xFF); // Present byte 1 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF); // Present byte 2 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF00)>>8; // Present byte 3 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF0000)>>16; // Present byte 4 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF000000)>>24; // Present byte 5 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (peakCH4 & 0xFF); // Present byte 6 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (peakCH4 & 0xFF00)>>8; // Present byte 7 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (tempRaw & 0xFF); // Present byte 8 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (tempRaw & 0xFF00)>>8; // Present byte 9 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state

//    Serial.println((channel & 0xFF),            BIN); // [1st byte]
//    Serial.println((timeMs  & 0xFF),            BIN); // [2nd byte]
//    Serial.println((timeMs  & 0xFF00)>>8,       BIN); // [3rd byte]
//    Serial.println((timeMs  & 0xFF0000)>>16,    BIN); // [4th byte]
//    Serial.println((timeMs  & 0xFF000000)>>24,  BIN); // [5th byte]
//    Serial.println((peakCH4 & 0xFF),            BIN); // [6th byte]
//    Serial.println((peakCH4 & 0xFF00)>>8,       BIN); // [7th byte]
//    Serial.println((tempRaw & 0xFF),            BIN); // [8th byte]
//    Serial.println((tempRaw & 0xFF00)>>8,       BIN); // [9th byte]
//    Serial.println("---------------------------------------------");
    
    // Reset peak value and interrupt flag for CH4
    peakCH4 = 0; newEventCH4 = false;
    delay(1000);
  }
 
  
}
