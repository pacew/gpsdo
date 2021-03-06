/* generated by mkregs.h */
struct s_dbg_dbgmcu_cr {
	unsigned int dbg_sleep : 1;
	unsigned int dbg_stop : 1;
	unsigned int dbg_standby : 1;
	unsigned int : 2;
	unsigned int trace_ioen : 1;
	unsigned int trace_mode : 2;
	unsigned int : 8;
	unsigned int dbg_i2c2_smbus_timeout : 1;
	unsigned int dbg_tim8_stop : 1;
	unsigned int dbg_tim5_stop : 1;
	unsigned int dbg_tim6_stop : 1;
	unsigned int dbg_tim7_stop : 1;
	unsigned int : 11;
} volatile * const dbg_dbgmcu_cr = (void *)0xe0042004;

struct s_dma1_s1cr {
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
	unsigned int ack : 1;
	unsigned int pburst : 2;
	unsigned int mburst : 2;
	unsigned int chsel : 3;
	unsigned int : 4;
} volatile * const dma1_s1cr = (void *)0x40026028;

struct s_dma1_s1ndtr {
	unsigned int ndt : 16;
	unsigned int : 16;
} volatile * const dma1_s1ndtr = (void *)0x4002602c;

struct s_dma1_s1par {
	unsigned int pa : 32;
} volatile * const dma1_s1par = (void *)0x40026030;

struct s_dma1_s1m0ar {
	unsigned int m0a : 32;
} volatile * const dma1_s1m0ar = (void *)0x40026034;

struct s_rcc_cr {
	unsigned int hsion : 1;
	unsigned int hsirdy : 1;
	unsigned int : 1;
	unsigned int hsitrim : 5;
	unsigned int hsical : 8;
	unsigned int hseon : 1;
	unsigned int hserdy : 1;
	unsigned int hsebyp : 1;
	unsigned int csson : 1;
	unsigned int : 4;
	unsigned int pllon : 1;
	unsigned int pllrdy : 1;
	unsigned int plli2son : 1;
	unsigned int plli2srdy : 1;
	unsigned int : 4;
} volatile * const rcc_cr = (void *)0x40023800;

struct s_rcc_pllcfgr {
	unsigned int pllm0 : 1;
	unsigned int pllm1 : 1;
	unsigned int pllm2 : 1;
	unsigned int pllm3 : 1;
	unsigned int pllm4 : 1;
	unsigned int pllm5 : 1;
	unsigned int plln0 : 1;
	unsigned int plln1 : 1;
	unsigned int plln2 : 1;
	unsigned int plln3 : 1;
	unsigned int plln4 : 1;
	unsigned int plln5 : 1;
	unsigned int plln6 : 1;
	unsigned int plln7 : 1;
	unsigned int plln8 : 1;
	unsigned int : 1;
	unsigned int pllp0 : 1;
	unsigned int pllp1 : 1;
	unsigned int : 4;
	unsigned int pllsrc : 1;
	unsigned int : 1;
	unsigned int pllq0 : 1;
	unsigned int pllq1 : 1;
	unsigned int pllq2 : 1;
	unsigned int pllq3 : 1;
	unsigned int : 4;
} volatile * const rcc_pllcfgr = (void *)0x40023804;

struct s_rcc_cfgr {
	unsigned int sw0 : 1;
	unsigned int sw1 : 1;
	unsigned int sws0 : 1;
	unsigned int sws1 : 1;
	unsigned int hpre : 4;
	unsigned int : 2;
	unsigned int ppre1 : 3;
	unsigned int ppre2 : 3;
	unsigned int rtcpre : 5;
	unsigned int mco1 : 2;
	unsigned int i2ssrc : 1;
	unsigned int mco1pre : 3;
	unsigned int mco2pre : 3;
	unsigned int mco2 : 2;
} volatile * const rcc_cfgr = (void *)0x40023808;

struct s_rcc_ahb1enr {
	unsigned int gpioaen : 1;
	unsigned int gpioben : 1;
	unsigned int gpiocen : 1;
	unsigned int gpioden : 1;
	unsigned int gpioeen : 1;
	unsigned int gpiofen : 1;
	unsigned int gpiogen : 1;
	unsigned int gpiohen : 1;
	unsigned int gpioien : 1;
	unsigned int : 3;
	unsigned int crcen : 1;
	unsigned int : 5;
	unsigned int bkpsramen : 1;
	unsigned int : 2;
	unsigned int dma1en : 1;
	unsigned int dma2en : 1;
	unsigned int : 2;
	unsigned int ethmacen : 1;
	unsigned int ethmactxen : 1;
	unsigned int ethmacrxen : 1;
	unsigned int ethmacptpen : 1;
	unsigned int otghsen : 1;
	unsigned int otghsulpien : 1;
	unsigned int : 1;
} volatile * const rcc_ahb1enr = (void *)0x40023830;

struct s_rcc_apb1enr {
	unsigned int tim2en : 1;
	unsigned int tim3en : 1;
	unsigned int tim4en : 1;
	unsigned int tim5en : 1;
	unsigned int tim6en : 1;
	unsigned int tim7en : 1;
	unsigned int tim12en : 1;
	unsigned int tim13en : 1;
	unsigned int tim14en : 1;
	unsigned int : 2;
	unsigned int wwdgen : 1;
	unsigned int : 2;
	unsigned int spi2en : 1;
	unsigned int spi3en : 1;
	unsigned int : 1;
	unsigned int usart2en : 1;
	unsigned int usart3en : 1;
	unsigned int uart4en : 1;
	unsigned int uart5en : 1;
	unsigned int i2c1en : 1;
	unsigned int i2c2en : 1;
	unsigned int i2c3en : 1;
	unsigned int : 1;
	unsigned int can1en : 1;
	unsigned int can2en : 1;
	unsigned int : 1;
	unsigned int pwren : 1;
	unsigned int dacen : 1;
	unsigned int : 2;
} volatile * const rcc_apb1enr = (void *)0x40023840;

struct s_rcc_apb2enr {
	unsigned int tim1en : 1;
	unsigned int tim8en : 1;
	unsigned int : 2;
	unsigned int usart1en : 1;
	unsigned int usart6en : 1;
	unsigned int : 2;
	unsigned int adc1en : 1;
	unsigned int adc2en : 1;
	unsigned int adc3en : 1;
	unsigned int sdioen : 1;
	unsigned int spi1en : 1;
	unsigned int : 1;
	unsigned int syscfgen : 1;
	unsigned int : 1;
	unsigned int tim9en : 1;
	unsigned int tim10en : 1;
	unsigned int tim11en : 1;
	unsigned int : 13;
} volatile * const rcc_apb2enr = (void *)0x40023844;

struct s_gpiob_moder {
	unsigned int moder0 : 2;
	unsigned int moder1 : 2;
	unsigned int moder2 : 2;
	unsigned int moder3 : 2;
	unsigned int moder4 : 2;
	unsigned int moder5 : 2;
	unsigned int moder6 : 2;
	unsigned int moder7 : 2;
	unsigned int moder8 : 2;
	unsigned int moder9 : 2;
	unsigned int moder10 : 2;
	unsigned int moder11 : 2;
	unsigned int moder12 : 2;
	unsigned int moder13 : 2;
	unsigned int moder14 : 2;
	unsigned int moder15 : 2;
} volatile * const gpiob_moder = (void *)0x40020400;

struct s_gpiob_bsrr {
	unsigned int bs0 : 1;
	unsigned int bs1 : 1;
	unsigned int bs2 : 1;
	unsigned int bs3 : 1;
	unsigned int bs4 : 1;
	unsigned int bs5 : 1;
	unsigned int bs6 : 1;
	unsigned int bs7 : 1;
	unsigned int bs8 : 1;
	unsigned int bs9 : 1;
	unsigned int bs10 : 1;
	unsigned int bs11 : 1;
	unsigned int bs12 : 1;
	unsigned int bs13 : 1;
	unsigned int bs14 : 1;
	unsigned int bs15 : 1;
	unsigned int br0 : 1;
	unsigned int br1 : 1;
	unsigned int br2 : 1;
	unsigned int br3 : 1;
	unsigned int br4 : 1;
	unsigned int br5 : 1;
	unsigned int br6 : 1;
	unsigned int br7 : 1;
	unsigned int br8 : 1;
	unsigned int br9 : 1;
	unsigned int br10 : 1;
	unsigned int br11 : 1;
	unsigned int br12 : 1;
	unsigned int br13 : 1;
	unsigned int br14 : 1;
	unsigned int br15 : 1;
} volatile * const gpiob_bsrr = (void *)0x40020418;

struct s_gpiob_afrh {
	unsigned int afrh8 : 4;
	unsigned int afrh9 : 4;
	unsigned int afrh10 : 4;
	unsigned int afrh11 : 4;
	unsigned int afrh12 : 4;
	unsigned int afrh13 : 4;
	unsigned int afrh14 : 4;
	unsigned int afrh15 : 4;
} volatile * const gpiob_afrh = (void *)0x40020424;

struct s_gpioa_moder {
	unsigned int moder0 : 2;
	unsigned int moder1 : 2;
	unsigned int moder2 : 2;
	unsigned int moder3 : 2;
	unsigned int moder4 : 2;
	unsigned int moder5 : 2;
	unsigned int moder6 : 2;
	unsigned int moder7 : 2;
	unsigned int moder8 : 2;
	unsigned int moder9 : 2;
	unsigned int moder10 : 2;
	unsigned int moder11 : 2;
	unsigned int moder12 : 2;
	unsigned int moder13 : 2;
	unsigned int moder14 : 2;
	unsigned int moder15 : 2;
} volatile * const gpioa_moder = (void *)0x40020000;

struct s_usart3_sr {
	unsigned int pe : 1;
	unsigned int fe : 1;
	unsigned int nf : 1;
	unsigned int ore : 1;
	unsigned int idle : 1;
	unsigned int rxne : 1;
	unsigned int tc : 1;
	unsigned int txe : 1;
	unsigned int lbd : 1;
	unsigned int cts : 1;
	unsigned int : 22;
} volatile * const usart3_sr = (void *)0x40004800;

struct s_usart3_dr {
	unsigned int dr : 9;
	unsigned int : 23;
} volatile * const usart3_dr = (void *)0x40004804;

struct s_usart3_brr {
	unsigned int div_fraction : 4;
	unsigned int div_mantissa : 12;
	unsigned int : 16;
} volatile * const usart3_brr = (void *)0x40004808;

struct s_usart3_cr1 {
	unsigned int sbk : 1;
	unsigned int rwu : 1;
	unsigned int re : 1;
	unsigned int te : 1;
	unsigned int idleie : 1;
	unsigned int rxneie : 1;
	unsigned int tcie : 1;
	unsigned int txeie : 1;
	unsigned int peie : 1;
	unsigned int ps : 1;
	unsigned int pce : 1;
	unsigned int wake : 1;
	unsigned int m : 1;
	unsigned int ue : 1;
	unsigned int : 1;
	unsigned int over8 : 1;
	unsigned int : 16;
} volatile * const usart3_cr1 = (void *)0x4000480c;

struct s_usart3_cr3 {
	unsigned int eie : 1;
	unsigned int iren : 1;
	unsigned int irlp : 1;
	unsigned int hdsel : 1;
	unsigned int nack : 1;
	unsigned int scen : 1;
	unsigned int dmar : 1;
	unsigned int dmat : 1;
	unsigned int rtse : 1;
	unsigned int ctse : 1;
	unsigned int ctsie : 1;
	unsigned int onebit : 1;
	unsigned int : 20;
} volatile * const usart3_cr3 = (void *)0x40004814;

struct s_c_adc_ccr {
	unsigned int mult : 5;
	unsigned int : 3;
	unsigned int delay : 4;
	unsigned int : 1;
	unsigned int dds : 1;
	unsigned int dma : 2;
	unsigned int adcpre : 2;
	unsigned int : 4;
	unsigned int vbate : 1;
	unsigned int tsvrefe : 1;
	unsigned int : 8;
} volatile * const c_adc_ccr = (void *)0x40012304;

struct s_tim1_cr1 {
	unsigned int cen : 1;
	unsigned int udis : 1;
	unsigned int urs : 1;
	unsigned int opm : 1;
	unsigned int dir : 1;
	unsigned int cms : 2;
	unsigned int arpe : 1;
	unsigned int ckd : 2;
	unsigned int : 22;
} volatile * const tim1_cr1 = (void *)0x40010000;

struct s_tim1_cr2 {
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
	unsigned int : 17;
} volatile * const tim1_cr2 = (void *)0x40010004;

struct s_tim1_sr {
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
	unsigned int : 19;
} volatile * const tim1_sr = (void *)0x40010010;

struct s_tim1_ccmr1_input {
	unsigned int cc1s : 2;
	unsigned int icpcs : 2;
	unsigned int ic1f : 4;
	unsigned int cc2s : 2;
	unsigned int ic2pcs : 2;
	unsigned int ic2f : 4;
	unsigned int : 16;
} volatile * const tim1_ccmr1_input = (void *)0x40010018;

struct s_tim1_ccer {
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
	unsigned int : 18;
} volatile * const tim1_ccer = (void *)0x40010020;

struct s_tim1_cnt {
	unsigned int cnt : 16;
	unsigned int : 16;
} volatile * const tim1_cnt = (void *)0x40010024;

struct s_tim1_ccr1 {
	unsigned int ccr1 : 16;
	unsigned int : 16;
} volatile * const tim1_ccr1 = (void *)0x40010034;

struct s_tim2_cr1 {
	unsigned int cen : 1;
	unsigned int udis : 1;
	unsigned int urs : 1;
	unsigned int opm : 1;
	unsigned int dir : 1;
	unsigned int cms : 2;
	unsigned int arpe : 1;
	unsigned int ckd : 2;
	unsigned int : 22;
} volatile * const tim2_cr1 = (void *)0x40000000;

struct s_tim2_smcr {
	unsigned int sms : 3;
	unsigned int : 1;
	unsigned int ts : 3;
	unsigned int msm : 1;
	unsigned int etf : 4;
	unsigned int etps : 2;
	unsigned int ece : 1;
	unsigned int etp : 1;
	unsigned int : 16;
} volatile * const tim2_smcr = (void *)0x40000008;

struct s_tim2_cnt {
	unsigned int cnt_l : 16;
	unsigned int cnt_h : 16;
} volatile * const tim2_cnt = (void *)0x40000024;

struct s_flash_acr {
	unsigned int latency : 3;
	unsigned int : 5;
	unsigned int prften : 1;
	unsigned int icen : 1;
	unsigned int dcen : 1;
	unsigned int icrst : 1;
	unsigned int dcrst : 1;
	unsigned int : 19;
} volatile * const flash_acr = (void *)0x40023c00;

