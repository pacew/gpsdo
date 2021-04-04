#include "gpsdo.h"


int enable_dma;

unsigned char gps_dmabuf[1024];
int gps_dmabuf_out_idx;


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

static void setup_usart3 (void);
int gps_getc (void);


void small_delay(void);


/* from ARMv7-m Architecture Reference Manual c1.7.2 ITM register summary */
#define ITM_STIM0 (*(unsigned char volatile *)0xE0000000)
#define ITM_TCR (*(unsigned int volatile *)0xE0000E80)
#define ITM_TER0 (*(unsigned int volatile *)0xE0000E00)
#define ITM_TPR (*(unsigned int volatile *)0xE0000E40)
#define ITM_LAR (*(unsigned int volatile *)0xE0000FB0)

/* from ARMv7-m Architecture Reference Manual c1.10.1 TPIO register summary */
#define TPIU_SSPSR (*(unsigned int volatile *)0xE0040000)
#define TPIU_CSPSR (*(unsigned int volatile *)0xE0040004) // current port size
#define TPIU_ACPR (*(unsigned int volatile *) 0xE0040010) // async clock prescl
#define TPIU_SPPR (*(unsigned int volatile *) 0xE00400f0) // selected pin protocol
#define TPIU_FORMATTER (*(unsigned int volatile *)0xe0040304)
#define TPIU_TYPE (*(unsigned int volatile *) 0xE0040fc8)

/* from ARM ® Cortex ® -M4 Processor technical reference manual  */
#define DEMCR (*(unsigned int volatile *) 0xe000edfc)

/* https://community.atmel.com/forum/how-display-itm-based-output-atmel-studio-arduino-due-j-link */

void
setup_swo (void)
{
	DEMCR |= (1 < 24); // TRCENA

	/* Chapter DBG Section ITM */
	/* Configure the TPIU and assign TRACE I/Os by configuring the
	   DBGMCU_CR (refer to Section 38.17.2 and Section 38.16.3) */
	union {
		struct {
			int dbg_sleep : 1;
			int dbg_stop : 1;
			int dbg_standby : 1;
			int : 2;
			int trace_ioen : 1;
			int trace_mode : 2;
		} bits;
		unsigned int word;
	} dbg_dbgmcu_cr;
			
	dbg_dbgmcu_cr.word = DBG_DBGMCU_CR;
	dbg_dbgmcu_cr.bits.trace_ioen = 1;
	dbg_dbgmcu_cr.bits.trace_mode = 0; // async trace traceswo pb3
	DBG_DBGMCU_CR = dbg_dbgmcu_cr.word;
		
	small_delay();

	/* Write 0xC5ACCE55 to the ITM Lock Access register to unlock
	   the write access to the ITM registers */
	ITM_LAR = 0xC5ACCE55;

	/* Write 0x00010005 to the ITM Trace Control register to
	   enable the ITM with Sync enabled and an ATB ID different
	   from 0x00 */

	/*
	  • Write 0x1 to the ITM Trace Enable register to enable the
            Stimulus Port 0

	  • Write 0x1 to the ITM Trace Privilege register to unmask
            stimulus ports 7:0

	  • Write the value to output in the Stimulus Port register 0:
	  this can be done by software (using a printf function)
	*/

	/* 200k appears safe with RZ coding */
	double swo_clock = 115200;
	
	int swoscaler = (hclk_hz / swo_clock) - 1;
	TPIU_ACPR = swoscaler;
	TPIU_SPPR = 1; // 2 = async NRZ; 1 = manchester

	TPIU_CSPSR = 1; // port size 1 bit
	
	TPIU_FORMATTER = 0x100;

	/* C1.7.6 Trace Control Register, ITM_TCR */
	/* also table 308 of stm32fxx rm */
	union {
		struct {
			int itmena : 1;
			int tsena : 1;
			int syncena : 1;
			int txena : 1; // dwtena
			int swoena : 1;
			int : 3;
			int tsprescale : 2;
			int gtsfreq : 2; // unused on stm32f4xx
			int : 4;
			int tracebusid : 7;
			int busy : 1;
			int : 8;
		} bits;
		unsigned int word;
	} itm_tcr;
	itm_tcr.word = ITM_TCR;
	itm_tcr.bits.tracebusid = 1;
	itm_tcr.bits.syncena = 1;
	itm_tcr.bits.itmena = 1;
	itm_tcr.bits.swoena = 0;
	ITM_TCR = itm_tcr.word;
		
	ITM_TPR = 0xffffffff;
	ITM_TER0 |= 1; /* enable stimulus ports */

	
}
	  
int
traceswo_ready (void)
{
	if (ITM_STIM0 & 1)
		return (1);
	return (0);
}

void
swo_putc (int c)
{
	while ((ITM_STIM0 & 1) == 0)
		;
	ITM_STIM0 = c;
}

int swobusy;

void
_putchar (char c)
{
	if (c == '\n')
		_putchar ('\r');

	while ((ITM_STIM0 & 1) == 0)
		swobusy++;
	ITM_STIM0 = c;
}


int delay_speed = 5000 * 1000;

void
small_delay()
{
	int i;
	for (i = 0; i < delay_speed; i++) {
		__asm__("nop");
	}
}

void
pb11_setup (void)
{
	RCC_AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
}


int
pb11_read (void)
{
	return (0);
}

void setup_dma (void);

void
setup_tim1 (void)
{
	struct {
		unsigned int cen : 1;
		unsigned int udis : 1;
		unsigned int urs : 1;
		unsigned int opm : 1;
		unsigned int dir : 1;
		unsigned int cms : 2;
		unsigned int arpe : 1;
		unsigned int ckd : 2;
		unsigned int : 6;
	} volatile *tim1_cr1 = (void *)&TIM1_CR1;

	struct {
		unsigned int ccpc : 1;
		unsigned int : 1;
		unsigned int ccus : 1;
		unsigned int ccds : 1;
		unsigned int mms : 3;
		unsigned int ti1s : 1;
		unsigned int ois1 : 1;
		unsigned int ois1n : 1;
		unsigned int ois2 : 1;
		unsigned int ois2n : 1;
		unsigned int ois3 : 1;
		unsigned int ois3n : 1;
		unsigned int ois4 : 1;
		unsigned int : 1;
	} volatile *tim1_cr2 = (void *)&TIM1_CR2;
		

	struct {
		unsigned int cc1s : 2;
		unsigned int ic1psc : 2;
		unsigned int ic1f : 4;
		unsigned int cc2s : 2;
		unsigned int ic2psc : 2;
		unsigned int ic2f : 4;
	} volatile *tim1_ccmr1_input = (void *)&TIM1_CCMR1_Input;
	
	struct {
		unsigned int cc1e : 1;
		unsigned int cc1p : 1;
		unsigned int cc1ne : 1;
		unsigned int cc1np : 1;
		unsigned int cc2e : 1;
		unsigned int cc2p : 1;
		unsigned int cc2ne : 1;
		unsigned int cc2np : 1;
		unsigned int cc3e : 1;
		unsigned int cc3p : 1;
		unsigned int cc3ne : 1;
		unsigned int cc3np : 1;
		unsigned int cc4e : 1;
		unsigned int cc4p : 1;
		unsigned int : 1;
		unsigned int cc4np : 1;
	} volatile *tim1_ccer = (void *)&TIM1_CCER;

	struct {
		unsigned int cen : 1;
		unsigned int udis : 1;
		unsigned int urs : 1;
		unsigned int opm : 1;
		unsigned int dir : 1;
		unsigned int cms : 2;
		unsigned int arpe : 1;
		unsigned int ckd : 2;
		unsigned int : 6;
	} volatile *tim2_cr1 = (void *)&TIM2_CR1;

	struct {
		unsigned int sms : 3;
		unsigned int : 1;
		unsigned int ts : 3;
		unsigned int msm : 1;
		unsigned int etf : 4;
		unsigned int etps : 2;
		unsigned int ece : 14;
		unsigned int etp : 15;
	} volatile *tim2_smcr = (void *)&TIM2_SMCR;

	RCC_APB2ENR |= RCC_APB2ENR_TIM1EN;
	RCC_APB1ENR |= RCC_APB1ENR_TIM2EN;
	small_delay ();

	if (1) {
		tim1_ccmr1_input->cc1s = 1; // IC1 is mapped on TI1
		tim1_ccer->cc1e = 1; // enable capture
	}

	tim1_cr2->mms = 2; // master mode: trigger output on update
	tim2_smcr->ts = 0; // select ITR0 which is TIM1_TRGO
	tim2_smcr->sms = 7; // ext clock, clock on rising edge of trigger

	tim1_cr1->cen = 1;
	tim2_cr1->cen = 1;

	if (0) {
		unsigned int volatile *reg = &TIM1_CR1;
		for (int i = 0; i < 18; i++) {
			printf ("%p 0x%x\n", &reg[i], reg[i]);
		}
	}
}

long long
get_tim1 (void)
{
	int t1, t1a;
	long long t2;

	while (1) {
		t1 = TIM1_CNT;
		t2 = TIM2_CNT;
		t1a = TIM1_CNT;
		if (t1 <= t1a)
			break;
	}
	return ((t2 << 16) | t1);
}

int
get_tim1_capture (void)
{
	struct {
		unsigned int uif : 1;
		unsigned int cc1if : 1;
		unsigned int cc2if : 1;
		unsigned int cc3if : 1;
		unsigned int cc4if : 1;
		unsigned int comif : 1;
		unsigned int tif : 1;
		unsigned int bif : 1;
		unsigned int : 1;
		unsigned int cc1of : 1;
		unsigned int cc2of : 1;
		unsigned int cc3of : 1;
		unsigned int cc4of : 1;
		unsigned int : 3;
	} volatile *tim1_sr = (void *)&TIM1_SR;

	if (tim1_sr->cc1if)
		return (TIM1_CCR1);

	return (-1);
}

void
gps_process (char *buf)
{
	if (strncmp (buf, "$GNGGA", 6) == 0)
		printf ("%s\n", buf);
}

char gps_buf[200];
int gps_buf_used;

void
gps_soak (void)
{
	int c;
	
	int col = 0;
	while (1) {
		if (! traceswo_ready ())
			break;
		if ((c = gps_getc ()) < 0)
			break;

		if (c == '$')
			gps_buf_used = 0;

		if (c == '\r' || c == '\n') {
			gps_buf[gps_buf_used] = 0;
			if (gps_buf[0] == '$')
				gps_process (gps_buf);
			gps_buf_used = 0;
		} else if (gps_buf_used < sizeof gps_buf - 2) {
			gps_buf[gps_buf_used++] = c;
			gps_buf[gps_buf_used] = 0;
		}

		if (0) {
			col++;
			printf ("%c", c);
			if (c == '\r')
				col = 0;
			if (col >= 100) {
				col = 0;
				printf ("\n");
			}
			break;
		}
	}
}


void
blinker (void)
{
	printf ("hello\n");

	delay_speed = 50000;

	/* LED_STATUS PA3 */
	RCC_AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	small_delay ();

	GPIOA_MODER = (GPIOA_MODER & ~0xc0) | 0x40; // output

	systick_setup ();

	pb11_setup ();
	setup_usart3 ();
	setup_dma ();
	setup_tim1 ();

	if (1) {
		GPIOB_MODER = (GPIOB_MODER & ~3) | 1; // output
		GPIOB_BSRR = (1 << 0);
		//GPIOB_BSRR = (1 << (0+16));
	}


	unsigned int last = systick_read();
	long long last_tick = 0;

	while (1) {
		if (systick_secs (last) >= 1) {
			last = systick_read ();
			int cap = get_tim1_capture ();

			long long this_tick = get_tim1 ();
			printf ("**** tick %15lld %15lld %10d *** \n", 
				this_tick, this_tick - last_tick, cap);
			last_tick = this_tick;
		}

		gps_soak ();
	}
}


extern unsigned char _sidata[], _sdata[], _edata[];
extern unsigned char _sbss[], _ebss[];

/*
 * vectors.S contains the hardware vector table and is placed at the
 * start of the flash image.  The first word points to _estack, a
 * symbol defined in ./ldscript to be the end of internal SRAM, and
 * this pointer is loaded into the stack pointer by the processor
 * reset sequence.  The second word points directly at the c function
 * "start", defined here.
 */
void
start (void)
{
	/* 
	 * the data segment is full of junk when we get here,
	 * so don't call out anywhere before the initialization
	 * immediately below.  gcc can insert a call
	 * memset(3) behind your back if you declare an
	 * initialized local array, so don't do anything here
	 * but the data and bss init, then call main() or some
	 * other function to carry on.
	 */

	/* clear the bss (uninitalized global variables) */
	for (int i = 0; i < _ebss - _sbss; i++)
		_sbss[i] = 0;

	/* copy block of data for initialized variables from flash to ram */
	for (int i = 0; i < _edata - _sdata; i++)
		_sdata[i] = _sidata[i];


	setup_swo ();
	
	blinker ();
}

void
intr_unhandled (void)
{
	while(1)
		;
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


static void
setup_usart3 (void)
{
	/* 
	 * USART3 TX on pin PB10 alt func 7
	 * USART3 RX on pin PB11 alt func 7
	 * on APB1
	 */

	/* careful ... different usarts use different clocks */
	float clock = hclk_hz;

	// enable devices
	RCC_APB1ENR |= RCC_APB1ENR_USART3EN;
	RCC_AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	busywait_ticks (1);

	printf ("before %x\n", GPIOB_MODER);
	GPIOB_MODER |= 2<<(10*2); // alternate func for PB10
	GPIOB_MODER |= 2<<(11*2); // alternate func for PB11

	GPIOB_AFRH |= 7<<((10-8)*4); // PB10 alternate 7
	GPIOB_AFRH |= 7<<((11-8)*4); // PB11 alternate 7

	USART3_BRR = clock / 9600;

	USART3_CR1 |= USART_CR1_UE; // USART Enable
	busywait_ticks (1);
	USART3_CR1 |= USART_CR1_TE | USART_CR1_RE; // xmit & rcv enable

	printf ("RCC_APB1ENR %x\n", RCC_APB1ENR);
	printf ("GPIOB_MODER %x\n", GPIOB_MODER);
	printf ("GPIOB_AFRH %x\n", GPIOB_AFRH);
	printf ("USART3_BRR %x\n", USART3_BRR);
	printf ("USART3_CR1 %x\n", USART3_CR1);

}

void
setup_dma (void)
{
	enable_dma = 1;
	
	if ((RCC_AHB1ENR & RCC_AHB1ENR_DMA1EN) == 0) {
		RCC_AHB1ENR |= RCC_AHB1ENR_DMA1EN;
		busywait_ms (1);
	}
			     
	DMA1_S1NDTR = sizeof gps_dmabuf;
	DMA1_S1PAR = (int)&USART3_DR;
	DMA1_S1M0AR = (int)gps_dmabuf;
	
	struct dma_sxcr {
		unsigned int en : 1;
		unsigned int dmeie : 1;
		unsigned int teie : 1;
		unsigned int htie : 1;
		unsigned int tcie : 1;
		unsigned int pfctrl : 1;
		unsigned int dir : 2;
		unsigned int circ : 1;
		unsigned int pinc : 1;
		unsigned int minc : 1;
		unsigned int psize : 2;
		unsigned int msize : 2;
		unsigned int pincos : 1;
		unsigned int pl : 2;
		unsigned int dbm : 1;
		unsigned int ct : 1;
		unsigned int : 1;
		unsigned int pburst : 2;
		unsigned int mburst : 2;
		unsigned int chsel : 3;
		unsigned int : 4;
	} volatile *s1cr = (void *)&DMA1_S1CR;
	printf ("s1cr before %x\n", DMA1_S1CR);
	s1cr->chsel = 4;
	s1cr->minc = 1;
	s1cr->pinc = 0;
	s1cr->circ = 1;
	s1cr->dir = 0;
	printf ("s1cr after %x\n", DMA1_S1CR);

	
	USART3_CR3 |= USART_CR3_DMAR;

	s1cr->en = 1;

}
	  

int
gps_getc (void)
{
	if (enable_dma == 0) {
		if ((USART3_SR & USART_SR_RXNE) == 0)
			return (-1);

		return (USART3_DR & 0xff);
	} else {
		int in_idx = (sizeof gps_dmabuf - DMA1_S1NDTR) % sizeof gps_dmabuf;
		
		if (gps_dmabuf_out_idx == in_idx)
			return (-1);
		
		int c = gps_dmabuf[gps_dmabuf_out_idx];
		gps_dmabuf_out_idx = (gps_dmabuf_out_idx + 1) 
			% sizeof gps_dmabuf;
		return (c);
	}
}


