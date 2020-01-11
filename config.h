#ifndef __INJECTOR_CONFIG_H__
#define __INJECTOR_CONFIG_H__

#include "types.h"


/* Memory options */
#define EEPROM_DATA_OFFSET   0x0
#define EEPROM_TOTAL_OFFSET  (EEPROM_DATA_OFFSET)
#define EEPROM_STATS_OFFSET  (EEPROM_TOTAL_OFFSET + ecc_sizeof(params_t))

/* Timer tick cost */
#define TIMER1_TICK_US 64.0
#define TIMER2_TICKS_EQ_A_SECOND 61

/* Evaluation constants */
#define VOLUMETRIC_FLOW_MILLILITERS_IN_MINUTE 185.0 /* 170.0 for volvo 740 regina */
#define INJECTORS 4
#define TICKS_PER_WHEEL_REVOLUTION 12
#define METERS_PER_WHEEL_REVOLUTION 2
#define FUEL_COST_RUB_L 42.50

/* Features */
#define T6963C_LCD /* NOKIA_5110_LCD, T6963C_LCD */
#define _USING_UART_PRESENT_CONFIG 1
#define _USING_UART_PRESENT 1
#define _USING_EEPROM 1
#define _USING_EEPROM_ECC 1


#endif /* __INJECTOR_CONFIG_H__ */
