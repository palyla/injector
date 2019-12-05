#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <inttypes.h>

#include "uart.h"
#include "timers.h"

static volatile uint64_t micros = 0;


// 100 times overflow == 1 secs
static volatile uint64_t ticks = 0;

 



ISR(TIMER2_OVF) {
	ticks += 1;
}
 

ISR(INT1_vect) {
	if (PIND & (1 << PD3)) {
	    // Prescaler 1024 and 16MHZ frequncy [46 micros ... 3 secs]
	    TCCR1B |= (1 << CS12) | (1 << CS10);
	} else {
		// printf("%u\n", TCNT1);
		micros += TCNT1;
		TCNT1 = 0x0;
	}

}


void pin_init(void) {
	
    // PCICR = (1 << PCIE2);
    // PCMSK2 = (1 << PD3);
    EICRA |= (1 << ISC10);
    EIMSK |= (1 << INT1);
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
