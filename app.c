#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <inttypes.h>
#include <avr/eeprom.h>
#include <string.h>
#include <util/crc16.h>

#include <DL_Hamming/DL_Hamming.h>

#include "types.h"
#include "config.h"
#include "io/uart.h"
#include "io/lcd.h"


static const char* config_msg = "{"                                                        \
                                    "\"settings\":"                                        \
                                    "{"                                                    \
                                        "\"TIMER1_TICK_US\":"                        "%f," \
                                        "\"TIMER2_TICKS_EQ_A_SECOND\":"              "%d," \
                                        "\"VOLUMETRIC_FLOW_MILLILITERS_IN_MINUTE\":" "%f," \
                                        "\"INJECTORS\":"                             "%d," \
                                        "\"TICKS_PER_WHEEL_REVOLUTION\":"            "%f," \
                                        "\"METERS_PER_WHEEL_REVOLUTION\":"           "%f"  \
                                    "}"                                                    \
                                "}";


static const char* telemetry_msg =  "{"                                 \
                                        "\"data\":"                     \
                                        "{"                             \
                                            "\"fuel_l_h\":"       "%f," \
                                            "\"engine_rpm\":"     "%f," \
                                            "\"engine_temp_c\":"  "%f," \
                                            "\"airflow_temp_c\":" "%f"  \
                                        "}"                             \
                                    "}";

static const char* integrity_msg =  "{"                       \
                                        "\"integrity\":"      \
                                        "{"                   \
                                            "\"crc16\":" "%d" \
                                        "}"                   \
                                    "}";

static uint16_t telemetry_msg_crc16 = 0xFFFF;

/* 1 tick == 64 micros */
static volatile uint64_t timer1_ticks = 0;
/* 61 ticks == 1 secs */
static volatile uint64_t timer2_ticks = 0;
static volatile uint64_t wheel_ticks = 0;

static params_t total;
static params_t current;
static forecast_t forecast;
static stats_t stats;

static uint64_t ecc_corrections = 0;


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

    current.path_km += path_km;
    current.fuel_l += fuel_l;
    current.fuel_rub += fuel_rub;
    
    total.path_km += path_km;
    total.fuel_l += fuel_l;
    total.fuel_rub += fuel_rub;
}

static void lprintf(int x, int y, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    lcd_goto(x, y);
    vfprintf(&lcdout, fmt, args);
    va_end(args);
}

static void uart_send_telemetry(void) {
    printf(telemetry_msg, 
        forecast.fuel_l_h,
        forecast.engine_rpm,
        current.engine_temp_c,
        current.airflow_temp_c
    );
    
    telemetry_msg_crc16 = 0xFFFF;
    for(register int i = 0; i < sizeof(telemetry_msg); i++) {
        telemetry_msg_crc16 = _crc16_update(telemetry_msg_crc16, (uint8_t)telemetry_msg[i]);
    }

    printf(integrity_msg, telemetry_msg_crc16);

    return ;
}

static void lcd_present(void) {
    lcd_clear();
    
    lprintf(0, 0, "%.4f KM", current.path_km);
    lprintf(0, 10, "%.4f L/KM", forecast.fuel_l_km);
    lprintf(0, 20, "%.4f L/H", forecast.fuel_l_h);
    lprintf(0, 30, "T %.4f L", current.fuel_l);
    lprintf(0, 40, "RUB %.2f", current.fuel_rub);

    lcd_render();
}

#if _USING_EEPROM

static void ecc_error_handler(void) {
    while(1)
        printf("ECC failed, corections=%lld\n", ecc_corrections);
}

static size_t eeprom_try_save(uint8_t* src, uint8_t* dst, size_t sz) {
    size_t new_sz = sz;
    
    #if _USING_EEPROM_ECC

    uint8_t parity = 0;
    uint8_t first = 0;
    uint8_t second = 0;

    if(!eeprom_is_ready())
        return 0;

    for(register int i = 0; i < sz; i += 2) {
        first = *src++;
        second = *src++;
        parity = (uint8_t)DL_HammingCalculateParity2416(first, second);
        eeprom_write_byte(dst++, first);
        eeprom_write_byte(dst++, second);
        eeprom_write_byte(dst++, parity);
        new_sz++;
    }

    if(!eeprom_is_ready())
        return 0;
    eeprom_write_block((const void *)src, (void *)dst, new_sz);
    
    return new_sz;
}

static void eeprom_load(uint8_t* src, uint8_t* dst, ecc_size_t sz) {
    uint8_t parity = 0;
    uint8_t first = 0;
    uint8_t second = 0;
    uint8_t status = 0;

    eeprom_busy_wait();
    
    for(register int i = 0; i < sz; i += 3) {
        first = eeprom_read_byte(src++);
        second = eeprom_read_byte(src++);
        parity = eeprom_read_byte(src++);
        status = (uint8_t)DL_HammingCorrect2416((byte*)&first, (byte*)&second, (byte)parity);
        switch(status) {
            case ECC_OK: continue; break;
            case ECC_TWO_CORRECTIONS: ecc_corrections++;
            case ECC_ONE_CORRECTION: ecc_corrections++; break;
            case ECC_ERROR: ecc_error_handler(); break;
        }
        *dst++ = first;
        *dst++ = second;
    }
    
    eeprom_busy_wait();
    eeprom_read_block((void *)dst, (const void *)src, sizeof(params_t));
}


int main(void) {
    init();

    eeprom_load((uint8_t*)EEPROM_TOTAL_OFFSET, (uint8_t*)&total, ecc_sizeof(params_t));
    eeprom_load((uint8_t*)EEPROM_STATS_OFFSET, (uint8_t*)&stats, ecc_sizeof(stats_t));

    sei();
    while (1) {
        reset_ticks();
        wait(1);
        evaluate();

        eeprom_try_save((uint8_t*)&total, (uint8_t*)EEPROM_TOTAL_OFFSET, sizeof(params_t));
        eeprom_try_save((uint8_t*)&stats, (uint8_t*)EEPROM_STATS_OFFSET, sizeof(stats_t));

        uart_send_telemetry();
        lcd_present();
    }
}
