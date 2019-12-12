#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <inttypes.h>
#include <avr/eeprom.h>
#include <string.h>

#include "uart.h"
#include "config.h"

/* 1 tick == 64 micros */
static volatile uint64_t timer1_ticks = 0;
/* 90 ticks == 1 secs */
static volatile uint64_t timer2_ticks = 0;
static volatile uint64_t wheel_ticks = 0;

static char buffer[LPRINTF_BUFFER_SIZE];

static double elapsed_m = 0.0; /* The sum of an open injector times */
static double spent_ml = 0.0; /* Spent fuel in milliliters */
static double spent_l = 0.0;
static double spent_l_h = 0.0;
static double path_m = 0.0;
static double path_km = 0.0;
static double cons_l_km = 0.0;
static double speed_km_h = 0.0;
static double spent_total_once_l = 0.0;
static double spent_total_once_rub = 0.0;
static double spent_total_l = 0.0;
static double path_total_once_km = 0.0;
static double path_total_km = 0.0;

static double* spent_eeprom_ptr = (double*)EEPROM_DATA_OFFSET;
static double* path_eeprom_ptr = (double*)(EEPROM_DATA_OFFSET + sizeof(double));


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
    EICRA |= (1 << ISC00) | (1 << ISC01);
    EIMSK |= (1 << INT0);
}

void timer2_init(void) {
    TIMSK2 |= (1 << TOIE2);
    TIFR2 |= (1 << TOV2);
    TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
}

void init(void) {
    uart_init();
    lcd_init();
    injector_pin_init();
    wheel_pin_init();
    timer2_init();
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
    spent_ml = elapsed_m * VOLUMETRIC_FLOW_MILLILITERS_IN_MINUTE; // Spent until 1 second 
    spent_l = spent_ml / 1000.0;
    spent_l_h = spent_ml * 3.6;

    if (wheel_ticks > TICKS_PER_WHEEL_REVOLUTION) {
        path_m = (wheel_ticks / TICKS_PER_WHEEL_REVOLUTION) * METERS_PER_WHEEL_REVOLUTION;
        path_km = path_m / 1000.0;
        cons_l_km = (spent_l * 1000.0) / path_m; /* per 100 km */ /* TODO Path may be 0! */

        speed_km_h = (path_km / 0.000278);
    } else {
        cons_l_km = 0;
        speed_km_h = 0;
    }
    spent_total_once_rub = spent_total_once_l * FUEL_COST_L;

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
    printf("VOLUMETRIC_FLOW_MILLILITERS_IN_MINUTE=%f\n", VOLUMETRIC_FLOW_MILLILITERS_IN_MINUTE);
    printf("INJECTORS=%d\n", INJECTORS);
    printf("TICKS_PER_WHEEL_REVOLUTION=%d\n", TICKS_PER_WHEEL_REVOLUTION);
    printf("METERS_PER_WHEEL_REVOLUTION=%d\n", METERS_PER_WHEEL_REVOLUTION);
}

void uart_present(void) {
    printf("-------------------------------------------------------\n");
    printf("Injector in state open %f minutes\n", elapsed_m);
    printf("Spent %f ML/sec\n", spent_ml);
    printf("Spent %f L/H\n", spent_l_h);
    printf("Consumption %f L/100KM\n", cons_l_km);
    printf("Speed %f KM/H\n", speed_km_h);
    printf(">>>>>>>>>>>>>>>>>>>>>> This trip >>>>>>>>>>>>>>>>>>>>>>\n");
    printf("Spent %f L\n", spent_total_once_l);
    printf("Kilometrage %f KM\n", path_total_once_km);
    printf("<<<<<<<<<<<<<<<<<<<<<<<< TOTAL <<<<<<<<<<<<<<<<<<<<<<<<\n");
    printf("Spent total %f L\n", spent_total_l);
    printf("Kilometrage total %f KM\n", path_total_km);
    printf("RUB %.2f\n", spent_total_once_rub);
    printf("-------------------------------------------------------\n");
}

void lprintf(int x, int y, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    lcd_set_cursor(x, y);

    memset(buffer, 0, LPRINTF_BUFFER_SIZE);
    vsprintf(buffer, fmt, args);

    lcd_write_string(buffer);

    va_end(args);
}

void lcd_present(void) {
    lcd_clear();
    
    // lprintf(0, 0, "%.1f KM/H", speed_km_h);
    lprintf(0, 0, "%.4f KM", path_total_once_km);
    lprintf(0, 10, "%.4f L/KM", cons_l_km);
    lprintf(0, 20, "%.4f L/H", spent_l_h);
    lprintf(0, 30, "T %.4f L", spent_total_once_l);
    lprintf(0, 40, "RUB %.2f", spent_total_once_rub);
    // lprintf(0, 40, "TF %.4f L", spent_total_l);
    // lprintf(0, 30, "P %.4f KM", path_total_once_km);

    lcd_render();
}

void eeprom_save(void) {
    if (eeprom_is_ready()) {
        eeprom_write_float((float*)&spent_eeprom_ptr, spent_total_l);
        eeprom_write_float((float*)&path_eeprom_ptr, path_total_km);
    }
}

void eeprom_load(void) {
    eeprom_busy_wait();
    spent_total_l = eeprom_read_float((float*)spent_eeprom_ptr);
    eeprom_busy_wait();
    path_total_km = eeprom_read_float((float*)path_eeprom_ptr);
}

int main(void) {
    init();
    uart_present_conf();
    // eeprom_load();
    sei();
    while (1) {
        reset_ticks();
        wait(1);
        evaluate();
        // eeprom_save();
        uart_present();
        lcd_present();
    }
}
