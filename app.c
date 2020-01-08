#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <inttypes.h>
#include <avr/eeprom.h>
#include <string.h>

#include <micro-ecc/uECC.h>
#define uECC_PLATFORM uECC_avr

#include "uart.h"
#include "config.h"


#typedef struct {
    float path_km;   /* Way over current trip [Kilometers] */
    float fuel_l;    /* Fuel spent over current trip [Liters] */
    float fuel_rub;  /* Money spent for fuel over current trip [Rubles] */
} params_t;  

#typedef struct {
    float speed_km_h; /* Forecast speed [Kilometers\Hour] */
    float fuel_l_h;   /* Forecast fuel consumption [Liter\Hour] */
    float fuel_rub_h;  /* Forecast fuel consumption [Rubles\Hour] */
    float fuel_l_km;  /* Forecast fuel consumption [Liter\100 Km] */
} forecast_t;

#if 0
#typedef struct {
    float _; /*  */
    float _; /*  */
    float _; /*  */
} stats_t;
#endif


/* 1 tick == 64 micros */
static volatile uint64_t timer1_ticks = 0;
/* 61 ticks == 1 secs */
static volatile uint64_t timer2_ticks = 0;
static volatile uint64_t wheel_ticks = 0;

static params_t total;
static params_t trip;
static forecast_t forecast;

static char buffer[LPRINTF_BUFFER_SIZE];

// static float elapsed_m = 0.0; /* The sum of an open injector times */
// static float spent_ml = 0.0; /* Spent fuel in milliliters */
// static float spent_l = 0.0;
// static float spent_l_h = 0.0;
// static float path_m = 0.0;
// static float path_km = 0.0;
// static float cons_l_km = 0.0;
// static float speed_km_h = 0.0;
// static float spent_total_once_l = 0.0;
// static float spent_total_once_rub = 0.0;
// static float spent_total_rub = 0.0;
// static float spent_total_l = 0.0;
// static float path_total_once_km = 0.0;
// static float path_total_km = 0.0;


ISR(TIMER2_OVF_vect) {
    timer2_ticks += 1;
}


ISR(INT0_vect) {
    wheel_ticks += 1;
}


ISR(INT1_vect) {
    if (PIND & (1 << PD3)) {
        /* Prescaler 1024 and 16MHZ frequncy [64 micros ... 4.19424 secs] */
        TCCR1B |= (1 << CS12) | (1 << CS10);
    } else {
        timer1_ticks += TCNT1;
        TCNT1 = 0x0;
        TCCR1B = 0x0;
    }
}

static void injector_pin_init(void) {
    EICRA |= (1 << ISC10);
    EIMSK |= (1 << INT1);
}

static void wheel_pin_init(void) {
    EICRA |= (1 << ISC00) | (1 << ISC01);
    EIMSK |= (1 << INT0);
}

static void timer2_init(void) {
    TIMSK2 |= (1 << TOIE2);
    TIFR2 |= (1 << TOV2);
    TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);
}

static void init(void) {
    uart_init();
    lcd_init();
    injector_pin_init();
    wheel_pin_init();
    timer2_init();
}

static void reset_ticks(void) {
    timer1_ticks = 0;
    timer2_ticks = 0;
    wheel_ticks = 0;
}

static void wait(int seconds) {
    timer2_ticks = 0;
    while (seconds--) {
        while (timer2_ticks < TIMER2_TICKS_EQ_A_SECOND)
            ;
    }
}

static void evaluate(void) {
    float spent_ml = 0.0;
    float path_m = 0.0;
    float path_km = 0.0;
    float fuel_l = 0.0;
    float fuel_rub = 0.0;
    float elapsed_m = 0.0; /* The sum of durations while an injector in open state [Minutes] */

    elapsed_m = ((float)timer1_ticks * TIMER1_TICK_US / (1000.0 * 1000.0 * 60.0)) * INJECTORS;
    spent_ml = (trip.elapsed_m * VOLUMETRIC_FLOW_MILLILITERS_IN_MINUTE);

    fuel_l = spent_ml / 1000.0;
    fuel_rub = trip.fuel_l * FUEL_COST_RUB_L;

    forecast.fuel_l_h = spent_ml * 3.6;
    forecast.fuel_rub_h = forecast.fuel_l_h * FUEL_COST_RUB_L;

    if (wheel_ticks > TICKS_PER_WHEEL_REVOLUTION) {
        path_m = ((wheel_ticks / TICKS_PER_WHEEL_REVOLUTION) * METERS_PER_WHEEL_REVOLUTION);
        path_km = path_m / 1000.0;

        forecast.fuel_l_km = (spent_l * 1000.0) / path_m;
        forecast.speed_km_h = path_km * 0.000278;

    } else {
        forecast.fuel_l_km = 0.0;
        forecast.speed_km_h = 0.0;
    }

    trip.path_km += path_km;
    trip.fuel_l += fuel_l;
    trip.fuel_rub += fuel_rub;
    
    total.path_km += path_km;
    total.fuel_l += fuel_l;
    total.fuel_rub += fuel_rub;
}

static void uart_present_conf(void) {
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

static void uart_present(void) {
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

static void lprintf(int x, int y, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    lcd_set_cursor(x, y);

    memset(buffer, 0, LPRINTF_BUFFER_SIZE);
    vsprintf(buffer, fmt, args);

    lcd_write_string(buffer);

    va_end(args);
}

static void lcd_present(void) {
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

static void eeprom_try_save(void) {
    if (eeprom_is_ready()) {
        eeprom_write_float(EEPROM_TOTAL_PATH_OFFSET, path_total_km);
        eeprom_write_float(EEPROM_TOTAL_SPENT_L_OFFSET, spent_total_l);
        eeprom_write_float(EEPROM_TOTAL_SPENT_RUB_OFFSET, spent_total_rub);
    }
}

static void eeprom_load(void) {
    eeprom_busy_wait();
    path_total_km = eeprom_read_float(EEPROM_TOTAL_PATH_OFFSET);
    spent_total_l = eeprom_read_float(EEPROM_TOTAL_SPENT_L_OFFSET);
    spent_total_rub = eeprom_read_float(EEPROM_TOTAL_SPENT_RUB_OFFSET);
}

int main(void) {
    init();
    uart_present_conf();
    eeprom_load();
    sei();
    while (1) {
        reset_ticks();
        wait(1);
        evaluate();
        eeprom_try_save();
        uart_present();
        lcd_present();
    }
}
