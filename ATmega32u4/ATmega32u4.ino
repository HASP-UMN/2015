#define HALF_FULL_INT 1  // this is the interrupt number, comes from FIFO
#define FIFO_READ_ENABLE B00100000// to FIFO, PF5
#define OUTPUT_ENABLE B00000001// to tri state buffer, PF0
#define BALE_INT 0 // this is the interrupt number, comes from Tomcat
#define IOR B01000000// from Tomcat, PC6
#define ADDR_VALID_IOR B11000000 // from decoder/Tomcat, PC7
#define IRQ_OUT B00000010 // to tomcat, PF1
#define IOCHRDY B00010000 // to tomcat, PF4
#define IOCHRDY_READ_OUTPUT_ENABLE B00110001 

volatile byte HF_count = 0;
volatile byte bale_count = 0;

void half_full_handler(){
  
  PORTF |= IRQ_OUT;
  PORTF &= ~IRQ_OUT;
  HF_count++;  
}

void bale_handler(){
  
  if ( (PINC & ADDR_VALID_IOR) == B10000000){ // think IOR is active low from tomcat
    PORTF &= ~IOCHRDY_READ_OUTPUT_ENABLE;
    PORTF |= IOCHRDY_READ_OUTPUT_ENABLE;    
  }
 // bale_count++;
  
}

void setup() {
  DDRF = IRQ_OUT | OUTPUT_ENABLE | FIFO_READ_ENABLE | IOCHRDY;  //set pins to output mode
  PORTF = PORTF | FIFO_READ_ENABLE | IOCHRDY | OUTPUT_ENABLE & ~IRQ_OUT; // setting read_enable high initially; this is a requirement so that the FIFO can be reset correctly by atmega2560
  // also setting IOCHRDY high by default, it is active low
  // also setting OUTPUT_ENABLE high (off) initially so tomcat can correctly start up
  attachInterrupt(HALF_FULL_INT, half_full_handler, FALLING); // this line is active low from FIFO 
  attachInterrupt(BALE_INT, bale_handler, RISING);  
  Serial.begin(115200); Serial.flush();
  delay(60000);
  Serial.println("Starting");
}

void loop() {
// just waiting for interrupts
  Serial.print("HF: "); Serial.println(HF_count);
  Serial.print("Bale: "); Serial.println(bale_count);
  delay(500);

}
