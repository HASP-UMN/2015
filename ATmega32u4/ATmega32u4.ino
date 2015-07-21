#define HALF_FULL_INT 1  // this is the interrupt number, comes from FIFO
#define FIFO_READ_ENABLE B00100000// to FIFO, PF5
#define OUTPUT_ENABLE B00000001// to tri state buffer, PF0
#define BALE_INT 0 // this is the interrupt number, comes from Tomcat
#define IOR B01000000// from Tomcat, PC6
#define ADDR_VALID B10000000 // from decoder/Tomcat, PC7
#define IRQ_OUT B00000010 // to tomcat, PF1

// 2 interrupts(0 and 1 which are PD0 and PD1), 2 regular inputs, and 3 outputs (PD4, PD6, PD7)
 
volatile bool read_fifo_flag = false;
volatile bool half_full_flag = false;
volatile bool address_valid_flag = false;
volatile bool IOR_flag = false;

void half_full_handler(){
  
  PORTF = PORTF | IRQ_OUT; //sets PF1 (IRQ_OUT);
  half_full_flag = true;
  //PORTF = PORTF & ~IRQ_OUT; // turn off IRQ line
}

void bale_handler(){
  
  address_valid_flag = PINC & ADDR_VALID;
  IOR_flag = PINC & IOR;
  
  if (address_valid_flag && IOR_flag){
    read_fifo_flag = true;
  }

  address_valid_flag = false;
  IOR_flag = false;
}

void setup() {
  PORTF |= FIFO_READ_ENABLE; // setting read_enable high initially; this is a requirement so that the FIFO can be reset correctly by atmega2560
  DDRF = IRQ_OUT | OUTPUT_ENABLE | FIFO_READ_ENABLE;  //set pins to output mode
  attachInterrupt(HALF_FULL_INT, half_full_handler, RISING); 
  attachInterrupt(BALE_INT, bale_handler, RISING); 
  
}

void loop() {
  
  if (read_fifo_flag){
    PORTF = PORTF & ~FIFO_READ_ENABLE & ~OUTPUT_ENABLE;  // set read_enable and output_enable, active LOW
    PORTF = PORTF | FIFO_READ_ENABLE | OUTPUT_ENABLE; // turn off read_enable, output_enable
    read_fifo_flag = false;
  }
  
  if (half_full_flag){ // just turns off everything that half_full_handler turns on
    PORTF = PORTF & ~IRQ_OUT; //turn off PF1/IRQ_OUT
    half_full_flag = false; 
  }
}
