#include <SPI.h>

#define ADC_CONFIG 0b00000110    // Enable internal Vref and temp. sensor
#define RANGE_CONFIG 0b01010000  // Set range from 0 to 10V
#define READ_CH0 0b00001010      // Read channel 0 with 0 to 10V range
#define READ_CH1 0b00101010      // Read channel 1 with 0 to 10V range
#define READ_CH2 0b01001010      // Read channel 2 with 0 to 10V range
#define READ_CH3 0b01101010      // Read channel 3 with 0 to 10V range
#define READ_TEMP 0b00000001     // Read temperature sensor
#define CONFIG_ADDR 0x06         // 7-bit internal control register address
#define CH0_RANGE_ADDR 0x10      // 7-bit address for channel 0 range register
#define CH1_RANGE_ADDR 0x11      // 7-bit address for channel 1 range register
#define CH2_RANGE_ADDR 0x12      // 7-bit address for channel 2 range register
#define CH3_RANGE_ADDR 0x13      // 7-bit address for channel 3 range register
#define MANUAL_READ_ADDR 0x04    // 7-bit address for manual mode read register

const int chipSelect = 53;

void setup() {
  
  pinMode(chipSelect, OUTPUT);
  digitalWrite(chipSelect, HIGH);
  
  Serial.begin(115200);
  
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV2);    // 8 MHz SPI clock
  SPI.setBitOrder(MSBFIRST);              // Most-significant bit first
  SPI.setDataMode(SPI_MODE0);             // Clock polarity = 0, clock phase = 0
  
  delay(100);
  
  // Write configuration register to enable internal Vref and temperature sensor
  digitalWrite(chipSelect, LOW);
  SPI.transfer(CONFIG_ADDR << 1); // Must shift address 1 bit left for write bit
  SPI.transfer(ADC_CONFIG);
  digitalWrite(chipSelect, HIGH);
  
  delay(10);
  
  // Configure range on all ranges to be 0 to 10V
//  digitalWrite(chipSelect, LOW);
//  SPI.transfer(CH0_RANGE_ADDR << 1);
//  SPI.transfer(RANGE_CONFIG);
//  SPI.transfer(CH1_RANGE_ADDR << 1);
//  SPI.transfer(RANGE_CONFIG);
//  SPI.transfer(CH2_RANGE_ADDR << 1);
//  SPI.transfer(RANGE_CONFIG);
//  SPI.transfer(CH3_RANGE_ADDR << 1);
//  SPI.transfer(RANGE_CONFIG);
//  digitalWrite(chipSelect, HIGH);
  
  delay(100);
  
  Serial.print("Ch 0"); Serial.print('\t');
  Serial.print("Ch 1"); Serial.print('\t');
  Serial.print("Ch 2"); Serial.print('\t');
  Serial.print("Ch 3"); Serial.print('\t');
  Serial.println("Temp");
  
  delay(100);
}

void loop() {
  int tempRaw = 0;
  float tempC = 0;
  int ch0 = 0;
  int ch1 = 0;
  int ch2 = 0;
  int ch3 = 0;
  
  // Read channel 0
  digitalWrite(chipSelect, LOW);
  SPI.transfer(MANUAL_READ_ADDR << 1);
  SPI.transfer(READ_CH0);
  digitalWrite(chipSelect, HIGH); delay(5);
  
  // Read channel 1
  digitalWrite(chipSelect, LOW);
  SPI.transfer(MANUAL_READ_ADDR << 1);
  SPI.transfer(READ_CH1);
  digitalWrite(chipSelect, HIGH); delay(5);
  
  // Read channel 2 and get channel 0 data
  digitalWrite(chipSelect, LOW);
  ch0 = SPI.transfer(MANUAL_READ_ADDR << 1) & 0x0F; 
  ch0 = ch0 << 8;
  ch0 += SPI.transfer(READ_CH2);
  digitalWrite(chipSelect, HIGH); delay(5);
  
  // Read channel 3 and get channel 1 data
  digitalWrite(chipSelect, LOW);
  digitalWrite(chipSelect, LOW);
  ch1 = SPI.transfer(MANUAL_READ_ADDR << 1) & 0x0F; 
  ch1 = ch1 << 8;
  ch1 += SPI.transfer(READ_CH3);
  digitalWrite(chipSelect, HIGH); delay(5);
  
  // Read temp sensor and get channel 2 data
  digitalWrite(chipSelect, LOW);
  ch2 = SPI.transfer(MANUAL_READ_ADDR << 1) & 0x0F;
  ch2 = ch2 << 8;
  ch2 += SPI.transfer(READ_TEMP);
  digitalWrite(chipSelect, HIGH); delay(5);
  
  // Get channel 3 data
  digitalWrite(chipSelect, LOW);
  ch3 = SPI.transfer(0x00) & 0x0F;
  ch3 = ch3 << 8;
  ch3 += SPI.transfer(0x00);
  digitalWrite(chipSelect, HIGH); delay(5);
  
  // Get temp sensor data
  digitalWrite(chipSelect, LOW);
  tempRaw = SPI.transfer(0x00) & 0x0F;
  tempRaw = tempRaw << 8;
  tempRaw += SPI.transfer(0x00);
  digitalWrite(chipSelect, HIGH); delay(5);
  
  tempC = (tempRaw - 3777.2)/0.47;
  
  Serial.print(ch0, DEC); Serial.print('\t');
  Serial.print(ch1, DEC); Serial.print('\t');
  Serial.print(ch2, DEC); Serial.print('\t');
  Serial.print(ch3, DEC); Serial.print('\t');
  Serial.println(tempRaw, DEC);
  
  delay(500);

}
