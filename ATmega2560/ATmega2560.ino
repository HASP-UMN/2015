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


// Peak detector resets, FIFO reset, and FIFO write enable. PORTH on the ATmega2560 (outputs).
#define ADC_CS    B11111101 // PH6: A/D converter chip select (active LOW)
#define RESET_CH1 B00000100 // PH5: Channel 1 peak detector reset (active HIGH)
#define RESET_CH2 B00001000 // PH4: Channel 2 peak detector reset (active HIGH)
#define RESET_CH3 B00010000 // PH3: Channel 3 peak detector reset (active HIGH)
#define RESET_CH4 B00100000 // PH2: Channel 4 peak detector reset (active HIGH)
#define FIFO_RST  B10111111 // PH1: FIFO Reset, which sets both internal read and write pointers to the first location. (active LOW)
#define FIFO_WR   B01111111 // PH0: FIFO Write Enable, which initiates a read cycle on its falling edge. (active LOW)


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
const int ADCchipSelect = 53; // Chip select for A/D converter
uint16_t tempRaw = 0; // A/D converter's internal temp sensor
uint8_t channel = 0; // Channel no. [1-4]
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
  //pinMode(ADCchipSelect, OUTPUT);
  //pinMode(RESET_CH1, OUTPUT);
  //pinMode(RESET_CH2, OUTPUT);
  //pinMode(RESET_CH3, OUTPUT);
  //pinMode(RESET_CH4, OUTPUT);
  //pinMode(FIFO_RST,  OUTPUT);
  //pinMode(FIFO_WR,   OUTPUT);
  DDRH = DDRH | B11111111;
  
  // Initialize the digital outputs on Port H to low. PORTH is the register for the state of the outputs.
  //digitalWrite(ADCchipSelect, HIGH);
  //digitalWrite(RESET_CH1, LOW);
  //digitalWrite(RESET_CH2, LOW);
  //digitalWrite(RESET_CH3, LOW);
  //digitalWrite(RESET_CH4, LOW);
  //digitalWrite(FIFO_RST,  HIGH); 
  //digitalWrite(FIFO_WR,   HIGH);
  PORTH = B11000010;
  
  
  // Set up Port F on the ATmega2560 as an output port. DDRF is the direction register for Port F.
  DDRF = DDRF | B11111111;
  
  // Initialize the digital outputs on Port F to low. PORTF is the register for the state of the outputs.
  PORTF = B00000000;

  
  PORTH = PORTH & ~FIFO_WR;         
  PORTH = PORTH |  FIFO_WR;
  
  // Open serial port
  Serial.begin(115200);

  // Initialize and configure SPI bus for A/D communications
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2);   // 8 MHz SPI clock
  SPI.setBitOrder(MSBFIRST);             // Most-significant bit first
  SPI.setDataMode(SPI_MODE0);            // Clock polarity = 0, clock phase = 0

  delay(100);

  // Write A/D configuration register to enable internal Vref and temperature sensor
  digitalWrite(ADCchipSelect, LOW);
  SPI.transfer(CONFIG_ADDR << 1);        // Must shift address 1 bit left for write bit
  SPI.transfer(ADC_CONFIG);
  digitalWrite(ADCchipSelect, HIGH);

  delay(100);

  // Print data header
  Serial.flush();
  Serial.print("channel"); Serial.print(',');
  Serial.print("timeMs");  Serial.print(',');
  Serial.print("peak"); Serial.print(','); Serial.println("temperature");
  
  delay(100);

  // Configure interrupts for all four threshhold discriminators and the FIFO FF
  attachInterrupt(DISCRIMINATOR1, eventISR_CH1, RISING);
  attachInterrupt(DISCRIMINATOR2, eventISR_CH2, RISING);
  attachInterrupt(DISCRIMINATOR3, eventISR_CH3, RISING);
  attachInterrupt(DISCRIMINATOR4, eventISR_CH4, RISING);
  attachInterrupt(FIFO_FF, FIFO_FF_ISR, FALLING);
  
  
  delay(100);
  delay(5000);
  Serial.println("=============================");
  
}

void loop() {

  if (FIFO_full_flag) {
     FIFO_full_flag = false;
     // if the FF flag is set in the beginning of the loop (i.e. at boot or after a reset), unset it
  }

  if (newEventCH1) {

    timeMs = millis();  // Get timestamp in milliseconds

    // Issue A/D command to read MCA channel 1 peak value
    digitalWrite(ADCchipSelect, LOW);
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(READ_CH1);
    digitalWrite(ADCchipSelect, HIGH);

    // Wait one additional frame for data to be ready
    digitalWrite(ADCchipSelect, LOW);
    SPI.transfer(0);
    SPI.transfer(0);
    digitalWrite(ADCchipSelect, HIGH);

    // Read out peak value for MCA channel 1
    digitalWrite(ADCchipSelect, LOW);
    peakCH1 = SPI.transfer(0) & 0x0F;
    peakCH1 = peakCH1 << 8;
    peakCH1 += SPI.transfer(0);
    digitalWrite(ADCchipSelect, HIGH);

    // Reset peak detector
    digitalWrite(RESET_CH1, HIGH);
    delayMicroseconds(5);
    digitalWrite(RESET_CH1, LOW);

    Serial.print("1");    Serial.print(',');
    Serial.print(timeMs); Serial.print(',');
    Serial.println(peakCH1);

    peakCH1 = 0;
    newEventCH1 = false;
  }

  if (newEventCH2) {

    timeMs = millis();  // Get timestamp in milliseconds
    channel = 2;        // Assign channel number

    // Read channel 2
    digitalWrite(ADCchipSelect, LOW);
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(READ_CH2);
    digitalWrite(ADCchipSelect, HIGH);
    
    // Read temp sensor
    digitalWrite(ADCchipSelect, LOW);
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(READ_TEMP);
    digitalWrite(ADCchipSelect, HIGH);

    // Get channel 2 data
    digitalWrite(ADCchipSelect, LOW);
    peakCH2 = SPI.transfer(0) & 0x0F;
    peakCH2 = peakCH2 << 8;
    peakCH2 += SPI.transfer(0);
    digitalWrite(ADCchipSelect, HIGH);
    
    // Get temp sensor data
    digitalWrite(ADCchipSelect, LOW);
    tempRaw = SPI.transfer(0) & 0x0F;
    tempRaw = tempRaw << 8;
    tempRaw += SPI.transfer(0);
    digitalWrite(ADCchipSelect, HIGH);

    // Reset peak detector
    digitalWrite(RESET_CH2, HIGH);
    delayMicroseconds(5);
    digitalWrite(RESET_CH2, LOW);

    Serial.print("2");    Serial.print(',');
    Serial.print(timeMs); Serial.print(',');
    Serial.println(peakCH2); Serial.println(" ");

    // Begin PORT7 write
//    digitalWrite(22, LOW); // hold read_enable low
//    digitalWrite(FIFO_WRITE_ENABLE, LOW);
//    PORTF = (channel & 0xFF);           digitalWrite(FIFO_WRITE_ENABLE, HIGH);    digitalWrite(22, HIGH);     digitalWrite(22, LOW); digitalWrite(FIFO_WRITE_ENABLE, LOW); PORTF = B00000000; // [1st byte]
//    PORTF = (timeMs  & 0xFF);           digitalWrite(FIFO_WRITE_ENABLE, HIGH);    digitalWrite(22, HIGH);     digitalWrite(22, LOW); digitalWrite(FIFO_WRITE_ENABLE, LOW); PORTF = B00000000; // [2nd byte]
//    PORTF = (timeMs  & 0xFF00)>>8;      digitalWrite(FIFO_WRITE_ENABLE, HIGH);    digitalWrite(22, HIGH);     digitalWrite(22, LOW); digitalWrite(FIFO_WRITE_ENABLE, LOW); PORTF = B00000000; // [3rd byte]
//    PORTF = (timeMs  & 0xFF0000)>>16;   digitalWrite(FIFO_WRITE_ENABLE, HIGH);    digitalWrite(22, HIGH);     digitalWrite(22, LOW); digitalWrite(FIFO_WRITE_ENABLE, LOW); PORTF = B00000000; // [4th byte]
//    PORTF = (timeMs  & 0xFF000000)>>24; digitalWrite(FIFO_WRITE_ENABLE, HIGH);    digitalWrite(22, HIGH);     digitalWrite(22, LOW); digitalWrite(FIFO_WRITE_ENABLE, LOW); PORTF = B00000000; // [5th byte]
//    PORTF = (peakCH2 & 0xFF);           digitalWrite(FIFO_WRITE_ENABLE, HIGH);    digitalWrite(22, HIGH);     digitalWrite(22, LOW); digitalWrite(FIFO_WRITE_ENABLE, LOW); PORTF = B00000000; // [6th byte]
//    PORTF = (peakCH2 & 0xFF00)>>8;      digitalWrite(FIFO_WRITE_ENABLE, HIGH);    digitalWrite(22, HIGH);     digitalWrite(22, LOW); digitalWrite(FIFO_WRITE_ENABLE, LOW); PORTF = B00000000; // [7th byte]
//    PORTF = (tempRaw & 0xFF);           digitalWrite(FIFO_WRITE_ENABLE, HIGH);    digitalWrite(22, HIGH);     digitalWrite(22, LOW); digitalWrite(FIFO_WRITE_ENABLE, LOW); PORTF = B00000000; // [8th byte]
//    PORTF = (tempRaw & 0xFF00)>>8;      digitalWrite(FIFO_WRITE_ENABLE, HIGH);    digitalWrite(22, HIGH);     digitalWrite(22, LOW); digitalWrite(FIFO_WRITE_ENABLE, LOW); PORTF = B00000000; // [9th byte]
//    digitalWrite(22, HIGH);
//    digitalWrite(FIFO_WRITE_ENABLE, HIGH);

    PORTF = 0;
    PORTB = PORTB | FIFO_WRITE_ENABLE;
    
    PORTF = (channel & 0xFF);              //1st byte
    PORTB = PORTB & ~FIFO_WRITE_ENABLE;    //first latch                
    PORTB = PORTB | FIFO_WRITE_ENABLE;

    PORTF = (timeMs  & 0xFF);              //2nd byte    
    PORTB = PORTB & ~FIFO_WRITE_ENABLE;       
    PORTB = PORTB | FIFO_WRITE_ENABLE;
    
    PORTF = (timeMs  & 0xFF00)>>8;         //3rd byte    
    PORTB = PORTB & ~FIFO_WRITE_ENABLE;        
    PORTB = PORTB | FIFO_WRITE_ENABLE;
    
    PORTF = (timeMs  & 0xFF0000)>>16;      //4th byte
    PORTB = PORTB & ~FIFO_WRITE_ENABLE;        
    PORTB = PORTB | FIFO_WRITE_ENABLE;
    
    PORTF = (timeMs  & 0xFF000000)>>24;    //5th byte    
    PORTB = PORTB & ~FIFO_WRITE_ENABLE;         
    PORTB = PORTB | FIFO_WRITE_ENABLE;
        
    PORTF = (peakCH2 & 0xFF);              //6th byte
    PORTB = PORTB & ~FIFO_WRITE_ENABLE;         
    PORTB = PORTB | FIFO_WRITE_ENABLE;
    
    PORTF = (peakCH2 & 0xFF00)>>8;         //7th byte    
    PORTB = PORTB & ~FIFO_WRITE_ENABLE;         
    PORTB = PORTB | FIFO_WRITE_ENABLE;
    
    PORTF = (tempRaw & 0xFF);              //8th byte
    PORTB = PORTB & ~FIFO_WRITE_ENABLE;         
    PORTB = PORTB | FIFO_WRITE_ENABLE;

    PORTF = (tempRaw & 0xFF00)>>8;         //9th byte
    PORTB = PORTB & ~FIFO_WRITE_ENABLE;         
    PORTB = PORTB | FIFO_WRITE_ENABLE;
       
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

    // Issue A/D command to read MCA channel 1 peak value
    digitalWrite(ADCchipSelect, LOW);
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(READ_CH3);
    digitalWrite(ADCchipSelect, HIGH);

    // Wait one additional frame for data to be ready
    digitalWrite(ADCchipSelect, LOW);
    SPI.transfer(0);
    SPI.transfer(0);
    digitalWrite(ADCchipSelect, HIGH);

    // Read out peak value for MCA channel 1
    digitalWrite(ADCchipSelect, LOW);
    peakCH3 = SPI.transfer(0) & 0x0F;
    peakCH3 = peakCH3 << 8;
    peakCH3 += SPI.transfer(0);
    digitalWrite(ADCchipSelect, HIGH);

    // Reset peak detector
    digitalWrite(RESET_CH3, HIGH);
    delayMicroseconds(5);
    digitalWrite(RESET_CH3, LOW);

    Serial.print("3");    Serial.print(',');
    Serial.print(timeMs); Serial.print(',');
    Serial.println(peakCH3);

    peakCH3 = 0;
    newEventCH3 = false;
  }

  if (newEventCH4) {

    timeMs = millis();  // Get timestamp in milliseconds

    // Issue A/D command to read MCA channel 1 peak value
    digitalWrite(ADCchipSelect, LOW);
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(READ_CH4);
    digitalWrite(ADCchipSelect, HIGH);

    // Wait one additional frame for data to be ready
    digitalWrite(ADCchipSelect, LOW);
    SPI.transfer(0);
    SPI.transfer(0);
    digitalWrite(ADCchipSelect, HIGH);

    // Read out peak value for MCA channel 1
    digitalWrite(ADCchipSelect, LOW);
    peakCH4 = SPI.transfer(0) & 0x0F;
    peakCH4 = peakCH4 << 8;
    peakCH4 += SPI.transfer(0);
    digitalWrite(ADCchipSelect, HIGH);

    // Reset peak detector
    digitalWrite(RESET_CH4, HIGH);
    delayMicroseconds(5);
    digitalWrite(RESET_CH4, LOW);

    Serial.print("4");    Serial.print(',');
    Serial.print(timeMs); Serial.print(',');
    Serial.println(peakCH4);

    peakCH4 = 0;
    newEventCH4 = false;
  }

}
