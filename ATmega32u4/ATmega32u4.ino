#define HALF_FULL_INT 1  // this is the interrupt number, comes from FIFO
#define FIFO_READ_ENABLE B00100000// to FIFO, PF5
#define OUTPUT_ENABLE B00000001// to tri state buffer, PF0
#define BALE_INT 0 // this is the interrupt number, comes from Tomcat
#define IOR B01000000// from Tomcat, PC6
#define ADDR_VALID_IOR B11000000 // from decoder/Tomcat, PC7
#define IRQ_OUT B00000010 // to tomcat, PF1

#define IOCHRDY B00010000 // to tomcat, PF4
#define IOCHRDY_READ_OUTPUT_ENABLE  B00000010
#define IOCHRDY_READ_HIGH           B00110010
#define IOCHRDY_READ_OUTPUT_DISABLE B00110001
#define BUFFER_DISABLE

#define IRQ_LOW_ENABLE  B00110000  // OE active, IRQ, IOCHRDY, FIFO_RD, inactive
#define IRQ_HIGH_ENABLE B00110010 // IRQ, OE active; IOCHRDY, FIFO_RD inactive
//#define IRQ_LOW_DISABLE B00110001 // IRQ, OE, IOCHRDY, FIFO_RD inactive. 
#define IRQ_HIGH_DISABLE  B00110001

volatile unsigned int HF_count = 0;
volatile unsigned int bale_count = 0;
volatile unsigned int av_count = 0;

void half_full_handler(){
  
//  PORTF = PORTF | IRQ_OUT & ~OUTPUT_ENABLE;// turning OUTPUT_ENABLE low here as well so irq gets through buffer
////  HF_count++;  
//  PORTF = PORTF & ~IRQ_OUT | OUTPUT_ENABLE; 
  PORTF = IRQ_LOW_ENABLE;  //Pulling IRQ low and opening buffer because tomcat holds irqs high by default even though they're active high
  PORTF = IRQ_HIGH_ENABLE; 
  PORTF = IRQ_HIGH_DISABLE;
// we have to leave the irq high because it's high by default and if we lower it it will go high when the buffer is tristated, causing a false interrupt
 // HF_count++;
  
}

void bale_handler(){
  
  //if ( (PINC & ADDR_VALID_IOR) == B10000000){ // think IOR is active low from tomcat
  if (PINC == B10000000){
    PORTF = B00000010;    //IOCHRDY low, read_enable low, output_enable low, irq high (default high)
    PORTF = B00110010;    //IOCHRDY high, read_enable high, output_enable low, irq high
    PORTF = B00110011;                     //IOCHRDY high, read_enable high, output_enable high, irq high
 //   av_count++;   
  }
//  bale_count++;
  
}

void setup() {
  DDRF = IRQ_OUT | OUTPUT_ENABLE | FIFO_READ_ENABLE | IOCHRDY;  //set pins to output mode
  PORTF = PORTF | FIFO_READ_ENABLE | IOCHRDY | OUTPUT_ENABLE; // setting read_enable high initially; this is a requirement so that the FIFO can be reset correctly by atmega2560
  DDRC = B00000000; // all inputs, only using 6 and 7
  // also setting IOCHRDY high by default, it is active low
  // also setting OUTPUT_ENABLE high (off) initially so tomcat can correctly start up
  delay(120000);
  attachInterrupt(HALF_FULL_INT, half_full_handler, FALLING); // this line is active low from FIFO 
  attachInterrupt(BALE_INT, bale_handler, RISING);  
//  Serial.begin(115200); Serial.flush();
//  Serial.println("Starting");
}

void loop() {
// just waiting for interrupts
//  Serial.print("HF: "); Serial.println(HF_count);
// // Serial.print("Bale: "); Serial.println(bale_count);
//  Serial.print("AV: "); Serial.println(av_count);
//  delay(500);

}
