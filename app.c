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

#ifdef NOKIA_5110_LCD
    #include "nokia5110_lcd.h"
#endif


typedef struct {
    float path_km;   /* Way over current trip [Kilometers] */
    float fuel_l;    /* Fuel spent over current trip [Liters] */
    float fuel_rub;  /* Money spent for fuel over current trip [Rubles] */
} params_t;

typedef struct {
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

static inline void init(void) {
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
    float path_m = 0.0;
    float path_km = 0.0;
    float fuel_ml = 0.0;
    float fuel_l = 0.0;
    float fuel_rub = 0.0;
    float elapsed_m = 0.0; /* The sum of durations while an injector in open state [Minutes] */

    elapsed_m = ((float)timer1_ticks * TIMER1_TICK_US / (1000.0 * 1000.0 * 60.0)) * INJECTORS;
    fuel_ml = (elapsed_m * VOLUMETRIC_FLOW_MILLILITERS_IN_MINUTE);

    fuel_l = fuel_ml / 1000.0;
    fuel_rub = fuel_l * FUEL_COST_RUB_L;

    forecast.fuel_l_h = fuel_ml * 3.6;
    forecast.fuel_rub_h = forecast.fuel_l_h * FUEL_COST_RUB_L;

    if (wheel_ticks > TICKS_PER_WHEEL_REVOLUTION) {
        path_m = ((wheel_ticks / TICKS_PER_WHEEL_REVOLUTION) * METERS_PER_WHEEL_REVOLUTION);
        path_km = path_m / 1000.0;

        forecast.fuel_l_km = (fuel_l * 1000.0) / path_m;
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

#if _USING_UART_PRESENT_CONFIG
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
#endif

#if _USING_UART_PRESENT
static void uart_present(void) {
    printf("-------------------------------------------------------\n");
    printf("FUEL %f L/H\n", forecast.fuel_l_h);
    printf("FUEL %f L/100KM\n", forecast.fuel_l_km);
    printf("SPEED %f KM/H\n\n", forecast.speed_km_h);

    printf("FUEL %f L\n", trip.fuel_l);
    printf("PATH %f KM\n\n", trip.path_km);

    printf("TOTAL FUEL %f L\n", total.fuel_l);
    printf("TOTAL PATH %f KM\n", total.path_km);
    printf("RUB %.2f\n", total.fuel_rub);
    printf("-------------------------------------------------------\n");
}
#endif

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
    
    lprintf(0, 0, "%.4f KM", trip.path_km);
    lprintf(0, 10, "%.4f L/KM", forecast.fuel_l_km);
    lprintf(0, 20, "%.4f L/H", forecast.fuel_l_h);
    lprintf(0, 30, "T %.4f L", trip.fuel_l);
    lprintf(0, 40, "RUB %.2f", trip.fuel_rub);

    lcd_render();
}

#if _USING_EEPROM
static void eeprom_try_save(void) {
    if (!eeprom_is_ready())
        return ;

    eeprom_write_block((const void *)&total, (void *)EEPROM_DATA_OFFSET, sizeof(params_t));
}

static void eeprom_load(void) {
    eeprom_busy_wait();
    eeprom_read_block((void *)&total, (const void *)EEPROM_DATA_OFFSET, sizeof(params_t));
}
#endif

int main(void) {
    init();

    #if _USING_UART_PRESENT_CONFIG
    uart_present_conf();
    #endif

    #if _USING_USING_EEPROM
    eeprom_load();
    #endif
    
    sei();
    while (1) {
        reset_ticks();
        wait(1);
        evaluate();

        #if _USING_EEPROM
        eeprom_try_save();
        #endif

        #if _USING_UART_PRESENT
        uart_present();
        #endif

        lcd_present();
    }
}
