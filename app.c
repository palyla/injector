#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <inttypes.h>

#include "uart.h"

#define TIMER1_TICK_US 46
#define TIMER2_TICKS_EQ_A_SECOND 100


#define FUEL_ML_SPENT_US 0.0000032

#define MILLILITERS_IN_MINUTE 191.8
#define MILLILITERS_IN_MICROS (MILLILITERS_IN_MINUTE/(60*1000*2))

#define LITTERS_IN_HOUR ((MILLILITERS_IN_MINUTE*60) / 1000)
#define LITTERS_IN_HOUR_FROM_MILLILITERS_IN_MICROS(X) ((X*1000*2*60*60)/1000)



// 1 tick == 46 micros
static volatile uint64_t timer1_ticks = 0;
// 100 times overflow == 1 secs
static volatile uint64_t timer2_ticks = 0;


ISR(TIMER2_OVF) {
	timer2_ticks += 1;
}


ISR(INT1_vect) {
	if (PIND & (1 << PD3)) {
	    // Prescaler 1024 and 16MHZ frequncy [46 micros ... 3 secs]
	    TCCR1B |= (1 << CS12) | (1 << CS10);
	} else {
		// printf("%u\n", TCNT1);
		timer1_ticks += TCNT1;
		TCNT1 = 0x0;
	}
}

void pin_init(void) {
    EICRA |= (1 << ISC10);
    EIMSK |= (1 << INT1);
}

uint64_t spent_fuel(uint64_t opened_us) {
	uint64_t opened_us = timer1_ticks * TIMER1_TICK_US;


	return ;
}

// 3.2 millititers in secound
int main(void) {
	uint64_t spent_ml = 0; /* milliliters in microsecound */


	uart_init();
	stdout = &uart_output;
	stdin  = &uart_input;

	pin_init();

    TIMSK2 |= (1 << TOIE2);
    TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);

    sei();
    while (1) {
	    
	    while (timer2_ticks < TIMER2_TICKS_EQ_A_SECOND)
	    	;

		spent_ml = timer1_ticks * TIMER1_TICK_US * MILLILITERS_IN_MICROS;
		printf("%u\n", LITTERS_IN_HOUR_FROM_MILLILITERS_IN_MICROS(spent_ml));

		timer1_ticks = 0;
	    timer2_ticks = 0;
	}
}
