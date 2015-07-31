/*
 * Four global registers are reserved for interrupt optimization
 * Must use caller saved registers (r18 - r27) for this to work
*/

#ifdef __ASSEMBLER__

#  define PRESENT_DATA				r18
#  define BUFFER_DISABLE			r19
#  define RESERVED					r20
#  define SREG_SAVE	                r21
#  define IOCHRDY_HIGH				r22
#else  /* !ASSEMBLER */

#include <stdint.h>

register uint8_t PRESENT_DATA asm("r18"); // //IOCHRDY low, read_enable low, output_enable low, irq high (default high);
register uint8_t BUFFER_DISABLE asm("r19");  //IOCHRDY high, read_enable high, output_enable low, irq high;
register uint8_t RESERVED asm("r20"); // reserving this register now for use in ISR: eliminates push(r24): if we dont have if statement this isn't necessary;
register uint8_t SREG_SAVE asm("r21"); // reserving this register for storing sreg : eliminates 2 pushes;
register uint8_t IOCHRDY_HIGH asm("r22");

#endif /* ASSEMBLER */
