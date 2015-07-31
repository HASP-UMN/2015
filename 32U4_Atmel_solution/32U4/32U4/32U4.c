#include <avr/interrupt.h>
#include <avr/io.h>
#include "32u4.h"

#define FIFO_READ_ENABLE 0B00100000// to FIFO, PF5
#define OUTPUT_ENABLE 0B00000001// to tri state buffer, PF0
#define IOR 0B01000000// from Tomcat, PC6
#define ADDR_VALID_IOR 0B11000000 // from decoder/Tomcat, PC7
#define IRQ_OUT 0B00000010 // to tomcat, PF1
#define IRQ_LOW_ENABLE 0B00110000
#define IRQ_HIGH_ENABLE 0B00110010
#define IRQ_HIGH_DISABLE 0B00110001
#define IOCHRDY 0B00010000

extern void BALE_HANDLER_vect(void);

ISR(INT1_vect){//HF
	PORTF = IRQ_LOW_ENABLE;  //Pulling IRQ low and opening buffer because tomcat holds irqs high by default even though they're active high
	PORTF = IRQ_HIGH_ENABLE;
	PORTF = IRQ_HIGH_DISABLE; // we have to leave the irq high because it's high by default and if we lower it it will go high when the buffer is tristated, causing a false interrupt
}


void init(){

	int i;
	DDRF = IRQ_OUT | OUTPUT_ENABLE | FIFO_READ_ENABLE | IOCHRDY;  //set pins to output mode
	PORTF = PORTF | FIFO_READ_ENABLE | IOCHRDY | OUTPUT_ENABLE; // setting read_enable high initially; this is a requirement so that the FIFO can be reset correctly by atmega2560
	DDRC = 0B00000000; // all inputs, only using 6 and 7
	
	cli(); // used to disable interrupts
	EICRA = EICRA | 0B00001011; // Set INT0 (BALE) as rising edge and INT1 (HF) as falling edge
	EIMSK = EIMSK | 0B00000011; // enable INT0 AND INT1
	sei();
	
	PRESENT_DATA = 0B00010010; // Buffer enabled, fifo_read enabled, isa off (high), iochrdy high (going for ready timing)
	BUFFER_DISABLE = 0B00110011; // buffer disabled, fifo_read disabled
	RESERVED = 0B00000000;
	SREG_SAVE = 0B00000000;


	// delay 2 minutes
	for(i = 0; i < 1600000000; i++){
		asm("nop");
	}

}

int main(){

	init();
	while(1){}


}
