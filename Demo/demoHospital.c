/*  Demo 3
    Hospital management
    
    The hospital has "MAX_ROOM" available to operate the patients
    with each room being working at the same time.
    
    Each patient has a colour code which identifies the priority and the aggravation time. In case
    the operation is not concluded before that time, the patient priority worsen like this:
    
            code green  --> code orange (10  seconds)
            code orange --> code red    (5   seconds)
            code red    --> dead        (2.5 seconds)

    
    Hypothesis:
        - a patient can exit an operating room, only when the operation is finished
        - Each code has a duration time of the operation
        - The operation are scheduled based on the colour priority and the difference between the operation time
          and the arrival time


    Each operation room is a queue of MAX_ROOM elements

    The patient are stored in three queue (based on their associated colour)

    Semaphores manages which patient queue enters in the operating room.
*/

/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "queue.h"
#include <string.h>

#define MAX_TASKS 3
#define MAX_COUNT 1
#define RED_OPERATION_TICK (5 * configTICK_RATE_HZ)
#define ORANGE_OPERATION_TICK 4 
#define GREEN_OPERATION_TICK (3 * configTICK_RATE_HZ)


#define PATIENT 9
#define MAX_ROOM 2

int greenPatients[PATIENT] = {2,6,12,14,18,22,26,28,32};
int redPatients[PATIENT] = {3,5,10,17,19,22,24,38,41};


/* Structure to handle the parameters of the fillQueue function */
typedef struct {
    QueueHandle_t queue;
    int dataArray[PATIENT];
} QueueFillParameters_t;

QueueFillParameters_t redParams;
QueueFillParameters_t greenParams;

// Function to fill a queue 
void fillQueue( void *pvParameters ) {
    QueueFillParameters_t *params = (QueueFillParameters_t *)pvParameters;
    const char *taskName = pcTaskGetName(NULL);
    const char *message;
    if (strcmp(taskName, "GreenFill") == 0) {
        message = "green";
    } else {
        message = "red";
    }

    TickType_t xStartTime = xTaskGetTickCount(); 

        for (int i = 0; i < PATIENT; i++) {
        // Compute the waiting time in ticks, from the start of the tasks 
        TickType_t delay = params->dataArray[i] * configTICK_RATE_HZ;
        TickType_t xNow = xTaskGetTickCount();
        TickType_t xWait = (xStartTime + delay > xNow) ? (xStartTime + delay - xNow) : 0;

        // Wait until the specified time
        vTaskDelay(xWait);

        // Insert the data in the queue
        if (xQueueSend(params->queue, &(params->dataArray[i]), portMAX_DELAY) != pdPASS) {
            printf("Error during the insert of the data in the queue\n");
        }

        printf("Time: %d seconds; Code: %s\n", params->dataArray[i], message);
    }


    for( ;; );
}

TimerHandle_t xTimers[MAX_ROOM];
SemaphoreHandle_t xSemaphoreOperatingRoom;

// Timer callback to notify the completion of an operation
void operationCompleteCallback(TimerHandle_t xTimer) {
    int salaId = (int)pvTimerGetTimerID(xTimer);
    TickType_t xTime = xTaskGetTickCount()/configTICK_RATE_HZ;
    printf("Operation ended in room %d at %d seconds\n", salaId, xTime);
    xSemaphoreGive(xSemaphoreOperatingRoom);
}

void operatingRoomTask(void *pvParameters) {
    int patient;
    char codePatient;

    for (;;) {
        // Flag to tell if a patient is found
        BaseType_t patientFound = pdFALSE;

        if (uxQueueMessagesWaiting(redParams.queue) > 0 && uxSemaphoreGetCount(xSemaphoreOperatingRoom)>0) {
            if (xQueueReceive(redParams.queue, &patient, 0) == pdTRUE) {
                codePatient = 'R';
                printf("\nTaking: %d\n",patient);
                patientFound = pdTRUE;
            }
        } else if (uxQueueMessagesWaiting(greenParams.queue) > 0 && uxSemaphoreGetCount(xSemaphoreOperatingRoom)>0) {
            if (xQueueReceive(greenParams.queue, &patient, 0) == pdTRUE) {
                codePatient = 'G';
                printf("\nTaking: %d\n",patient);
                patientFound = pdTRUE;
            }
        }

        // Start an operation if a patient is found
        if (patientFound && xSemaphoreTake(xSemaphoreOperatingRoom, 0) == pdTRUE) {
            TickType_t operationTicks = (codePatient == 'R') ? RED_OPERATION_TICK : GREEN_OPERATION_TICK;
            
            for (int i = 0; i < MAX_ROOM; i++) {
                if (xTimerIsTimerActive(xTimers[i]) == pdFALSE) {
                    printf("Start operating patient %d, in room %d\n", patient,i);
                    xTimerChangePeriod(xTimers[i], operationTicks, 0);
                    xTimerReset(xTimers[i], 0);
                    vTimerSetTimerID(xTimers[i], (void *)(intptr_t)i);
                    break;
                }
            }
        } else {
            // If there are no patient, wait a bit of time
            vTaskDelay(100);
        }
    }
}

/*
void salaOperatoriaTask(void *pvParameters) {
    int paziente;
    for (;;) {
        // Aspetta finché non ci sono pazienti in una delle due code
        if (uxQueueMessagesWaiting(redParams.queue) == 0 && uxQueueMessagesWaiting(greenParams.queue) == 0) {
            vTaskDelay(100); // Attesa prima di controllare di nuovo
        }

        // Cerca prima un paziente nella coda rossa
        if (uxQueueMessagesWaiting(redParams.queue) > 0) {
            if (xQueueReceive(redParams.queue, &paziente, 0) == pdTRUE) {
                // Prova ad ottenere l'accesso a una sala operatoria
                if (xSemaphoreTake(xSemaphoreSaleOperatorie, portMAX_DELAY) == pdTRUE) {
                    printf("Inizio operazione su paziente rosso arrivato al secondo %d\n", paziente);
                    // Simula l'operazione...
                    vTaskDelay(RED_OPERATION_TICK);
                    printf("Fine operazione su paziente rosso arrivato al secondo %d\n", paziente);
                    xSemaphoreGive(xSemaphoreSaleOperatorie);
                }
            }
        }

        // Se non ci sono pazienti rossi, cerca nella coda verde
        else if (uxQueueMessagesWaiting(greenParams.queue) > 0) {
            if (xQueueReceive(greenParams.queue, &paziente, 0) == pdTRUE) {
                // Prova ad ottenere l'accesso a una sala operatoria
                if (xSemaphoreTake(xSemaphoreSaleOperatorie, portMAX_DELAY) == pdTRUE) {
                    printf("Inizio operazione su paziente verde arrivato al secondo %d\n", paziente);
                    // Simula l'operazione...
                    vTaskDelay(GREEN_OPERATION_TICK);
                    printf("Fine operazione su paziente verde arrivato al secondo %d\n", paziente);
                    xSemaphoreGive(xSemaphoreSaleOperatorie);
                }
            }
        }
    }
}

*/


int demoHospital( void ) {
    QueueHandle_t redQueue = xQueueCreate(PATIENT, sizeof(int));
    QueueHandle_t greenQueue = xQueueCreate(PATIENT, sizeof(int));

    if (redQueue == NULL || greenQueue == NULL) {
        printf("Errore nella creazione delle code\n");
        return 1;
    }

    redParams.queue = redQueue;
    greenParams.queue = greenQueue;

    memcpy(redParams.dataArray, redPatients, sizeof(redPatients));
    memcpy(greenParams.dataArray, greenPatients, sizeof(greenPatients));

    xSemaphoreOperatingRoom = xSemaphoreCreateCounting(MAX_ROOM, MAX_ROOM);

    for (int i = 0; i < MAX_ROOM; i++) {
        xTimers[i] = xTimerCreate("OperationTimer", pdMS_TO_TICKS(RED_OPERATION_TICK), pdFALSE, (void *)(intptr_t)i, operationCompleteCallback);
    }


    xTaskCreate( fillQueue, "RedFill", configMINIMAL_STACK_SIZE, &redParams, tskIDLE_PRIORITY + 1, NULL );
    xTaskCreate( fillQueue, "GreenFill", configMINIMAL_STACK_SIZE, &greenParams, tskIDLE_PRIORITY + 1, NULL );

    xTaskCreate(operatingRoomTask, "SalaOper", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);


    vTaskStartScheduler();

    for( ;; );
}


/*
periodic task
aperiodic task

*/
