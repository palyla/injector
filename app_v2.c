#include "config.h"

#include <stdio.h>

#include "FreeRTOS.h" /* Must come first. */
#include "FreeRTOSConfig.h"

#include "task.h"     /* RTOS task related API prototypes. */
#include "queue.h"    /* RTOS queue related API prototypes. */
#include "timers.h"   /* Software timer related API prototypes. */
#include "semphr.h"   /* Semaphore related API prototypes. */

#include "io/uart.h"

#if defined _DEBUG_SERIAL_
	#define fatal(fmt, ...) \
	        do { fprintf(stdout, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); } while (1)
#elif defined _DEBUG_DISPLAY_
	#define fatal(fmt, ...)
#else
	#define fatal(fmt, ...)
#endif

#define _ERR_MSG_NO_HEAP "Insufficient FreeRTOS heap"

#define TASK_DEFAULT_PRIORITY 255
#define TASK_DEFAULT_STACK_SIZE 64
#define TIMER_INTERVAL_MS 250

TimerHandle_t xTimerInterval = NULL;

TaskHandle_t xHandleTaskDisplay = NULL;
TaskHandle_t xHandleTaskSerial = NULL;
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
}

void vTaskDisplay(void * pvParameters) {
    while(1) {
	    printf("1\n");
    }
}

void vTaskSerial(void * pvParameters) {
    while(1) {
	    printf("2\n");
    }
}



int main(void) {

	vInitHW();
	vInitOS();

	xReturned = xTimerStart(xTimerInterval, 0);
	if(xReturned != pdPASS) {
	    fatal("xTimerInterval: " _ERR_MSG_NO_HEAP);
	}
	
	vTaskStartScheduler();
    
    /* We shouldn't be here */
    fatal("Insufficient RAM");
}
