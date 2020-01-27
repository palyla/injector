#ifndef __INJECTOR_TYPES_H__
#define __INJECTOR_TYPES_H__

#include <time.h>

#define ecc_sizeof(type) ((ecc_size_t)((sizeof(type)/2) + sizeof(type)))

typedef size_t EccSize_t;

typedef struct {
    float    fPathKm;          /* Way over current trip [Kilometers] */
    float    fFuelLit;    	   /* Fuel spent over current trip [Liters] */
    float    fFuelRub;  	   /* Money spent for fuel over current trip [Rubles] */
    float    fAirflowCel;      /* The input air flow temperature [Celsius] */
    float    fEngineCel;       /* The engine temperature [Celsius] */
    float    fSpeedKmPerHr;    /* Forecast speed [Kilometers\Hour] */
    float    fFuelLitPerHr;    /* Forecast fuel consumption [Liter\Hour] */
    float    fFuelRubPerHr;    /* Forecast fuel consumption [Rubles\Hour] */
    float    fFuelLitPer100Km; /* Forecast fuel consumption [Liter\100 Km] */
    uint16_t uEngineRpm;       /* Forecast engine rpms [Rpm] */
} Params_t;

typedef struct {
    float fFuelLastWeekLit;  /* Spent fuel over the last week */
    float fFuelLastMonthLit; /* Spent fuel over the last month */
    time_t xPit; 		     /* Point in time */
} Stats_t;

/* incremental type */
/* re-calculate type */

#endif /* __INJECTOR_TYPES_H__ */
