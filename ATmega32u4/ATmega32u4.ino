#define HALF_FULL_INT 1  // this is the interrupt number, comes from FIFO
#define FIFO_READ_ENABLE B00100000// to FIFO, PF5
#define OUTPUT_ENABLE B00000001// to tri state buffer, PF0
#define BALE_INT 0 // this is the interrupt number, comes from Tomcat
#define IOR B01000000// from Tomcat, PC6
#define ADDR_VALID_IOR B11000000 // from decoder/Tomcat, PC7
#define IRQ_OUT B00000010 // to tomcat, PF1
#define IOCHRDY B00010000 // to tomcat, PF4
#define IOCHRDY_READ_OUTPUT_ENABLE B00110001 

volatile bool bale_int_flag = false;
volatile bool half_full_flag = false;
//volatile bool address_valid_flag = false;
//volatile bool IOR_flag = false;

void half_full_handler(){
  
  PORTF |= IRQ_OUT;
  PORTF &= ~IRQ_OUT;
}

void bale_handler(){
  
  if ( (PINC & ADDR_VALID_IOR) == B10000000){ // think IOR is active low from tomcat
    PORTF &= ~IOCHRDY_READ_OUTPUT_ENABLE;
    PORTF |= IOCHRDY_READ_OUTPUT_ENABLE;    
  }
  
}

void setup() {
  DDRF = IRQ_OUT | OUTPUT_ENABLE | FIFO_READ_ENABLE | IOCHRDY;  //set pins to output mode
  PORTF = PORTF | FIFO_READ_ENABLE | IOCHRDY | OUTPUT_ENABLE; // setting read_enable high initially; this is a requirement so that the FIFO can be reset correctly by atmega2560
  // also setting IOCHRDY high by default, it is active low
  // also setting OUTPUT_ENABLE high (off) initially so tomcat can correctly start up
  attachInterrupt(HALF_FULL_INT, half_full_handler, FALLING); // this line is active low from FIFO 
  attachInterrupt(BALE_INT, bale_handler, RISING);  

}

void loop() {
// just waiting for interrupts

}
