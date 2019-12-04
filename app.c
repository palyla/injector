#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <inttypes.h>

#include "uart.h"
#include "timers.h"





int main(void) {
    uart_init();
    stdout = &uart_output;
    stdin  = &uart_input;


	timer1_init();
	timer2_init();


    sei();
    // enable interrupts


    while (1)
        ;
}

