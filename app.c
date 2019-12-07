#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <inttypes.h>
#include <avr-nokia5110/nokia5110.h>
#include <avr/eeprom.h>
#include <string.h>

#include "uart.h"

#define TIMER1_TICK_US 64.0
#define TIMER2_TICKS_EQ_A_SECOND 61

// #define MILLILITERS_IN_MINUTE 191.8 // This is the old incorrect value
#define MILLILITERS_IN_MINUTE 213.9
#define REGINA_MILLILITERS_IN_MINUTE 176.0
// #define LITTERS_IN_HOUR_FROM_MILLILITERS_IN_MINUTE(X) ((X * 60.0) / MILLILITERS_IN_MINUTE)
// #define LITTERS_IN_HOUR_FROM_MILLILITERS_IN_MINUTE(X) ((X * 60.0) / 1000)

#define INJECTORS 4
#define TICKS_PER_WHEEL_REVOLUTION 8
#define METERS_PER_WHEEL_REVOLUTION 2

/* 1 tick == 64 micros */
static volatile uint64_t timer1_ticks = 0;
/* 90 ticks == 1 secs */
static volatile uint64_t timer2_ticks = 0;
static volatile uint64_t wheel_ticks = 0;


static double elapsed_m = 0.0; /* The sum of an open injector times */
static double spent_ml = 0.0; /* Spent fuel in milliliters */
static double spent_l = 0.0;
static double spent_ml_h = 0.0;
static double path_m = 0.0;
static double path_km = 0.0;
static double cons_l_km = 0.0;
static double speed_km_h = 0.0;
static double spent_total_once_l = 0.0;
static double spent_total_l = 0.0;
static double path_total_once_km = 0.0;
static double path_total_km = 0.0;


ISR(TIMER2_OVF_vect) {
    timer2_ticks += 1;
}


ISR(INT0_vect) {
    wheel_ticks += 1;
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

void injector_pin_init(void) {
    EICRA |= (1 << ISC10);
    EIMSK |= (1 << INT1);
}

void wheel_pin_init(void) {
    EICRA |= (1 << ISC00);
    EIMSK |= (1 << INT0);
}

void timer2_init(void) {
    TIMSK2 |= (1 << TOIE2);
    TIFR2 |= (1 << TOV2);
    TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
}

void lcd_init(void) {
    nokia_lcd_init();
    nokia_lcd_clear();
    nokia_lcd_write_string("Volvo", 1);
    nokia_lcd_set_cursor(0, 10);
    nokia_lcd_write_string("- one love!", 1);
    nokia_lcd_set_cursor(0, 20);
    nokia_lcd_render();
}

void init(void) {
    uart_init();
    lcd_init();
    injector_pin_init();
    wheel_pin_init();
    timer2_init();
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

void reset_ticks(void) {
    timer1_ticks = 0;
    timer2_ticks = 0;
    wheel_ticks = 0;
}

void wait(int seconds) {
    timer2_ticks = 0;
    while (seconds--) {
        while (timer2_ticks < TIMER2_TICKS_EQ_A_SECOND)
            ;
    }
}

void evaluate(void) {
    elapsed_m = ((double)timer1_ticks * TIMER1_TICK_US / (1000.0 * 1000.0 * 60.0)) * INJECTORS;
    spent_ml = elapsed_m * MILLILITERS_IN_MINUTE;
    spent_l = spent_ml / 1000.0;    
    spent_ml_h = (spent_ml * 60.0) / elapsed_m;

    path_m = (wheel_ticks / TICKS_PER_WHEEL_REVOLUTION) * METERS_PER_WHEEL_REVOLUTION;
    path_km = path_m / 1000.0;
    cons_l_km = (spent_l * 1000.0) / path_m; /* per 100 km */

    speed_km_h = (path_km / 0.000278);

    spent_total_once_l += spent_l;
    spent_total_l += spent_l; /* TODO store in the EEPROM */
    path_total_once_km += path_km;
    path_total_km += path_km; /* TODO store in the EEPROM */
}

void uart_present_conf(void) {
    printf("*******************************************************\n");
    printf("*                     SETTINGS                        *\n");
    printf("*******************************************************\n\n");
    printf("TIMER1_TICK_US=%f\n", TIMER1_TICK_US);
    printf("TIMER2_TICKS_EQ_A_SECOND=%d\n", TIMER2_TICKS_EQ_A_SECOND);
    printf("MILLILITERS_IN_MINUTE=%f\n", MILLILITERS_IN_MINUTE);
    printf("INJECTORS=%d\n", INJECTORS);
    printf("TICKS_PER_WHEEL_REVOLUTION=%d\n", TICKS_PER_WHEEL_REVOLUTION);
    printf("METERS_PER_WHEEL_REVOLUTION=%d\n", METERS_PER_WHEEL_REVOLUTION);
}

void uart_present(void) {
    uart_putchar((char)0x1B, stdout);
    printf("-------------------------------------------------------\n");
    printf("Injector in state open %f minutes\n", elapsed_m);
    printf("Spent %f ML/H\n", spent_ml_h);
    printf("Consumption %f L/100KM\n", cons_l_km);
    printf("Speed %f KM/H\n", speed_km_h);
    printf(">>>>>>>>>>>>>>>>>>>>>> This trip >>>>>>>>>>>>>>>>>>>>>>\n");
    printf("Spent %f L\n", spent_total_once_l);
    printf("Kilometrage %f KM\n", path_total_once_km);
    printf("<<<<<<<<<<<<<<<<<<<<<<<< TOTAL <<<<<<<<<<<<<<<<<<<<<<<<\n");
    printf("Spent total %f L\n", spent_total_l);
    printf("Kilometrage total %f KM\n", path_total_km);
    printf("-------------------------------------------------------\n");
}

void lcd_present(void) {
    nokia_lcd_clear();
    
    lprintf(0, 0, "CS %.4f L/KM", cons_l_km);
    lprintf(0, 10, "S %.1f KM/H", speed_km_h);
    lprintf(0, 20, "SP %.4f L", spent_total_once_l);
    lprintf(0, 30, "P %.4f KM", path_total_once_km);

    nokia_lcd_render();
}

int main(void) {
    init();
    uart_present_conf();
    sei();
    while (1) {
        reset_ticks();
        wait(1);
        evaluate();
        uart_present();
        lcd_present();
    }
}
