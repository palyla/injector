#include "config.h"

#include <stdio.h>

#include "FreeRTOS.h" /* Must come first. */
#include "FreeRTOSConfig.h"

#include "task.h"     /* RTOS task related API prototypes. */
#include "queue.h"    /* RTOS queue related API prototypes. */
#include "timers.h"   /* Software timer related API prototypes. */
#include "semphr.h"   /* Semaphore related API prototypes. */

#include "io/uart.h"

TaskHandle_t xHandleMain = NULL;
TaskHandle_t xHandleMain2 = NULL;






void vTaskMain(void * pvParameters)
{
	BaseType_t xReturned;


    while(1) {
	    printf("1\n");
    }
}
void vTaskMain2(void * pvParameters)
{
	BaseType_t xReturned;

    while(1) {
	    printf("2\n");
    }
}



int main(void) {
	BaseType_t xReturned;
	uart_init();


    xReturned = xTaskCreate(vTaskMain, "vTaskMain", 128, ( void * ) NULL, 1, &xHandleMain);
    xReturned = xTaskCreate(vTaskMain2, "vTaskMain2", 128, ( void * ) NULL, 1, &xHandleMain2);
    if(xReturned == pdPASS)
    {
        //vTaskDelete(xHandle);
    	vTaskStartScheduler();
    }

    while (1)
    	;
}
