#include <SPI.h>

// Addresses and commands for A/D operation. See ADS8634 datasheet.
#define ADC_CONFIG 0b00000110    // Enable internal Vref and temperature sensor
#define READ_CH1 0b01001010      // Read A/D channel 2 (MCA ch.1) with 0 to 10V range
#define READ_CH2 0b01101010      // Read A/D channel 3 (MCA ch.2) with 0 to 10V range
#define READ_CH3 0b00001010      // Read A/D channel 0 (MCA ch.3) with 0 to 10V range
#define READ_CH4 0b00101010      // Read A/D channel 1 (MCA ch.4) with 0 to 10V range
#define READ_TEMP 0b00000001     // Read temperature sensor
#define CONFIG_ADDR 0x06         // 7-bit internal control register address
#define MANUAL_READ_ADDR 0x04    // 7-bit address for manual mode read register

// Peak detector reset pins and discriminator interrupts
#define RESET_CH1 4  // Channel 1 peak detector reset from Arduino pin 4
#define RESET_CH2 5  // Channel 2 peak detector reset from Arduino pin 5
#define RESET_CH3 6  // Channel 3 peak detector reset from Arduino pin 6
#define RESET_CH4 7  // Channel 4 peak detector reset from Arduino pin 7
#define IRQ_CH1 0    // Arduino Mega 2560 interrupt 0 (pin 2)
#define IRQ_CH2 1    // Arduino Mega 2560 interrupt 1 (pin 3)
#define IRQ_CH3 4    // Arduino Mega 2560 interrupt 4 (pin 19)
#define IRQ_CH4 5    // Arduino Mega 2560 interrupt 5 (pin 18)

// Global variables
const int ADCchipSelect = 53;        // Chip select for A/D converter
volatile bool newEventCH1 = false;   // New event flag for MCA channel 1
volatile bool newEventCH2 = false;   // New event flag for MCA channel 2
volatile bool newEventCH3 = false;   // New event flag for MCA channel 3
volatile bool newEventCH4 = false;   // New event flag for MCA channel 4
unsigned long timeMs = 0;            // Time in milliseconds
uint16_t peakCH1 = 0;                // Peak value for MCA channel 1
uint16_t peakCH2 = 0;                // Peak value for MCA channel 2
uint16_t peakCH3 = 0;                // Peak value for MCA channel 3
uint16_t peakCH4 = 0;                // Peak value for MCA channel 4


// Interrupt handlers
void eventISR_CH1() {
  
  if(!newEventCH1 && !newEventCH2 && !newEventCH3 && !newEventCH4)
    newEventCH1 = true;  
  // Ignore new events if another event (on any channel) is currently being processed
}

void eventISR_CH2() {
  
  if(!newEventCH1 && !newEventCH2 && !newEventCH3 && !newEventCH4)
    newEventCH2 = true;  
  // Ignore new events if another event (on any channel) is currently being processed
}

void eventISR_CH3() {
  
  if(!newEventCH1 && !newEventCH2 && !newEventCH3 && !newEventCH4)
    newEventCH3 = true;  
  // Ignore new events if another event (on any channel) is currently being processed
}

void eventISR_CH4() {
  
  if(!newEventCH1 && !newEventCH2 && !newEventCH3 && !newEventCH4)
    newEventCH4 = true;  
  // Ignore new events if another event (on any channel) is currently being processed
}


void setup() {
  // Set up digital outputs
  pinMode(ADCchipSelect, OUTPUT);
  pinMode(RESET_CH1, OUTPUT);
  pinMode(RESET_CH2, OUTPUT);
  pinMode(RESET_CH3, OUTPUT);
  pinMode(RESET_CH4, OUTPUT);
  
  digitalWrite(ADCchipSelect, HIGH);
  digitalWrite(RESET_CH1, LOW);
  digitalWrite(RESET_CH2, LOW);
  digitalWrite(RESET_CH3, LOW);
  digitalWrite(RESET_CH4, LOW);
  
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
  Serial.print("channel"); Serial.print(',');
  Serial.print("timeMs");  Serial.print(',');
  Serial.println("peak");
  
  delay(100);
  
  // Configure interrupts
  attachInterrupt(IRQ_CH1, eventISR_CH1, RISING);
  attachInterrupt(IRQ_CH2, eventISR_CH2, RISING);
  attachInterrupt(IRQ_CH3, eventISR_CH3, RISING);
  attachInterrupt(IRQ_CH4, eventISR_CH4, RISING);
  
  Serial.println("INTERRUPTS ARE CONFIGURED");
  
  delay(100);
}

void loop() {
  
  
  Serial.print("CH. 1 FLAG:"); Serial.print(newEventCH1); Serial.print(" ");
  Serial.print("CH. 2 FLAG:"); Serial.print(newEventCH2); Serial.print(" ");
  Serial.print("CH. 3 FLAG:"); Serial.print(newEventCH3); Serial.print(" ");
  Serial.print("CH. 4 FLAG:"); Serial.print(newEventCH4); Serial.println(" ");
  
  if(newEventCH1) {
    
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
  
  if(newEventCH2) {
    
    timeMs = millis();  // Get timestamp in milliseconds
    
    // Issue A/D command to read MCA channel 1 peak value
    digitalWrite(ADCchipSelect, LOW);
    SPI.transfer(MANUAL_READ_ADDR << 1);
    SPI.transfer(READ_CH2);
    digitalWrite(ADCchipSelect, HIGH);
    
    // Wait one additional frame for data to be ready
    digitalWrite(ADCchipSelect, LOW);
    SPI.transfer(0);
    SPI.transfer(0);
    digitalWrite(ADCchipSelect, HIGH);
  
    // Read out peak value for MCA channel 1
    digitalWrite(ADCchipSelect, LOW);
    peakCH2 = SPI.transfer(0) & 0x0F; 
    peakCH2 = peakCH2 << 8;
    peakCH2 += SPI.transfer(0);
    digitalWrite(ADCchipSelect, HIGH);
    
    // Reset peak detector
    digitalWrite(RESET_CH2, HIGH);
    delayMicroseconds(5);
    digitalWrite(RESET_CH2, LOW);
    
    Serial.print("2");    Serial.print(',');
    Serial.print(timeMs); Serial.print(',');
    Serial.println(peakCH2);
    
    peakCH2 = 0;
    newEventCH2 = false;
  }
  
  if(newEventCH3) {
    
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
  
  if(newEventCH4) {
    
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
