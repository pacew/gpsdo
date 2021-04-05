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
			
	dbg_dbgmcu_cr->trace_ioen = 1;
	dbg_dbgmcu_cr->trace_mode = 0; // async trace traceswo pb3
		
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
	rcc_ahb1enr->gpioben = 1;
	small_delay ();
}


void setup_dma (void);

void
setup_tim1 (void)
{
	rcc_apb2enr->tim1en = 1;
	rcc_apb1enr->tim2en = 1;
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
}

long long
get_tim1 (void)
{
	int t1, t1a;
	long long t2;

	while (1) {
		t1 = tim1_cnt->cnt;
		t2 = *(int *)tim2_cnt;
		t1a = tim1_cnt->cnt;
		if (t1 <= t1a)
			break;
	}
	return ((t2 << 16) | t1);
}

int
get_tim1_capture (void)
{
	if (tim1_sr->cc1if)
		return (tim1_ccr1->ccr1);

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
	rcc_ahb1enr->gpioaen = 1;
	small_delay ();
	gpioa_moder->moder3 = 1; // output

	systick_setup ();

	pb11_setup ();
	setup_usart3 ();
	setup_dma ();
	setup_tim1 ();

	if (1) {
		/* PB0 gps onoff */
		gpiob_moder->moder0 = 1; // output
		gpiob_bsrr->bs0 = 1;
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
	// TODO cache bits
	flash_acr->latency = 5; // 5 wait states
	
	rcc_cfgr->ppre2 = 4; // APB2: code 4 mean divide by 2
	rcc_cfgr->ppre1 = 5; // APB1: code 5 means divide by 4
	rcc_cfgr->hpre = 0; // AHB prescaler: 0 means not divided
	
	c_adc_ccr->adcpre = 1; // code 1 = divide by 4: adc clk = 84/4=21

	rcc_pllcfgr->pllsrc = 1; // select HSE clock
	int m = 8; // code 8 means divide by 8
	rcc_pllcfgr->pllm0 = (m & 0x01) ? 1 : 0;
	rcc_pllcfgr->pllm1 = (m & 0x02) ? 1 : 0;
	rcc_pllcfgr->pllm2 = (m & 0x04) ? 1 : 0;
	rcc_pllcfgr->pllm3 = (m & 0x08) ? 1 : 0;
	rcc_pllcfgr->pllm4 = (m & 0x10) ? 1 : 0;
	rcc_pllcfgr->pllm5 = (m & 0x20) ? 1 : 0;

	int n = 168; // code 168 means times 168
	rcc_pllcfgr->plln0 = (n & 0x001) ? 1 : 0;
	rcc_pllcfgr->plln1 = (n & 0x002) ? 1 : 0;
	rcc_pllcfgr->plln2 = (n & 0x004) ? 1 : 0;
	rcc_pllcfgr->plln3 = (n & 0x008) ? 1 : 0;
	rcc_pllcfgr->plln4 = (n & 0x010) ? 1 : 0;
	rcc_pllcfgr->plln5 = (n & 0x020) ? 1 : 0;
	rcc_pllcfgr->plln6 = (n & 0x040) ? 1 : 0;
	rcc_pllcfgr->plln7 = (n & 0x080) ? 1 : 0;
	rcc_pllcfgr->plln8 = (n & 0x100) ? 1 : 0;

	int p = 0; // code 0 means divide by 2
	rcc_pllcfgr->pllp0 = (p & 0) ? 1 : 0;
	rcc_pllcfgr->pllp1 = (p & 1) ? 1 : 0;

	int q = 7; // code 7 means divide by 7
	rcc_pllcfgr->pllq0 = (q & 0x1) ? 1 : 0;
	rcc_pllcfgr->pllq1 = (q & 0x2) ? 1 : 0;
	rcc_pllcfgr->pllq2 = (q & 0x4) ? 1 : 0;
	rcc_pllcfgr->pllq3 = (q & 0x8) ? 1 : 0;

	/* run long enough for all the prescalers to cycle */
	busywait_ticks (1000);

	/* turn on external oscillator, await ready */
	rcc_cr->hseon = 1;
	while (rcc_cr->hserdy == 0)
		;

	/* turn on pll, await ready */
	rcc_cr->pllon = 1;
	while (rcc_cr->pllrdy == 0)
		;

	/* switch to pll */
	
	struct s_rcc_cfgr cfgr = *rcc_cfgr;
	int sw = 2;
	cfgr.sw0 = (sw & 0x1) ? 1 : 0;
	cfgr.sw1 = (sw & 0x2) ? 1 : 0;
	*rcc_cfgr = cfgr;

	/* wait for switch to happen */
	while (1) {
		cfgr = *rcc_cfgr;
		int sws = 0;
		if (cfgr.sw0) sws |= 1;
		if (cfgr.sw1) sws |= 2;
		if (sws == 2)
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
	rcc_apb1enr->usart3en = 1;
	rcc_ahb1enr->gpioben = 1;
	busywait_ticks (1);

	gpiob_moder->moder10 = 2; // alternate func for PB10
	gpiob_moder->moder11 = 2; // alternate func for PB11

	gpiob_afrh->afrh10 = 7; // PB10 alternate 7
	gpiob_afrh->afrh11 = 7; // PB11 alternate 7

	int div = clock / 9600;
	usart3_brr->div_mantissa = (div >> 4);
	usart3_brr->div_fraction = div & 4;

	usart3_cr1->ue = 1; // USART Enable
	busywait_ticks (1);
	usart3_cr1->te = 1; // xmit enable
	usart3_cr1->re = 1; // rcv enable
}

void
setup_dma (void)
{
	enable_dma = 1;
	
	if ((rcc_ahb1enr->dma1en) == 0) {
		rcc_ahb1enr->dma1en = 1;
		busywait_ms (1);
	}
			     
	dma1_s1ndtr->ndt = sizeof gps_dmabuf;
	dma1_s1par->pa = (int)usart3_dr;
	dma1_s1m0ar->m0a = (int)gps_dmabuf;
	
	dma1_s1cr->chsel = 4;
	dma1_s1cr->minc = 1;
	dma1_s1cr->pinc = 0;
	dma1_s1cr->circ = 1;
	dma1_s1cr->dir = 0;
	
	usart3_cr3->dmar = 1;

	dma1_s1cr->en = 1;

}
	  

int
gps_getc (void)
{
	if (enable_dma == 0) {
		if ((usart3_sr->rxne) == 0)
			return (-1);

		return (usart3_dr->dr & 0xff);
	} else {
		int in_idx = (sizeof gps_dmabuf - dma1_s1ndtr->ndt)
			% sizeof gps_dmabuf;
		
		if (gps_dmabuf_out_idx == in_idx)
			return (-1);
		
		int c = gps_dmabuf[gps_dmabuf_out_idx];
		gps_dmabuf_out_idx = (gps_dmabuf_out_idx + 1) 
			% sizeof gps_dmabuf;
		return (c);
	}
}


