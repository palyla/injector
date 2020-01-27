#include "config.h"

#include <stdio.h>

#include "FreeRTOS.h" /* Must come first. */
#include "FreeRTOSConfig.h"

#include "task.h"     /* RTOS task related API prototypes. */
#include "queue.h"    /* RTOS queue related API prototypes. */
#include "timers.h"   /* Software timer related API prototypes. */
#include "semphr.h"   /* Semaphore related API prototypes. */

#include "io/uart.h"

#define TASK_DEFAULT_PRIORITY 255
#define TASK_DEFAULT_STACK_SIZE 64
#define TIMER_INTERVAL_MS 250

TimerHandle_t xTimerInterval = NULL;

TaskHandle_t xHandleMain = NULL;
TaskHandle_t xHandleMain2 = NULL;
BaseType_t xReturned = NULL;


static inline void vInterruptInjector(void);
static inline void vInterruptWheel(void);


/* 1 tick == 64 micros */
static volatile uint64_t ullTicksInjector = 0;
static volatile uint64_t ullTicksWheel = 0;


ISR(INT0_vect) { vInterruptWheel(); }
ISR(INT1_vect) { vInterruptInjector(); }


void vTimerIntervalCallback(TimerHandle_t xTimer) {
	vTiksReset();
	
	// TODO calculate parameters
}


static inline void vInterruptInjector(void) {
    if (PIND & (1 << PD3)) {
        /* Prescaler 1024 and 16MHZ frequncy [64 micros ... 4.19424 secs] */
        TCCR1B |= (1 << CS12) | (1 << CS10);
    } else {
        ullTicksInjector += TCNT1;
        TCNT1 = 0x0;
        TCCR1B = 0x0;
    }
}

static inline void vInterruptWheel(void) {
	ullTicksWheel += 1;
}

static void vTiksReset(void) {
	ullTicksInjector = 0;
	ullTicksWheel = 0;
}

static void vInterruptInitInjector(void) {
    EICRA |= (1 << ISC10);
    EIMSK |= (1 << INT1);
}

static void vInterruptInitWheel(void) {
    EICRA |= (1 << ISC00) | (1 << ISC01);
    EIMSK |= (1 << INT0);
}

static void vInitHW(void) {
    uart_init(); /* vInitUART */
    // lcd_init();
    vInterruptInitInjector();
    vInterruptInitWheel();
}

static void vInitOS(void) {
	xTimerInterval = xTimerCreate(
		"xTimerInterval", 
		pdMS_TO_TICKS(TIMER_INTERVAL_MS), 
		pdTRUE, 
		(void*) NULL, 
		vTimerIntervalCallback
	);

	if(xTimerInterval == NULL) {
		// TODO err
	}

    xReturned = xTaskCreate(vTaskMain, "vTaskMain", TASK_DEFAULT_STACK_SIZE, ( void * ) NULL, TASK_DEFAULT_PRIORITY, &xHandleMain);

    xReturned = xTaskCreate(vTaskMain2, "vTaskMain2", TASK_DEFAULT_STACK_SIZE, ( void * ) NULL, TASK_DEFAULT_PRIORITY, &xHandleMain2);
    if(xReturned == pdPASS)
    {
        //vTaskDelete(xHandle);
    	
    }
	

}

void vTaskMain(void * pvParameters)
{
    while(1) {
	    printf("1\n");
    }
}

void vTaskMain2(void * pvParameters)
{
    while(1) {
	    printf("2\n");
    }
}



int main(void) {

	vInitHW();
	vInitOS();


	xReturned = xTimerStart(xTimerInterval, 0);
	if(xReturned != pdPASS) {
		/* The timer could not be set into the Active
		state. */
		// TODO err
	}
	
	vTaskStartScheduler();
    
    while (1)
    	;
}
