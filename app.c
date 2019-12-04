#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <inttypes.h>

#include "uart.h"
#include "timers.h"

static volatile uint64_t micros = 0;


ISR(PCINT2_vect) {
    // if this happens it MUST be PD2 as it's the only 
    // interrupt I enabled in PCMSK2 !
	if (PIND & (1 << PD2)) {
	    TCCR1B |= (1 << CS12) | (1 << CS10);
	} else {
		printf("%u\n", TCNT1);
		TCNT1 = 0x0;
	}

}


void pin_init(void) {
	
    PCICR = (1 << PCIE2);
    PCMSK2 = (1 << PD2);
}

int main(void) {
	uart_init();
	stdout = &uart_output;
	stdin  = &uart_input;


	// timer1_init();
	// timer2_init();
	pin_init();

    sei();
    // enable interrupts

    while (1)
        ;
}
