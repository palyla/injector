#include "config.h"

#include <stdio.h>
#include <avr/interrupt.h>

#include "FreeRTOS.h" /* Must come first. */
#include "FreeRTOSConfig.h"

#include "task.h"     /* RTOS task related API prototypes. */
#include "queue.h"    /* RTOS queue related API prototypes. */
#include "timers.h"   /* Software timer related API prototypes. */
#include "semphr.h"   /* Semaphore related API prototypes. */

#include "io/uart.h"

#if defined _DEBUG_SERIAL
    #define fatal(fmt, ...) do { printf("%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__); } while (1)
#elif defined _DEBUG_DISPLAY
    #define fatal(fmt, ...)
#else
    #define fatal(fmt, ...)
#endif

#define _ERR_MSG_NO_HEAP    "Insufficient heap"
#define _ERR_MSG_QUEUE_FULL "Queue is full"

#define TASK_DEFAULT_PRIORITY 255
#define TASK_DEFAULT_STACK_SIZE 256
// #define TIMER_INTERVAL_MS 250
#define TIMER_INTERVAL_MS 1000
#define MUTEX_WAIT_TICKS 10


static const char* data_msg =  "{"                                    \
                                   "\"params_t\":"                    \
                                   "{"                                \
                                       "\"fPathKm\":"          "%f,"  \
                                       "\"fFuelLit\":"         "%f,"  \
                                       "\"fFuelRub\":"         "%f,"  \
                                       "\"fAirflowCel\":"      "%f,"  \
                                       "\"fEngineCel\":"       "%f,"  \
                                       "\"fSpeedKmPerHr\":"    "%f,"  \
                                       "\"fFuelLitPerHr\":"    "%f,"  \
                                       "\"fFuelRubPerHr\":"    "%f,"  \
                                       "\"fFuelLitPer100Km\":" "%f,"  \
                                       "\"uEngineRpm\":"       "%d"   \
                                   "},"                               \
                                   "\"stats_t\":"                     \
                                   "{"                                \
                                       "\"fFuelLastWeekLit\":"  "%f," \
                                       "\"fFuelLastMonthLit\":" "%f," \
                                       "\"xPit\":" "\"%s\""           \
                                   "}"                                \
                               "}\n";

static const char* integrity_msg =  "{"                         \
                                        "\"integrity\":"        \
                                        "{"                     \
                                            "\"crc16\":" "0x%x" \
                                        "}"                     \
                                    "}\n";



TimerHandle_t xTimerInterval = NULL;
TaskHandle_t xHandleTaskDisplay = NULL;
TaskHandle_t xHandleTaskSerial = NULL;
SemaphoreHandle_t xSemData;

params_t xParams;
params_t xParamsCopy;
stats_t xStats;
stats_t xStatsCopy;


static inline void vInterruptInjector(void);
static inline void vInterruptWheel(void);
void vTaskDisplay(void * pvParameters);
void vTaskSerial(void * pvParameters);
static void vTiksReset(void);


/* 1 tick == 64 micros */
static volatile uint64_t ullTicksInjector = 0;
static volatile uint64_t ullTicksWheel = 0;


ISR(INT0_vect) { vInterruptWheel(); }
ISR(INT1_vect) { vInterruptInjector(); }



static void vEvaluate(void) {
    BaseType_t xReturned;
    float fPassPathM = 0.0;
    float fPassPathKm = 0.0;
    float fPassFuelMl = 0.0;
    float fPassFuelLit = 0.0;
    float fPassFuelRub = 0.0;
    float fElapsedM = 0.0; /* The sum of durations while an injector in open state [Minutes] */

    xReturned = xSemaphoreTake(xSemData, (TickType_t) MUTEX_WAIT_TICKS);
    if(xReturned != pdTRUE)
        taskYIELD();

    fElapsedM = ((float)ullTicksInjector * TIMER1_TICK_US / (1000.0 * 1000.0 * 60.0)) * INJECTORS;
    fPassFuelMl = (fElapsedM * VOLUMETRIC_FLOW_MILLILITERS_IN_MINUTE);

    fPassFuelLit = fPassFuelMl / 1000.0;
    fPassFuelRub = fPassFuelLit * FUEL_COST_RUB_L;

    xParams.fFuelLitPerHr = fPassFuelMl * 3.6;
    xParams.fFuelRubPerHr = xParams.fFuelLitPerHr * FUEL_COST_RUB_L;

    if (ullTicksWheel > TICKS_PER_WHEEL_REVOLUTION) {
        fPassPathM = ((ullTicksWheel / TICKS_PER_WHEEL_REVOLUTION) * METERS_PER_WHEEL_REVOLUTION);
        fPassPathKm = fPassPathM / 1000.0;

        xParams.fPathKm += fPassPathKm;
        xParams.fFuelLitPer100Km = (fPassFuelLit * 1000.0) / fPassPathM;
        xParams.fSpeedKmPerHr = fPassPathKm * 0.000278;
    } else {
        xParams.fFuelLitPer100Km = 0.0;
        xParams.fSpeedKmPerHr = 0.0;
    }

    xParams.fFuelLit += fPassFuelLit;
    xParams.fFuelRub += fPassFuelRub;
    xParams.fAirflowCel = 0.0;
    xParams.fEngineCel = 0.0;
    xParams.uEngineRpm = 0.0;

    xReturned = xSemaphoreGive(xSemData);
    if(xReturned == pdFALSE) {
        fatal("xSemData:xSemaphoreGive: " _ERR_MSG_QUEUE_FULL)
    }
}


void vTimerIntervalCallback(TimerHandle_t xTimer) {
    uint64_t ullTicksInjectorCopy;
    uint64_t ullTicksWheelCopy;

    taskENTER_CRITICAL();
    ullTicksInjectorCopy = ullTicksInjector;
    ullTicksWheelCopy = ullTicksWheel;
    vTiksReset();
    taskEXIT_CRITICAL();

    vEvaluate();
}


static inline void vInterruptInjector(void) {
    taskENTER_CRITICAL_FROM_ISR();

    if (PIND & (1 << PD3)) {
        /* Prescaler 1024 and 16MHZ frequncy [64 micros ... 4.19424 secs] */
        TCCR1B |= (1 << CS12) | (1 << CS10);
    } else {
        ullTicksInjector += TCNT1;
        TCNT1 = 0x0;
        TCCR1B = 0x0;
    }
    taskEXIT_CRITICAL_FROM_ISR();
}

static inline void vInterruptWheel(void) {
    taskENTER_CRITICAL_FROM_ISR();

    ullTicksWheel += 1;
    
    taskEXIT_CRITICAL_FROM_ISR();
}

static void vTiksReset(void) {
    ullTicksInjector = 0;
    ullTicksWheel = 0;
}

static void vInitInterruptInjector(void) {
    EICRA |= (1 << ISC10);
    EIMSK |= (1 << INT1);
}

static void vInitInterruptWheel(void) {
    EICRA |= (1 << ISC00) | (1 << ISC01);
    EIMSK |= (1 << INT0);
}

static void vInitHW(void) {
    vInitUART();
    // lcd_init();
    vInitInterruptInjector();
    vInitInterruptWheel();
}

static void vInitOS(void) {
    BaseType_t xReturned;

    xSemData = xSemaphoreCreateMutex();
    if(xSemData == NULL) {
        fatal("xSemData: " _ERR_MSG_NO_HEAP);
    }

    xTimerInterval = xTimerCreate(
        "xTimerInterval", 
        pdMS_TO_TICKS(TIMER_INTERVAL_MS), 
        pdTRUE, 
        (void*) NULL, 
        vTimerIntervalCallback
    );

    if(xTimerInterval == NULL) {
        fatal("xTimerInterval: " _ERR_MSG_NO_HEAP);
    }

    xReturned = xTaskCreate(
        vTaskDisplay, 
        "vTaskDisplay", 
        TASK_DEFAULT_STACK_SIZE, 
        (void *) NULL, 
        TASK_DEFAULT_PRIORITY, 
        &xHandleTaskDisplay
    );
    if(xReturned != pdPASS) {
        fatal("vTaskDisplay: " _ERR_MSG_NO_HEAP);
    }

    xReturned = xTaskCreate(
        vTaskSerial, 
        "vTaskSerial", 
        TASK_DEFAULT_STACK_SIZE, 
        (void *) NULL, 
        TASK_DEFAULT_PRIORITY, 
        &xHandleTaskSerial
    );
    if(xReturned != pdPASS) {
        fatal("vTaskSerial: " _ERR_MSG_NO_HEAP);
    }

    xReturned = xTimerStart(xTimerInterval, 0);
    if(xReturned != pdPASS) {
        fatal("xTimerInterval: " _ERR_MSG_NO_HEAP);
    }
}

void vTaskDisplay(void * pvParameters) {
    while(1) {
        // printf("1\n");
    }
}

void vTaskSerial(void * pvParameters) {
    BaseType_t xReturned;

    while(1) {
        xReturned = xSemaphoreTake(xSemData, (TickType_t) MUTEX_WAIT_TICKS);
        if(xReturned != pdTRUE)
            taskYIELD();
        
        xParamsCopy = xParams;
        xStatsCopy = xStats;

        xReturned = xSemaphoreGive(xSemData);
        if(xReturned == pdFALSE) {
            fatal("xSemData:xSemaphoreGive: " _ERR_MSG_QUEUE_FULL)
        }

        printf(
            data_msg,
            xParamsCopy.fPathKm,
            xParamsCopy.fFuelLit,
            xParamsCopy.fFuelRub,
            xParamsCopy.fAirflowCel,
            xParamsCopy.fEngineCel,
            xParamsCopy.fSpeedKmPerHr,
            xParamsCopy.fFuelLitPerHr,
            xParamsCopy.fFuelRubPerHr,
            xParamsCopy.fFuelLitPer100Km,
            xParamsCopy.uEngineRpm,
            xStatsCopy.fFuelLastWeekLit,
            xStatsCopy.fFuelLastMonthLit,
            "xPit"
        );

    }
}


int main(void) {
    vInitHW();
    vInitOS();
    vTaskStartScheduler();
    
    /* We shouldn't be here */
    fatal("Insufficient RAM");
}
