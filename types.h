#ifndef __INJECTOR_TYPES_H__
#define __INJECTOR_TYPES_H__

#include <time.h>

#define ecc_sizeof(type) ((ecc_size_t)((sizeof(type)/2) + sizeof(type)))

typedef size_t ecc_size_t;

typedef struct {
    float path_km;   	  /* Way over current trip [Kilometers] */
    float fuel_l;    	  /* Fuel spent over current trip [Liters] */
    float fuel_rub;  	  /* Money spent for fuel over current trip [Rubles] */
    float airflow_temp_c; /* The input air flow temperature [Celsius] */
    float engine_temp_c;  /* The engine temperature [Celsius] */
} params_t;

typedef struct {
    float    speed_km_h; /* Forecast speed [Kilometers\Hour] */
    float    fuel_l_h;   /* Forecast fuel consumption [Liter\Hour] */
    float    fuel_rub_h; /* Forecast fuel consumption [Rubles\Hour] */
    float    fuel_l_km;  /* Forecast fuel consumption [Liter\100 Km] */
    uint16_t engine_rpm; /* Forecast engine rpms [Rpm] */
} forecast_t;

typedef struct {
    float fuel_week_l; 	/* Spent fuel over the last week */
    float fuel_month_l; /* Spent fuel over the last month */
    time_t pit; 		/* Point in time */
} stats_t;

/* incremental type */
/* re-calculate type */

#endif /* __INJECTOR_TYPES_H__ */
