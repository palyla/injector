#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <inttypes.h>

#include "uart.h"

volatile uint64_t micros = 0;
volatile uint64_t secs = 0;


void timer1_init(void) {
    OCR1A = 0x3D08;
    
    TCCR1B |= (1 << WGM12);
    // Mode 4, CTC on OCR1A

    TIMSK1 |= (1 << OCIE1A);
    //Set interrupt on compare match

    TCCR1B |= (1 << CS12) | (1 << CS10);
    // set prescaler to 1024 and start the timer
}

void timer2_init(void) {
	OCR2A = 248;

    TCCR2A |= (1 << WGM21);
    // Set to CTC Mode

    TIMSK2 |= (1 << OCIE2A);
    //Set interrupt on compare match

    TCCR2B |= (1 << CS21);
    // set prescaler to 64 and starts PWM
}

int main(void) {
    uart_init();
    stdout = &uart_output;
    stdin  = &uart_input;


	timer1_init();
	timer2_init();


    sei();
    // enable interrupts


    while (1) {
        // we have a working Timer
    }
}

ISR (TIMER1_COMPA_vect) {
    // action to be done every 1 sec
	secs += 1;
	// printf("Seconds passed: %lu\n", secs);
	// printf("Micross passed: %lu\n", micros);
}

ISR (TIMER2_COMPA_vect)
{
	micros += 1;
    // action to be done every 1 micro sec
}
