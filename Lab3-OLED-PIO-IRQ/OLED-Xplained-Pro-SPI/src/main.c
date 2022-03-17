#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

#define BUT_1_PIO PIOD
#define BUT_1_PIO_ID ID_PIOD
#define BUT_1_IDX 28
#define BUT_1_IDX_MASK (1u << BUT_1_IDX)

#define BUT_2_PIO PIOC
#define BUT_2_PIO_ID ID_PIOC
#define BUT_2_IDX 31
#define BUT_2_IDX_MASK (1u << BUT_2_IDX)

#define BUT_3_PIO PIOA
#define BUT_3_PIO_ID ID_PIOA
#define BUT_3_IDX 19
#define BUT_3_IDX_MASK (1u << BUT_3_IDX)

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

volatile char but_1_flag;
volatile char but_2_flag;
volatile char but_2_flag_;
volatile char but_3_flag;

void but_1_callback(void)
{
	if (pio_get(BUT_1_PIO, PIO_INPUT, BUT_1_IDX_MASK)) {
		// PINO == 1 --> Borda de subida
		but_1_flag = 1;
		} else {
		// PINO == 0 --> Borda de descida
	}
}

void but_2_callback(void)
{
	if (pio_get(BUT_2_PIO, PIO_INPUT, BUT_2_IDX_MASK)) {
		// PINO == 1 --> Borda de subida
		but_2_flag = 1;
		but_2_flag_ =1;
		} else {
		// PINO == 0 --> Borda de descida
	}
}

void but_3_callback(void)
{
	if (pio_get(BUT_3_PIO, PIO_INPUT, BUT_3_IDX_MASK)) {
		// PINO == 1 --> Borda de subida
		but_3_flag = 1;
		} else {
		// PINO == 0 --> Borda de descida
	}
}

void pisca_led(int n, int t){
	for (int i=0;i<n;i++){
		pio_clear(LED_1_PIO, LED_1_IDX_MASK);
		delay_ms(t);
		pio_set(LED_1_PIO, LED_1_IDX_MASK);
		delay_ms(t);
	}
}

void init(void)
{
	sysclk_init();

	// Desativa WatchDog Timer
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	// Ativa o PIO na qual o LED foi conectado
	// para que possamos controlar o LED.
	pmc_enable_periph_clk(LED_1_PIO_ID);
	pmc_enable_periph_clk(LED_2_PIO_ID);
	pmc_enable_periph_clk(LED_3_PIO_ID);
	
	//Inicializa como saída
	pio_set_output(LED_1_PIO, LED_1_IDX_MASK, 1, 0, 0);
	pio_set_output(LED_2_PIO, LED_2_IDX_MASK, 1, 0, 0);
	pio_set_output(LED_3_PIO, LED_3_IDX_MASK, 1, 0, 0);
	
	// Inicializa PIO do botao
	pmc_enable_periph_clk(BUT_1_PIO_ID);
	pmc_enable_periph_clk(BUT_2_PIO_ID);
	pmc_enable_periph_clk(BUT_3_PIO_ID);
	
	// configura pino ligado ao botão como entrada com um pull-up.
	pio_configure(BUT_1_PIO, PIO_INPUT, BUT_1_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT_2_PIO, PIO_INPUT, BUT_2_IDX_MASK, PIO_PULLUP);
	pio_configure(BUT_3_PIO, PIO_INPUT, BUT_3_IDX_MASK, PIO_PULLUP);
	
	pio_handler_set(BUT_1_PIO,
	BUT_1_PIO_ID,
	BUT_1_IDX_MASK,
	PIO_IT_EDGE,
	but_1_callback);
	
	pio_handler_set(BUT_2_PIO,
	BUT_2_PIO_ID,
	BUT_2_IDX_MASK,
	PIO_IT_EDGE,
	but_2_callback);
	
	pio_handler_set(BUT_3_PIO,
	BUT_3_PIO_ID,
	BUT_3_IDX_MASK,
	PIO_IT_EDGE,
	but_3_callback);
	
	pio_enable_interrupt(BUT_1_PIO, BUT_1_IDX_MASK);
	pio_get_interrupt_status(BUT_1_PIO);
	
	pio_enable_interrupt(BUT_2_PIO, BUT_2_IDX_MASK);
	pio_get_interrupt_status(BUT_2_PIO);
	
	pio_enable_interrupt(BUT_3_PIO, BUT_3_IDX_MASK);
	pio_get_interrupt_status(BUT_3_PIO);
	
	NVIC_EnableIRQ(BUT_1_PIO_ID);
	NVIC_SetPriority(BUT_1_PIO_ID, 4);
	
	NVIC_EnableIRQ(BUT_2_PIO_ID);
	NVIC_SetPriority(BUT_2_PIO_ID, 4);
	
	NVIC_EnableIRQ(BUT_3_PIO_ID);
	NVIC_SetPriority(BUT_3_PIO_ID, 4);

}


int main (void)
{
	init();
	board_init();
	sysclk_init();
	delay_init();
	int delay = 500;

	// Desativa watchdog
	WDT->WDT_MR = WDT_MR_WDDIS;

  // Init OLED
	gfx_mono_ssd1306_init();
  
	while(1) {
		
		if(but_1_flag){
			char str[128]; //
			sprintf(str, "%d", delay); //
			gfx_mono_draw_string(str, 0, 0, &sysfont);
			pisca_led(3,delay);
			but_1_flag = 0;
		}
		
		if(but_2_flag){
			delay -= 100;
			char str[128]; //
			sprintf(str, "%d", delay); //
			gfx_mono_draw_string(str, 0, 0, &sysfont);
			pisca_led(3,delay);
			but_2_flag = 0;
			but_2_flag_ = 0;
		}
		
		while(but_2_flag_){
			delay += 100;
			char str[128]; //
			sprintf(str, "%d", delay); //
			gfx_mono_draw_string(str, 0, 0, &sysfont);
			pisca_led(3,delay);
			but_2_flag_ = 0;
			
		}
		
		if(but_3_flag){
			pio_set(LED_1_PIO, LED_1_IDX_MASK);
			but_3_flag = 0;
			but_1_flag = 0;
			but_2_flag = 0;
			but_2_flag_ = 0;
			
		}
		
		
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);

	}
}
