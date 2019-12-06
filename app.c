#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <inttypes.h>
#include <avr-nokia5110/nokia5110.h>

#include "uart.h"

#define TIMER1_TICK_US 64.0
#define TIMER2_TICKS_EQ_A_SECOND 61

#define MILLILITERS_IN_MINUTE 191.8
#define LITTERS_IN_HOUR_FROM_MILLILITERS_IN_MINUTE(X) ((X * 60.0) / MILLILITERS_IN_MINUTE)


/* 1 tick == 64 micros */
static volatile uint64_t timer1_ticks = 0;
/* 90 ticks == 1 secs */
static volatile uint64_t timer2_ticks = 0;


ISR(TIMER2_OVF_vect) {
	timer2_ticks += 1;
}


ISR(INT1_vect) {
	if (PIND & (1 << PD3)) {
	    /* Prescaler 1024 and 16MHZ frequncy [64 micros ...  secs] */
	    TCCR1B |= (1 << CS12) | (1 << CS10);
	} else {
		timer1_ticks += TCNT1;
		TCNT1 = 0x0;
		TCCR1B = 0x0;
	}
}

void pin_init(void) {
    EICRA |= (1 << ISC10);
    EIMSK |= (1 << INT1);
}

void lcd_init(void) {
    nokia_lcd_init();
    nokia_lcd_clear();
    nokia_lcd_write_string("kwas love!", 1);
    nokia_lcd_write_string("one love!", 1);
    nokia_lcd_set_cursor(0, 10);
    nokia_lcd_set_cursor(0, 20);
    nokia_lcd_render();
}

void lprintf(int x, int y, const char *fmt, ...) {
	char buffer[1024];
    
    va_list args;
    va_start(args, fmt);

    nokia_lcd_set_cursor(x, y);

	memset(buffer, 0, 1024);
    vsprintf(buffer, fmt, args);
    nokia_lcd_write_string(buffer, 1);

    va_end(args);
}


int main(void) {
	double elapsed_m = 0; /* The sum of an open injector times */
	double spent_ml = 0; /* Spent fuel in milliliters */
	double spent_l = 0;
	double spent_total_l = 0;

	uart_init();
	stdout = &uart_output;
	stdin  = &uart_input;

	lcd_init();

	pin_init();

    TIMSK2 |= (1 << TOIE2);
    TIFR2 |= (1 << TOV2);
    TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);

    sei();
    while (1) {
		timer1_ticks = 0;
	    timer2_ticks = 0;
	    while (timer2_ticks < TIMER2_TICKS_EQ_A_SECOND)
	    	;

		elapsed_m = (double)timer1_ticks * TIMER1_TICK_US / (1000.0 * 1000.0 * 60.0);
		spent_ml = elapsed_m * MILLILITERS_IN_MINUTE;
		spent_l = LITTERS_IN_HOUR_FROM_MILLILITERS_IN_MINUTE(spent_ml);
		spent_total_l += spent_l * 0.000278;

		printf("---------------------------------------------\n");
		printf("MILLILITERS_IN_MINUTE %f \n", MILLILITERS_IN_MINUTE);
		printf("Elapsed %f minutes\n", elapsed_m);
		printf("Spent %f mililiters/minute\n", spent_ml);
		printf("Spent %f litters/hour\n", spent_l);

		nokia_lcd_clear();

	    lprintf(0, 0, "EL %.4f M", elapsed_m);
	    lprintf(0, 10, "SP %.4f ML/M", spent_ml);
	    lprintf(0, 20, "SP %.4f L/H", spent_l);
	    lprintf(0, 30, "T %.4f L", spent_total_l);

	    nokia_lcd_render();
	}
}
