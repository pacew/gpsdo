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

double hclk_hz = 16e6;
double apb1_hz = 4e6;
double apb2_hz = 8e6;

/* see DDI0489B_cortex_m7_trm.pdf and DDI0403E_B_armv7m_arm.pdf */
#define SYST_CSR (*(unsigned int volatile *)0xE000E010)
#define SYST_RVR (*(unsigned int volatile *)0xE000E014)
#define SYST_CVR (*(unsigned int volatile *)0xE000E018)
#define SYST_CALIB (*(unsigned int volatile *)0xE000E01C)
static int systicks_per_10ms = 0;

/*
 * call this after changing clock speed
 *
 * sets up systick to be freerunning 24 bit downcounter
 * at ahb_hz/8 ticks per second. figure on it wrapping
 * about once a minue
 */
static void
systick_setup (void)
{
	SYST_RVR = 0xffffff; // reload register
	SYST_CVR = 0; // current count

	/* enable, use ahb_hz / 8 clock, no interrupt */
	SYST_CSR = (SYST_CSR & ~7) | 1;

	systicks_per_10ms = hclk_hz / 8 / 100;
}

#define systick_read() (SYST_CVR & 0x00ffffff)

float
systick_delta_secs (unsigned int end, unsigned int start)
{
	unsigned int delta_ticks = (start - end) & 0x00ffffff;
	return ((float)delta_ticks / (systicks_per_10ms * 100));
}

float
systick_secs (unsigned int start)
{
	unsigned int delta_ticks = (start - systick_read()) & 0x00ffffff;
	return ((float)delta_ticks / (systicks_per_10ms * 100));
}

void
busywait_ticks (int ticks)
{
	unsigned int start = SYST_CVR & 0xffffff;

	if ((SYST_CSR & 1) == 0)
		systick_setup ();
	
	while (1) {
		unsigned int delta = (start - SYST_CVR) & 0x00ffffff;
		if (delta >= ticks)
			break;
	}
}

void
busywait_ms (int ms)
{
	int togo = ms;
	
	/* simple strategy with no overflow worries */
	while (togo >= 10) {
		busywait_ticks (systicks_per_10ms);
		togo -= 10;
	}
	busywait_ticks (togo * (systicks_per_10ms + 9) / 10);
}

void
busywait_us (int us)
{
	int togo = us;
	
	/* simple strategy with no overflow worries */
	while (togo >= 10000) {
		busywait_ticks (systicks_per_10ms);
		togo -= 10000;
	}
	busywait_ticks (togo * (systicks_per_10ms + 9999) / 10000);
}

/* processor specific */
void
setup_clock (void)
{
	union {
		struct {
			/* TODO cache bits */
			int latency : 3;
		} bits;
		unsigned int word;
	} flash;
	flash.word = FLASH_ACR;
	flash.bits.latency = 5; // 5 wait states
	FLASH_ACR = flash.word;

	union {
		struct rcc_cfgr {
			unsigned int sw : 2;
			unsigned int sws : 2;
			unsigned int hpre : 4;
			unsigned int ppre1 : 3;
			unsigned int ppre2 : 3;
			unsigned int rtcpre : 5;
			unsigned int mco1 : 2;
			unsigned int i2ssrc : 1;
			unsigned int mco1pre : 3;
			unsigned int mco2pre : 3;
			unsigned int mco2 : 2;
		} bits;
		unsigned int word;
	} rcc_cfgr;
	rcc_cfgr.word = RCC_CFGR;
	rcc_cfgr.bits.ppre2 = 4; // APB2: code 4 mean divide by 2
	rcc_cfgr.bits.ppre1 = 5; // APB1: code 5 means divide by 4
	rcc_cfgr.bits.hpre = 0; // AHB prescaler: 0 means not divided
	RCC_CFGR = rcc_cfgr.word;
	
	union {
		struct {
			unsigned int : 16;
			unsigned int adcpre : 2;
		} bits;
		unsigned int word;
	} c_adc_ccr;
	c_adc_ccr.word = C_ADC_CCR;
	c_adc_ccr.bits.adcpre = 1; // code 1 = divide by 4: adc clk = 84/4=21
	C_ADC_CCR = c_adc_ccr.word;


	union {
		struct {
			unsigned int m : 6;
			unsigned int n : 9;
			unsigned int : 1;
			unsigned int p : 2;
			unsigned int : 4;
			unsigned int src : 1;
			unsigned int q : 4;
			unsigned int : 4;
		} bits;
		unsigned int word;
	} pllcfgr;
	pllcfgr.word = RCC_PLLCFGR;
	pllcfgr.bits.src = 1; // select HSE clock
	pllcfgr.bits.m = 8; // code 8 means divide by 8
	pllcfgr.bits.n = 168; // code 168 means times 168
	pllcfgr.bits.p = 0; // code 0 means divide by 2
	pllcfgr.bits.q = 7; // code 7 means divide by 7
	RCC_PLLCFGR = pllcfgr.word;


	/* run long enough for all the prescalers to cycle */
	busywait_ticks (1000);

	/* turn on external oscillator, await ready */
	RCC_CR |= RCC_CR_HSEON;
	while ((RCC_CR & RCC_CR_HSERDY) == 0)
		;

	/* turn on pll, await ready */
	RCC_CR |= RCC_CR_PLLON;
	while ((RCC_CR & RCC_CR_PLLRDY) == 0)
		;

	/* switch to pll */
	rcc_cfgr.bits.sw = 2;
	RCC_CFGR = rcc_cfgr.word;

	/* wait for switch to happen */
	while (1) {
		rcc_cfgr.word = RCC_CFGR;
		if (rcc_cfgr.bits.sws == 2)
			break;
	}

	/* update software speeds */
	hclk_hz = 168e6;
	apb1_hz = 42e6;
	apb2_hz = 84e6;
	systick_setup ();
}
