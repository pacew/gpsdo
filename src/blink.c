/*
 * simple blink & serial output program
 *
 * bare environment - c initializers won't work
 */

#include "regs.h"

void
small_delay()
{
	int i;
	for (i = 0; i < 1000000; i++) {
		__asm__("nop");
	}
}

/*
 * vectors.S contains the hardware vector table and is placed at the
 * start of the flash image.  The first word points to _estack, a
 * symbol defined in ./ldscript to be the end of internal SRAM, and
 * this pointer is loaded into the stack pointer by the processor
 * reset sequence.  The second word points directly at the c function
 * "start", defined here.
 */
void
start ()
{
	/* LED_STATUS PA3 */
	RCC_AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	small_delay ();

	GPIOA_MODER = (GPIOA_MODER & ~0xc0) | 0x40; // output

	while (1) {
		GPIOA_BSRR = (1 << 3);
		small_delay();

		GPIOA_BSRR = (1 << (3+16));
		small_delay();
	}
}

void
intr_unhandled ()
{
	while(1)
		;
}



