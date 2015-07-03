#define HALF_FULL_INT 0  // this is the interrupt number, comes from FIFO
#define READ_ENABLE B00010000// to FIFO
#define OUTPUT_ENABLE B01000000// to tri state buffer
#define BALE_INT 1 // this is the interrupt number, comes from Tomcat
#define IOR B00001000// PD2, from Tomcat
#define ADDRESS_DECODER B00000100 // PD3, from decoder/Tomcat
#define IRQ_OUT B10000000 // to tomcat

// 2 interrupts(0 and 1 which are PD0 and PD1), 2 regular inputs, and 3 outputs (PD4, PD6, PD7)
 
volatile bool flush_fifo_flag = false;
volatile bool half_full_flag = false;
volatile bool address_valid_flag = false;
volatile bool IOR_flag = false;

void half_full_handler(){
  
  PORTD = PORTD | IRQ_OUT; //sets PD7 (IRQ_OUT);
  half_full_flag = true;
}

void bale_handler(){
  
  address_valid_flag = PIND & ADDRESS_DECODER;
  IOR_flag = PIND & IOR;
  
  if (address_valid_flag && IOR_flag){ // wrong, these are constants, needs to check the pins they represent
    flush_fifo_flag = true;
    address_valid_flag = false;
    IOR_flag = false;
  }
}

void setup() {
  
  DDRD = IRQ_OUT | OUTPUT_ENABLE | READ_ENABLE;
  // PD7, PD6, and PD4 will be outputs IRQ_OUT, output_enable, and read_enable
  // PD7 = DigitalPin 6, PD6 = DigitalPin 12, PD4 = DigitalPin4
  
  attachInterrupt(HALF_FULL_INT, half_full_handler, RISING); 
  attachInterrupt(BALE_INT, bale_handler, RISING); 

}

void loop() {
  
  if (flush_fifo_flag){
    PORTD = PORTD | READ_ENABLE | OUTPUT_ENABLE;  // set read_enable and output_enable
    PORTD = PORTD & ~READ_ENABLE & ~OUTPUT_ENABLE & ~IRQ_OUT // turn off read_enable, output_enable, and IRQ_OUT
    flush_fifo_flag = false;
  }
  
  if (half_full_flag){ // just turns off everything that half_full_handler turns on
    PORTD = PIND & ~IRQ_OUT; //turn off PD7/IRQ_OUT
    half_full_flag = false; 
  }
