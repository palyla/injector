#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <inttypes.h>

#include "uart.h"

#define TIMER1_TICK_US 64.0
#define TIMER2_TICKS_EQ_A_SECOND 61

#define MILLILITERS_IN_MINUTE 191.8
#define LITTERS_IN_HOUR_FROM_MILLILITERS_IN_MINUTE(X) ((X * 60.0) / MILLILITERS_IN_MINUTE)


// 1 tick == 64 micros
static volatile uint64_t timer1_ticks = 0;
// 90 times overflow == 1 secs
static volatile uint64_t timer2_ticks = 0;


ISR(TIMER2_OVF_vect) {
	// printf("timer2\n");
	timer2_ticks += 1;
}


ISR(INT1_vect) {
	if (PIND & (1 << PD3)) {
	    /* Prescaler 1024 and 16MHZ frequncy [64 micros ...  secs] */
	    TCCR1B |= (1 << CS12) | (1 << CS10);
	} else {
		// printf("%f\n", TCNT1 * TIMER1_TICK_US);
		timer1_ticks += TCNT1;
		TCNT1 = 0x0;
		TCCR1B = 0x0;
	}
}

void pin_init(void) {
    EICRA |= (1 << ISC10);
    EIMSK |= (1 << INT1);
}

// 3.2 millititers in secound
int main(void) {
	double elapsed_m = 0; /* elapsed microsecounds */
	double spent_ml = 0; /* milliliters in microsecound */


	uart_init();
	stdout = &uart_output;
	stdin  = &uart_input;

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
		
		printf("---------------------------------------------\n");
		printf("MILLILITERS_IN_MINUTE %f \n", MILLILITERS_IN_MINUTE);
		printf("Elapsed %f minutes\n", elapsed_m);
		printf("Spent %f mililiters/minute\n", spent_ml);
		printf("Spent %f litters/hour\n", LITTERS_IN_HOUR_FROM_MILLILITERS_IN_MINUTE(spent_ml));
	}
}
