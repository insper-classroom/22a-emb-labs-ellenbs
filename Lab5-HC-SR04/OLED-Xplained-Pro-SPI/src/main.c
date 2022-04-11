#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

#define TRIG_PIO PIOA
#define TRIG_PIO_ID ID_PIOD
#define TRIG_IDX 3
#define TRIG_IDX_MASK (1u << TRIG_IDX)

#define ECHO_PIO PIOA
#define ECHO_PIO_ID ID_PIOA
#define ECHO_IDX 4
#define ECHO_IDX_MASK (1u << ECHO_IDX)

#define PIO_IT_EDGE             (1u << 6)

volatile Bool f_rtt_alarme = false;
volatile uint32_t time = 0;

static void RTT_init(uint16_t pllPreScale, uint32_t IrqNPulses);
void pin_toggle(Pio *pio, uint32_t mask);

static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource) {

	uint16_t pllPreScale = (int) (((float) 32768) / freqPrescale);
	
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	if (rttIRQSource & RTT_MR_ALMIEN) {
		uint32_t ul_previous_time;
		ul_previous_time = rtt_read_timer_value(RTT);
		while (ul_previous_time == rtt_read_timer_value(RTT));
		rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);
	}

	/* config NVIC */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 4);
	NVIC_EnableIRQ(RTT_IRQn);

	/* Enable RTT interrupt */
	if (rttIRQSource & (RTT_MR_RTTINCIEN | RTT_MR_ALMIEN))
	rtt_enable_interrupt(RTT, rttIRQSource);
	else
	rtt_disable_interrupt(RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN);
	
}

uint32_t ul_hour, ul_minute, ul_second;

void echo_callback (void) {
		if(pio_get(ECHO_PIO, PIO_INPUT, ECHO_IDX_MASK)){
			uint32_t irqRTTvalue = 8000;
			
			// reinicia RTT para gerar um novo IRQ
			RTT_init(1, irqRTTvalue, 0);
		}
		else{
			time = rtt_read_timer_value(RTT);
		}	
}

void io_init(void){
	/* led */
	pmc_enable_periph_clk(TRIG_PIO_ID);
	pio_configure(TRIG_PIO, PIO_OUTPUT_0, ECHO_IDX_MASK, PIO_DEFAULT);
	
	pmc_enable_periph_clk(ECHO_PIO_ID);
	pio_configure(ECHO_PIO, PIO_INPUT, ECHO_IDX_MASK, PIO_DEFAULT);
	
	pio_handler_set(ECHO_PIO,
	ECHO_PIO_ID,
	ECHO_IDX_MASK,
	PIO_IT_EDGE,
	echo_callback);
	
	NVIC_EnableIRQ(ECHO_PIO_ID);
	pio_enable_interrupt(ECHO_PIO, ECHO_IDX_MASK);
	pio_get_interrupt_status(ECHO_PIO);
	NVIC_SetPriority(ECHO_PIO_ID, 4);
}

void pulse(void){
	pio_set(TRIG_PIO,TRIG_IDX_MASK);
	delay_ms(10);
}

float calcula_dist(){
	return ((float)time)*340/(2*8000);
	
}

int main (void)
{
	board_init();
	sysclk_init();
	delay_init();
	io_init();

  // Init OLED
	gfx_mono_ssd1306_init();
	char str[32];
	float dist;
  
  // Escreve na tela um circulo e um texto
    

	while(1) {
		pulse();
		gfx_mono_draw_string(str, 50,16, &sysfont);
		while (time==0){
		}
		dist = calcula_dist()*100;
		sprintf(str,"%f", dist);
		gfx_mono_draw_string(str, 50,16, &sysfont);
}
}
