#define HALF_FULL_INT 1  // this is the interrupt number, comes from FIFO
#define FIFO_READ_ENABLE B00100000// to FIFO, PF5
#define OUTPUT_ENABLE B00000001// to tri state buffer, PF0
#define BALE_INT 0 // this is the interrupt number, comes from Tomcat
#define IOR B01000000// from Tomcat, PC6
#define ADDR_VALID_IOR B11000000 // from decoder/Tomcat, PC7
#define IRQ_OUT B00000010 // to tomcat, PF1
#define IOCHRDY B00010000 // to tomcat, PF4
 
volatile bool bale_int_flag = false;
volatile bool half_full_flag = false;
//volatile bool address_valid_flag = false;
//volatile bool IOR_flag = false;

volatile unsigned int half_full_count = 0;
volatile unsigned int bale_count = 0;
volatile unsigned int av_count = 0;
volatile int i;

void half_full_handler(){
  PORTF = PORTF | IRQ_OUT;
  half_full_count++;
  
}

void bale_handler(){
  
  //address_valid_flag = PINC & ADDR_VALID;
  //IOR_flag = PINC & IOR;
  
//  if (address_valid_flag && IOR_flag){
    if ( (PINC & ADDR_VALID_IOR) == B10000000){ // think IOR is active low from tomcat
    PORTF = PORTF & ~IOCHRDY; // active low
    PORTF = PORTF & ~FIFO_READ_ENABLE & ~OUTPUT_ENABLE;  // set read_enable and output_enable, active LOW
    PORTF = PORTF | IOCHRDY; // alerts tomcat that data should be available on ISA bus
    PORTF = PORTF | FIFO_READ_ENABLE | OUTPUT_ENABLE; // turn off read_enable, output_enable    
    av_count++;
  }

  //address_valid_flag = false;
  //IOR_flag = false;
  bale_count++;
  
}

void setup() {
  DDRF = IRQ_OUT | OUTPUT_ENABLE | FIFO_READ_ENABLE;  //set pins to output mode
  PORTF = PORTF | FIFO_READ_ENABLE | IOCHRDY & ~OUTPUT_ENABLE; // setting read_enable high initially; this is a requirement so that the FIFO can be reset correctly by atmega2560
  // also setting IOCHRDY high by default, it is active low
  // also setting OUTPUT_ENABLE high (off) initially so tomcat can correctly start up
  attachInterrupt(HALF_FULL_INT, half_full_handler, FALLING); // this line is active low from FIFO 
  attachInterrupt(BALE_INT, bale_handler, RISING);  
  half_full_count = 0;
  bale_count = 0;
  av_count = 0;  
  delay(10000);
  Serial.begin(115200); Serial.flush();
  Serial.println("Interrupt Counters:");
  Serial.println("=====================");


}

void loop() {
// just waiting for interrupts
//  Serial.print("HF: ");Serial.println(half_full_count);
//  Serial.print("Bale: "); Serial.println(bale_count);
//  Serial.print("AV: "); Serial.println(av_count);
//  delay(500); 
  if (half_full_count > 0){
    for(i = 0; i < 512; i++){
      PORTF = PORTF & ~FIFO_READ_ENABLE;  //sets PF1 (IRQ_OUT);
      delay(500); 
      Serial.println("read cycle"); 
      PORTF = PORTF | FIFO_READ_ENABLE;//turn off PF1/IRQ_OUT  // data should show on bus now
      delay(500);
    }  
  }
}
