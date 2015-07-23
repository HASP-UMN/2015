#define HALF_FULL_INT 1  // this is the interrupt number, comes from FIFO
#define FIFO_READ_ENABLE B00100000// to FIFO, PF5
#define OUTPUT_ENABLE B00000001// to tri state buffer, PF0
#define BALE_INT 0 // this is the interrupt number, comes from Tomcat
#define IOR B01000000// from Tomcat, PC6
#define ADDR_VALID B10000000 // from decoder/Tomcat, PC7
#define IRQ_OUT B00000010 // to tomcat, PF1
#define IOCHRDY B00010000 // to tomcat, PF4
 
volatile bool bale_int_flag = false;
volatile bool half_full_flag = false;
volatile bool address_valid_flag = false;
volatile bool IOR_flag = false;

void half_full_handler(){
  
  PORTF = PORTF | IRQ_OUT; //sets PF1 (IRQ_OUT);
  half_full_flag = true;
}

void bale_handler(){
  
  address_valid_flag = PINC & ADDR_VALID;
  IOR_flag = PINC & IOR;
  
  if (address_valid_flag && IOR_flag){
    PORTF = PORTF & ~IOCHRDY; // active low
    bale_int_flag = true;
  }

  address_valid_flag = false;
  IOR_flag = false;
}

void setup() {
  DDRF = IRQ_OUT | OUTPUT_ENABLE | FIFO_READ_ENABLE;  //set pins to output mode
  PORTF = PORTF | FIFO_READ_ENABLE | IOCHRDY; // setting read_enable high initially; this is a requirement so that the FIFO can be reset correctly by atmega2560
  // also setting IOCHRDY high by default, it is active low
  
  attachInterrupt(HALF_FULL_INT, half_full_handler, FALLING); // this line is active low from FIFO 
  attachInterrupt(BALE_INT, bale_handler, RISING);   
}

void loop() {
  
  if (bale_int_flag){
    PORTF = PORTF & ~FIFO_READ_ENABLE & ~OUTPUT_ENABLE;  // set read_enable and output_enable, active LOW
    PORTF = PORTF | IOCHRDY; // alerts tomcat that data should be available on ISA bus
    PORTF = PORTF | FIFO_READ_ENABLE | OUTPUT_ENABLE; // turn off read_enable, output_enable
    bale_int_flag = false;
  }
  
  if (half_full_flag){ // just turns off everything that half_full_handler turns on
    PORTF = PORTF & ~IRQ_OUT; //turn off PF1/IRQ_OUT
    half_full_flag = false; 
  }
}
