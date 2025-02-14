/* 
    In this demo, we use a timer to create a delay between output messages.
    First, the delay is 1 second, and after each print, it is increased by 1.
    In order to make it easier, we have defined a maximum number of reperition, 
    but the same code, could be written for an undefined number of repetition of print.
*/

/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* Limit the number of print */
#define MAX_REPS 7

/* Set a fixed delay */
#define DELAY 1000

/* Definition of the timer used to delay output messages */
TimerHandle_t xTimer;

/* Repetitions counter used to limit the number of messages printed */
int repetitions = 0;

/* Used to compute the current delay */
int currentDelay = DELAY;

/* 
Callback function used to:
    - check the number of repetitions already performed;
    - increased it if less than MAX_REPS;
    - compute the current delay;
    - print the message;
    - stop the timer if repetitions = MAX_REPS.
*/
void vTimerCallback(TimerHandle_t xTimer) {
    if (repetitions < MAX_REPS) {
        printf("Message delay: %d\n", currentDelay);
        repetitions++;
        currentDelay += 1000;
        xTimerChangePeriod(xTimer, pdMS_TO_TICKS((currentDelay)), 0);
    } else {
        xTimerStop(xTimer, 0);
    }
}

void demoTimer() {
    /* Timer creation */
    xTimer = xTimerCreate("Timer", pdMS_TO_TICKS(currentDelay), pdTRUE, (void *) 0, vTimerCallback);

    if (xTimer != NULL) {
        /* Function used to start the timer. No block time is specified. */
        if ((xTimerStart(xTimer, 0)) != pdPASS) {
            printf("ERROR: timer cannot be started.\n");
        }

        /* Starting FreeRTOS scheduler */
        vTaskStartScheduler();
        
        for(;;);
    } else {
        printf("ERROR: bad timer creation.\n");
    }
}