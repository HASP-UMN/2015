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

// Bitmasks for: peak detector resets, FIFO reset, and FIFO write enable. PORTH on the ATmega2560 (outputs).
#define ADC_CS    B01000000 // PH6: A/D converter chip select (active LOW)
#define PK_RST1   B00100000 // PH5: Channel 1 peak detector reset (active HIGH)
#define PK_RST2   B00010000 // PH4: Channel 2 peak detector reset (active HIGH)
#define PK_RST3   B00001000 // PH3: Channel 3 peak detector reset (active HIGH)
#define PK_RST4   B00000100 // PH2: Channel 4 peak detector reset (active HIGH)
#define FIFO_RST  B00000010 // PH1: FIFO Resetsets both internal R/W pointers to the 1st location. (active LOW)
#define FIFO_WR   B00000001 // PH0: FIFO Write Enable initiates a read cycle on its falling edge. (active LOW)

// Peak threshhold discriminators. INT[7-4] on the ATmega2560 (inputs).
#define DISCRIMINATOR1 9 // INT7: Channel 1 discrimintor.
#define DISCRIMINATOR2 8 // INT6: Channel 2 discrimintor.
#define DISCRIMINATOR3 7 // INT5: Channel 3 discrimintor.
#define DISCRIMINATOR4 6 // INT4: Channel 4 discrimintor.

// FIFO full flag. Pin 88 on the ATmega2560 (input).
#define FIFO_FF 88 // PCINT17: FF output from the FIFO.
volatile bool FIFO_full_flag = false; // Flag signifying the FIFO is completely full

// Timestamping from GPS pulse-per-second (PPS) and real-time clock (RTC)
unsigned long timeMs = 0; // Time in milliseconds
//etc.
//etc.
//etc.

// Front end A/D converter
//const int ADCchipSelect = 53; // Chip select for A/D converter
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
  // Configure interrupts for all four threshhold discriminators and the FIFO FF
  attachInterrupt(DISCRIMINATOR1, eventISR_CH1, RISING);
  attachInterrupt(DISCRIMINATOR2, eventISR_CH2, RISING);
  attachInterrupt(DISCRIMINATOR3, eventISR_CH3, RISING);
  attachInterrupt(DISCRIMINATOR4, eventISR_CH4, RISING);
  attachInterrupt(FIFO_FF, FIFO_FF_ISR, FALLING);
  
  
  // Set up Port H on the ATmega2560 as an output port. DDRH is the direction register for Port H.
  pinMode(18, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  //DDRH = DDRH | B01111111;
  
  
  // Initialize the digital outputs on Port H to low. PORTH is the register for the state of the outputs.
  digitalWrite(18, HIGH);
  digitalWrite(17, LOW);
  digitalWrite(16, LOW);
  digitalWrite(15, LOW);
  digitalWrite(14, LOW);
  digitalWrite(13, HIGH); 
  digitalWrite(12, HIGH);
  //PORTH = B01000011;
  
  
  // Set up Port F on the ATmega2560 as an output port. DDRF is the direction register for Port F.
  DDRF = DDRF | B11111111;

  // Initialize the digital outputs on Port F to low. PORTF is the register for the state of the outputs.
  PORTF = B00000000;


  // Reset the FIFO.
  //PORTH = PORTH & ~FIFO_RST; // Toggle FIFO_RST pin from HIGH to LOW
  digitalWrite(13, LOW);
  delayMicroseconds(5);
  digitalWrite(13, HIGH);
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
  digitalWrite(18, LOW);
  SPI.transfer(CONFIG_ADDR << 1); // Must shift address 1 bit left for write bit
  SPI.transfer(ADC_CONFIG);
  digitalWrite(18, HIGH);
//  PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
//  delayMicroseconds(5);
//  SPI.transfer(CONFIG_ADDR << 1); // Must shift address 1 bit left for write bit
//  SPI.transfer(ADC_CONFIG);
//  PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH
  Serial.println("[x] Write A/D configuration register");
  
  
  delay(100);


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
  Serial.print("FIFO_full_flag = "); Serial.println(FIFO_full_flag);
  Serial.println("Waiting for interrupts...");
  
//  if (newEventCH1) {
//    timeMs = millis();  // Get timestamp in milliseconds
//
//    // Issue A/D command to read MCA channel 1 peak value
//    digitalWrite(ADCchipSelect, LOW);
//    SPI.transfer(MANUAL_READ_ADDR << 1);
//    SPI.transfer(READ_CH1);
//    digitalWrite(ADCchipSelect, HIGH);
//
//    // Wait one additional frame for data to be ready
//    digitalWrite(ADCchipSelect, LOW);
//    SPI.transfer(0);
//    SPI.transfer(0);
//    digitalWrite(ADCchipSelect, HIGH);
//
//    // Read out peak value for MCA channel 1
//    digitalWrite(ADCchipSelect, LOW);
//    peakCH1 = SPI.transfer(0) & 0x0F;
//    peakCH1 = peakCH1 << 8;
//    peakCH1 += SPI.transfer(0);
//    digitalWrite(ADCchipSelect, HIGH);
//
//    // Reset peak detector
//    digitalWrite(RESET_CH1, HIGH);
//    delayMicroseconds(5);
//    digitalWrite(RESET_CH1, LOW);
//
//    Serial.print("1");    Serial.print(',');
//    Serial.print(timeMs); Serial.print(',');
//    Serial.println(peakCH1);
//
//    peakCH1 = 0;
//    newEventCH1 = false;
//    
//  }


  if (newEventCH2) {
    
    timeMs = millis();  // Get timestamp in milliseconds
    channel = 2;        // Assign channel number

    Serial.println("Entering newEventCH2()!");

    // Read channel 2
    digitalWrite(18, LOW);
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(READ_CH2);
    digitalWrite(18, HIGH);
//    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
//    SPI.transfer(MANUAL_READ_ADDR << 1);
//    SPI.transfer(READ_CH2);
//    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH
        
    // Read temp sensor
    digitalWrite(18, LOW);
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(READ_TEMP);
    digitalWrite(18, HIGH);
//    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
//    SPI.transfer(MANUAL_READ_ADDR << 1);
//    SPI.transfer(READ_TEMP);
//    PORTH = PORTH | ADC_CS; // Toggle ADC_CS HIGH

    // Get channel 2 data
    digitalWrite(18, LOW);
    peakCH2 = SPI.transfer(0) & 0x0F;
    peakCH2 = peakCH2 << 8;
    peakCH2 += SPI.transfer(0);
    digitalWrite(18, HIGH);
//    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
//    peakCH2 = SPI.transfer(0) & 0x0F;
//    peakCH2 = peakCH2 << 8;
//    peakCH2 += SPI.transfer(0);
//    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH
  
    // Get temp sensor data
    digitalWrite(18, LOW);
    tempRaw = SPI.transfer(0) & 0x0F;
    tempRaw = tempRaw << 8;
    tempRaw += SPI.transfer(0);
    digitalWrite(18, HIGH);
//    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
//    tempRaw = SPI.transfer(0) & 0x0F;
//    tempRaw = tempRaw << 8;
//    tempRaw += SPI.transfer(0);
//    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH

    // Reset the peak detector. PH4 (the 5th bit of Port H) is the CH2 reset signal.
    digitalWrite(16, HIGH);
    delayMicroseconds(5);
    digitalWrite(16, LOW);
//    PORTH = PORTH | PK_RST2; // Toggle PK_RST2 HIGH
//    delayMicroseconds(5);
//    PORTH = PORTH & ~PK_RST2; // Toggle PK_RST2 LOW
    
    // Debugging
    Serial.print("2");    Serial.print(',');
    Serial.print(timeMs); Serial.print(',');
    Serial.println(peakCH2); Serial.println(" ");

//    // Send detector data (byte-by-byte) to the FIFO memory chip
//    //PORTF = (channel & 0xFF);              //1st byte
//    //PORTB = PORTB & ~FIFO_WRITE_ENABLE;    //first latch                
//    //PORTB = PORTB | FIFO_WRITE_ENABLE;
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (channel & 0xFF); // Present byte 1 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    //PORTF = (timeMs  & 0xFF);              //2nd byte    
//    //PORTB = PORTB & ~FIFO_WRITE_ENABLE;       
//    //PORTB = PORTB | FIFO_WRITE_ENABLE;
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF); // Present byte 2 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//   
//    //PORTF = (timeMs  & 0xFF00)>>8;         //3rd byte    
//    //PORTB = PORTB & ~FIFO_WRITE_ENABLE;        
//    //PORTB = PORTB | FIFO_WRITE_ENABLE;
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF00)>>8; // Present byte 3 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    //PORTF = (timeMs  & 0xFF0000)>>16;      //4th byte
//    //PORTB = PORTB & ~FIFO_WRITE_ENABLE;        
//    //PORTB = PORTB | FIFO_WRITE_ENABLE;
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF0000)>>16; // Present byte 4 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//    
//    //PORTF = (timeMs  & 0xFF000000)>>24;    //5th byte    
//    //PORTB = PORTB & ~FIFO_WRITE_ENABLE;         
//    //PORTB = PORTB | FIFO_WRITE_ENABLE;
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF000000)>>24; // Present byte 5 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//    
//    //PORTF = (peakCH2 & 0xFF);              //6th byte
//    //PORTB = PORTB & ~FIFO_WRITE_ENABLE;         
//    //PORTB = PORTB | FIFO_WRITE_ENABLE;
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (peakCH2 & 0xFF); // Present byte 6 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//    
//    //PORTF = (peakCH2 & 0xFF00)>>8;         //7th byte    
//    //PORTB = PORTB & ~FIFO_WRITE_ENABLE;         
//    //PORTB = PORTB | FIFO_WRITE_ENABLE;
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (peakCH2 & 0xFF00)>>8; // Present byte 7 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//       
//    //PORTF = (tempRaw & 0xFF);              //8th byte
//    //PORTB = PORTB & ~FIFO_WRITE_ENABLE;         
//    //PORTB = PORTB | FIFO_WRITE_ENABLE;
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (tempRaw & 0xFF); // Present byte 8 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//        
//    //PORTF = (tempRaw & 0xFF00)>>8;         //9th byte
//    //PORTB = PORTB & ~FIFO_WRITE_ENABLE;         
//    //PORTB = PORTB | FIFO_WRITE_ENABLE;
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (tempRaw & 0xFF00)>>8; // Present byte 9 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
    
    // Debugging
    Serial.println((channel & 0xFF),            BIN); // [1st byte]
    Serial.println((timeMs  & 0xFF),            BIN); // [2nd byte]
    Serial.println((timeMs  & 0xFF00)>>8,       BIN); // [3rd byte]
    Serial.println((timeMs  & 0xFF0000)>>16,    BIN); // [4th byte]
    Serial.println((timeMs  & 0xFF000000)>>24,  BIN); // [5th byte]
    Serial.println((peakCH2 & 0xFF),            BIN); // [6th byte]
    Serial.println((peakCH2 & 0xFF00)>>8,       BIN); // [7th byte]
    Serial.println((tempRaw & 0xFF),            BIN); // [8th byte]
    Serial.println((tempRaw & 0xFF00)>>8,       BIN); // [9th byte]
    Serial.println("---------------------------------------------");
    
    // Reset peak value and interrupt flag for CH2
    peakCH2 = 0; newEventCH2 = false;
    delay(1000);
  }

  if (newEventCH3) {
    
    timeMs = millis();  // Get timestamp in milliseconds
    channel = 3;        // Assign channel number

    Serial.println("Entering newEventCH3()!");

    // Read channel 3
    digitalWrite(18, LOW);
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(READ_CH3);
    digitalWrite(18, HIGH);
//    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
//    SPI.transfer(MANUAL_READ_ADDR << 1);
//    SPI.transfer(READ_CH3);
//    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH
      
    // Read temp sensor
    digitalWrite(18, LOW);
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(READ_TEMP);
    digitalWrite(18, HIGH);
//    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
//    SPI.transfer(MANUAL_READ_ADDR << 1);
//    SPI.transfer(READ_TEMP);
//    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH

    // Get channel 3 data
    digitalWrite(18, LOW);
    peakCH3 = SPI.transfer(0) & 0x0F;
    peakCH3 = peakCH3 << 8;
    peakCH3 += SPI.transfer(0);
    digitalWrite(18, HIGH);
//    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
//    peakCH3 = SPI.transfer(0) & 0x0F;
//    peakCH3 = peakCH3 << 8;
//    peakCH3 += SPI.transfer(0);
//    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH
    
    // Get temp sensor data
    digitalWrite(18, LOW);
    tempRaw = SPI.transfer(0) & 0x0F;
    tempRaw = tempRaw << 8;
    tempRaw += SPI.transfer(0);
    digitalWrite(18, HIGH);
//    PORTH = PORTH & ~ADC_CS; // Toggle ADC_CS LOW
//    tempRaw = SPI.transfer(0) & 0x0F;
//    tempRaw = tempRaw << 8;
//    tempRaw += SPI.transfer(0);
//    PORTH = PORTH |  ADC_CS; // Toggle ADC_CS HIGH

    // Reset the peak detector. PH3 (the 4th bit of Port H) is the CH3 reset signal.
    digitalWrite(15, HIGH);
    delayMicroseconds(5);
    digitalWrite(15, LOW);
//    PORTH = PORTH | PK_RST3; // Toggle PK_RST3 HIGH
//    delayMicroseconds(5);
//    PORTH = PORTH & ~PK_RST3; // Toggle PK_RST3 LOW
    
    // Debugging    
    Serial.print("3");    Serial.print(',');
    Serial.print(timeMs); Serial.print(',');
    Serial.println(peakCH3); Serial.println(" ");

//    // Send detector data (byte-by-byte) to the FIFO memory chip
//    //PORTF = (channel & 0xFF);              //1st byte
//    //PORTB = PORTB & ~FIFO_WRITE_ENABLE;    //first latch                
//    //PORTB = PORTB | FIFO_WRITE_ENABLE;
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (channel & 0xFF); // Present byte 1 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    //PORTF = (timeMs  & 0xFF);              //2nd byte    
//    //PORTB = PORTB & ~FIFO_WRITE_ENABLE;       
//    //PORTB = PORTB | FIFO_WRITE_ENABLE;
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF); // Present byte 2 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//  
//    //PORTF = (timeMs  & 0xFF00)>>8;         //3rd byte    
//    //PORTB = PORTB & ~FIFO_WRITE_ENABLE;        
//    //PORTB = PORTB | FIFO_WRITE_ENABLE;
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF00)>>8; // Present byte 3 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//
//    //PORTF = (timeMs  & 0xFF0000)>>16;      //4th byte
//    //PORTB = PORTB & ~FIFO_WRITE_ENABLE;        
//    //PORTB = PORTB | FIFO_WRITE_ENABLE;
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF0000)>>16; // Present byte 4 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//    
//    //PORTF = (timeMs  & 0xFF000000)>>24;    //5th byte    
//    //PORTB = PORTB & ~FIFO_WRITE_ENABLE;         
//    //PORTB = PORTB | FIFO_WRITE_ENABLE;
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (timeMs  & 0xFF000000)>>24; // Present byte 5 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//    
//    //PORTF = (peakCH2 & 0xFF);              //6th byte
//    //PORTB = PORTB & ~FIFO_WRITE_ENABLE;         
//    //PORTB = PORTB | FIFO_WRITE_ENABLE;
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (peakCH3 & 0xFF); // Present byte 6 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//    
//    //PORTF = (peakCH2 & 0xFF00)>>8;         //7th byte    
//    //PORTB = PORTB & ~FIFO_WRITE_ENABLE;         
//    //PORTB = PORTB | FIFO_WRITE_ENABLE;
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (peakCH3 & 0xFF00)>>8; // Present byte 7 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state    
//    
//    //PORTF = (tempRaw & 0xFF);              //8th byte
//    //PORTB = PORTB & ~FIFO_WRITE_ENABLE;         
//    //PORTB = PORTB | FIFO_WRITE_ENABLE;
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (tempRaw & 0xFF); // Present byte 8 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
//    
//    //PORTF = (tempRaw & 0xFF00)>>8;         //9th byte
//    //PORTB = PORTB & ~FIFO_WRITE_ENABLE;         
//    //PORTB = PORTB | FIFO_WRITE_ENABLE;
//    PORTH = PORTH & ~FIFO_WR; // Assert FIFO_WR to LOW state
//    PORTF = (tempRaw & 0xFF00)>>8; // Present byte 9 to Port F
//    PORTH = PORTH |  FIFO_WR; // Return FIFO_WR to HIGH state
        
    // Debugging
    Serial.println((channel & 0xFF),            BIN); // [1st byte]
    Serial.println((timeMs  & 0xFF),            BIN); // [2nd byte]
    Serial.println((timeMs  & 0xFF00)>>8,       BIN); // [3rd byte]
    Serial.println((timeMs  & 0xFF0000)>>16,    BIN); // [4th byte]
    Serial.println((timeMs  & 0xFF000000)>>24,  BIN); // [5th byte]
    Serial.println((peakCH3 & 0xFF),            BIN); // [6th byte]
    Serial.println((peakCH3 & 0xFF00)>>8,       BIN); // [7th byte]
    Serial.println((tempRaw & 0xFF),            BIN); // [8th byte]
    Serial.println((tempRaw & 0xFF00)>>8,       BIN); // [9th byte]
    Serial.println("---------------------------------------------");
    
    // Reset peak value and interrupt flag for CH3
    peakCH3 = 0; newEventCH3 = false;
    delay(1000);
  }

//  if (newEventCH4) {
//
//    timeMs = millis();  // Get timestamp in milliseconds
//
//    // Issue A/D command to read MCA channel 1 peak value
//    digitalWrite(ADCchipSelect, LOW);
//    SPI.transfer(MANUAL_READ_ADDR << 1);
//    SPI.transfer(READ_CH4);
//    digitalWrite(ADCchipSelect, HIGH);
//
//    // Wait one additional frame for data to be ready
//    digitalWrite(ADCchipSelect, LOW);
//    SPI.transfer(0);
//    SPI.transfer(0);
//    digitalWrite(ADCchipSelect, HIGH);
//
//    // Read out peak value for MCA channel 1
//    digitalWrite(ADCchipSelect, LOW);
//    peakCH4 = SPI.transfer(0) & 0x0F;
//    peakCH4 = peakCH4 << 8;
//    peakCH4 += SPI.transfer(0);
//    digitalWrite(ADCchipSelect, HIGH);
//
//    // Reset peak detector
//    digitalWrite(RESET_CH4, HIGH);
//    delayMicroseconds(5);
//    digitalWrite(RESET_CH4, LOW);
//
//    Serial.print("4");    Serial.print(',');
//    Serial.print(timeMs); Serial.print(',');
//    Serial.println(peakCH4);
//
//    peakCH4 = 0;
//    newEventCH4 = false;
//  }

}
