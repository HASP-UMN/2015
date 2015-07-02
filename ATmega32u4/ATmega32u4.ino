#define half_full_int 0  // this is the interrupt number, comes from FIFO
//#define read_enable  // to FIFO
//#define output_enable // to tri state buffer
#define bale_int 1 // this is the interrupt number, comes from Tomcat
//#define IOR // PD2, from Tomcat
//#define address_valid // PD3, from decoder/Tomcat
//#define IRQ_OUT // to Tomcat

// 2 interrupts(0 and 1 which are PD0 and PD1), 2 regular inputs, and 3 outputs (PD4, PD6, PD7)
 
volatile bool flush_fifo_flag = false;
volatile bool half_full_flag = false;
volatile bool address_valid_flag = false;
volatile bool IOR_flag = false;


void half_full_handler(){
  
  PORTD = PORTD | B10000000; //sets PD7 (IRQ_OUT)
  half_full_flag = true;
}

void bale_handler(){
  
  address_valid_flag = PIND & B00000100;
  IOR_flag = PIND & B00001000;
  
  if (address_valid_flag && IOR_flag){ // wrong, these are constants, needs to check the pins they represent
    flush_fifo_flag = true;
    address_valid_flag = false;
    IOR_flag = false;
  }
}

void setup() {
  
  DDRD = B11010000;
  // PD7, PD6, and PD4 will be outputs IRQ_OUT, output_enable, and read_enable
  // PD7 = DigitalPin 6, PD6 = DigitalPin 12, PD4 = DigitalPin4
  
  attachInterrupt(half_full_int, half_full_handler, RISING); 
  attachInterrupt(bale_int, bale_handler, RISING); 

}

void loop() {
  
  if (flush_fifo_flag){
    PORTD = PORTD | B01010000  // set read_enable and output_enable
    PORTD = PORTD & ~B11010000 // turn off read_enable, output_enable, and IRQ_OUT
    flush_fifo_flag = false;
  }
  
  if (half_full_flag){ // just turns off everything that half_full_handler turns on
    PORTD = PIND | B0000000; //turn off PD7/IRQ_OUT
    half_full_flag = false; 
  }
