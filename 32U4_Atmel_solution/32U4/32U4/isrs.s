#include <avr/io.h>
#include "32u4.h"

.global INT0_vect

INT0_vect:
	in 	SREG_SAVE, 0x3f    	 	; getting SREG, wont need this if we do the hardware fix
	in	RESERVED, 0x06		 	; PINC, start of if statement
	cpi	RESERVED, 0x80			; PINC == B10000000
	brne	.+10
	out		0x11, PRESENT_DATA		; must beat IOR here
	nop
	nop
	nop
	out 	0x11, BUFFER_DISABLE		; this may be too fast actually, insert nops between outs if so
	out 	0x3f, SREG_SAVE			; restore SREG
	reti

