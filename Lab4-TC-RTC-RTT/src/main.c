#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

#define BUT_1_PIO PIOD
#define BUT_1_PIO_ID ID_PIOD
#define BUT_1_IDX 28
#define BUT_1_IDX_MASK (1u << BUT_1_IDX)

#define LED_PIO PIOC
#define LED_PIO_ID ID_PIOC
#define LED_PIO_IDX 8
#define LED_IDX_MASK (1 << LED_PIO_IDX)

#define LED_1_PIO PIOA
#define LED_1_PIO_ID ID_PIOA
#define LED_1_IDX 0
#define LED_1_IDX_MASK (1u << LED_1_IDX)

#define LED_2_PIO PIOC
#define LED_2_PIO_ID ID_PIOC
#define LED_2_IDX 30
#define LED_2_IDX_MASK (1u << LED_2_IDX)

#define LED_3_PIO PIOB
#define LED_3_PIO_ID ID_PIOB
#define LED_3_IDX 2
#define LED_3_IDX_MASK (1u << LED_3_IDX)

typedef struct  {
	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t week;
	uint32_t hour;
	uint32_t minute;
	uint32_t second;
} calendar;

#define PIO_IT_EDGE             (1u << 6)

/************************************************************************/
/* VAR globais                                                          */
/************************************************************************/
volatile char counter = 0;
volatile char but_1_flag;
volatile char flag_rtc_alarm = 0;
volatile Bool f_rtt_alarme = false;

/************************************************************************/
/* PROTOTYPES                                                           */
/************************************************************************/

void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq);
static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses);
void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type);
void pin_toggle(Pio *pio, uint32_t mask);
void but_1_callback(void);
void pisca_led (int n, int t);
void init();

/************************************************************************/
/* Handlers                                                             */
/************************************************************************/

void TC1_Handler(void) {
	volatile uint32_t status = tc_get_status(TC0, 1);
	pin_toggle(LED_PIO, LED_IDX_MASK);
}

void TC4_Handler(void) {
	volatile uint32_t status = tc_get_status(TC1, 1);
	pin_toggle(LED_1_PIO, LED_1_IDX_MASK);
}
void RTT_Handler(void)
{
	counter ++;
	uint32_t ul_status;

	/* Get RTT status - ACK */
	ul_status = rtt_get_status(RTT);

	if (counter % 4 == 0){
		/* IRQ due to Time has changed */
		if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
			//f_rtt_alarme = false;
			pin_toggle(LED_2_PIO, LED_2_IDX_MASK);    // BLINK Led
			
		}

		/* IRQ due to Alarm */
		if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
			// pin_toggle(LED_PIO, LED_IDX_MASK);    // BLINK Led
			f_rtt_alarme = true;                  // flag RTT alarme
		}
		counter = 0;
	}
}

void RTC_Handler(void) {
	uint32_t ul_status = rtc_get_status(RTC);
	
	/* seccond tick */
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		// o código para irq de segundo vem aqui
	}
	
	/* Time or date alarm */
	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
		// o código para irq de alame vem aqui
		flag_rtc_alarm = 1;
	}

	rtc_clear_status(RTC, RTC_SCCR_SECCLR);
	rtc_clear_status(RTC, RTC_SCCR_ALRCLR);
	rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
	rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
	rtc_clear_status(RTC, RTC_SCCR_CALCLR);
	rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
}

/************************************************************************/
/* Funcoes                                                              */
/************************************************************************/

void pin_toggle(Pio *pio, uint32_t mask){
	if(pio_get_output_data_status(pio, mask))
	pio_clear(pio, mask);
	else
	pio_set(pio,mask);
}

void init(void)
{
	sysclk_init();

	// Desativa WatchDog Timer
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	// Ativa o PIO na qual o LED foi conectado
	// para que possamos controlar o LED.
	pmc_enable_periph_clk(LED_PIO_ID);
	pmc_enable_periph_clk(LED_1_PIO_ID);
	pmc_enable_periph_clk(LED_2_PIO_ID);
	pmc_enable_periph_clk(LED_3_PIO_ID);
	
	//Inicializa como saída
	pio_set_output(LED_PIO, LED_IDX_MASK, 1, 0, 0);
	pio_set_output(LED_1_PIO, LED_1_IDX_MASK, 1, 0, 0);
	pio_set_output(LED_2_PIO, LED_2_IDX_MASK, 1, 0, 0);
	pio_set_output(LED_3_PIO, LED_3_IDX_MASK, 1, 0, 0);
	
	pmc_enable_periph_clk(BUT_1_PIO_ID);
	pio_configure(BUT_1_PIO, PIO_INPUT, BUT_1_IDX_MASK, PIO_PULLUP);
	
	pio_handler_set(BUT_1_PIO,
	BUT_1_PIO_ID,
	BUT_1_IDX_MASK,
	PIO_IT_EDGE,
	but_1_callback);
	
	pio_enable_interrupt(BUT_1_PIO, BUT_1_IDX_MASK);
	pio_get_interrupt_status(BUT_1_PIO);
	
	NVIC_EnableIRQ(BUT_1_PIO_ID);
	NVIC_SetPriority(BUT_1_PIO_ID, 4);

}
static float get_time_rtt(){
	uint ul_previous_time = rtt_read_timer_value(RTT);
}

void but_1_callback(void)
{
	if (pio_get(BUT_1_PIO, PIO_INPUT, BUT_1_IDX_MASK)) {
		// PINO == 1 --> Borda de subida
		but_1_flag = 1;
		} else {
		// PINO == 0 --> Borda de descida
	}
}

/************************************************************************/
/*Inits	                                                                */
/************************************************************************/

void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq){
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	/* Configura o PMC */
	/* O TimerCounter ? meio confuso
	o uC possui 3 TCs, cada TC possui 3 canais
	TC0 : ID_TC0, ID_TC1, ID_TC2
	TC1 : ID_TC3, ID_TC4, ID_TC5
	TC2 : ID_TC6, ID_TC7, ID_TC8
	*/
	pmc_enable_periph_clk(ID_TC);

	/** Configura o TC para operar em  freq hz e interrup?c?o no RC compare */
	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	/* Configura e ativa interrup?c?o no TC canal 0 */
	/* Interrup??o no C */
  NVIC_SetPriority(ID_TC, 4);
	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);

	/* Inicializa o canal 0 do TC */
	tc_start(TC, TC_CHANNEL);
}

static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses)
{
	uint32_t ul_previous_time;

	/* Configure RTT for a 1 second tick interrupt */
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	ul_previous_time = rtt_read_timer_value(RTT);
	while (ul_previous_time == rtt_read_timer_value(RTT));
	
	rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);

	/* Enable RTT interrupt */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 4);
	NVIC_EnableIRQ(RTT_IRQn);
	rtt_enable_interrupt(RTT, RTT_MR_ALMIEN | RTT_MR_RTTINCIEN);
}


void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type){
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(rtc, 0);

	/* Configura data e hora manualmente */
	rtc_set_date(rtc, t.year, t.month, t.day, t.week);
	rtc_set_time(rtc, t.hour, t.minute, t.second);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(id_rtc);
	NVIC_ClearPendingIRQ(id_rtc);
	NVIC_SetPriority(id_rtc, 4);
	NVIC_EnableIRQ(id_rtc);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(rtc,  irq_type);
}

/************************************************************************/
/* Main Code	                                                        */
/************************************************************************/

uint32_t ul_hour, ul_minute, ul_second;
int main (void)
{
	board_init();
	sysclk_init();
	delay_init();
	init();

	/* Disable the watchdog */
	WDT->WDT_MR = WDT_MR_WDDIS;

	TC_init(TC0, ID_TC1, 1, 5);
	tc_start(TC0, 1);
	
	TC_init(TC1, ID_TC4, 1, 4);
	tc_start(TC1, 1);
	
	calendar rtc_initial = {2018, 3, 19, 12, 15, 45 ,1};
	RTC_init(RTC, ID_RTC, rtc_initial, RTC_IER_ALREN);
	
	rtc_set_date_alarm(RTC, 1, rtc_initial.month, 1, rtc_initial.day);
	rtc_set_time_alarm(RTC, 1, rtc_initial.hour, 1, rtc_initial.minute, 1, rtc_initial.second + 20);
	
	
	// Init OLED
	gfx_mono_ssd1306_init();	  
	  // Inicializa RTT com IRQ no alarme.
	f_rtt_alarme = true;
	uint32_t ul_hour, ul_minute, ul_second;
	rtc_get_time(RTC, &ul_hour, &ul_minute, &ul_second);
	
	while (1) {
		
		if (f_rtt_alarme){
      
		  /*
		   * IRQ (interrup??o ocorre) apos 4s => 4 pulsos por sengundo (0,25s) -> 16 pulsos s?o necess?rios para dar 4s
		   * tempo[s] = 0,25 * 16 = 4s
		   */
			uint16_t pllPreScale = (int) (((float) 32768) / 1);
			uint32_t irqRTTvalue = 16;
      
		  // reinicia RTT para gerar um novo IRQ
			RTT_init(pllPreScale, irqRTTvalue);         
      
			f_rtt_alarme = false;
		}
		if (but_1_flag){
			if(flag_rtc_alarm){
				
				rtc_set_time_alarm(RTC, 1, ul_hour, 1 , ul_minute, 1 ,ul_second + 1);
				
				pin_toggle(5, 200);
				flag_rtc_alarm = 0;
				but_1_flag = 0;
			}
			
			
		}
	
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}

  
}
