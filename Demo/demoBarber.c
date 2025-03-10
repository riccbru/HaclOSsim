/*
Barber Shop Simulation using FreeRTOS Queues

Problem Description:
A barber shop has a waiting room with a limited number of chairs.  The barber
works only when there are customers to serve. If there are no customers,
the barber sleeps. When a customer arrives, if the barber is free, the
customer wakes up the barber and gets a haircut. If the barber is busy, the
customer waits in the waiting room if there is an available chair; otherwise,
the customer leaves.

This demo uses FreeRTOS queues to synchronize the barber and customers.
*/

#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Constants */
#define NUM_SEATS                     25          //  Max customers that can wait in the waiting room
#define NUM_CUSTOMERS                 50          //  Total number of customers arriving at the shop
#define HAIR_CUT_DURATION_MS          2000        //  Time for a haircut (milliseconds)
#define CUSTOMER_ARRIVAL_DELAY_MS     500         //  Delay between customer arrivals

/* Queue and Semaphore Handles (Shared Objects) */
QueueHandle_t xWaitingRoomQueue;                  // Queue for customers waiting in the waiting room
SemaphoreHandle_t xBarberChair;     // Binary semaphore representing the barber: available or not

/* Function Prototypes */
void vBarberTask( void *pvParameters );
void vCustomerTask( void *pvParameters );
void vCustomerGeneratorTask(void *pvParameters);  // New task to create customers

/*-----------------------------------------------------------*/

void vBarberTask( void *pvParameters )
{
    int customerID;
    BaseType_t xStatus;

    // Suppress warning: Unused Parameter
    ( void ) pvParameters;

    for ( ;; ) {
        // Wait for a customer to enter the waiting room
        xStatus = xQueueReceive(xWaitingRoomQueue, &customerID, portMAX_DELAY);

        if (xStatus == pdPASS) {
            // Customer found in the waiting room
            printf("\033[95m [  BARBER  ]\033[0m\t\033[1;90mWakes up\033[0m\n");

            // Take the barber chair (wait until it's available)
            xSemaphoreTake(xBarberChair, portMAX_DELAY);
            printf("\033[95m [  BARBER  ]\033[0m\t\033[93mCutting\033[0m customer %d's hair\n", customerID);

            // Simulate haircut duration
            vTaskDelay(pdMS_TO_TICKS(HAIR_CUT_DURATION_MS));

            printf("\033[95m [  BARBER  ]\033[0m\t\033[92mFinished haircut for customer %d\033[0m\n", customerID);

            // Release the barber chair
            xSemaphoreGive(xBarberChair);

            if (uxQueueMessagesWaiting(xWaitingRoomQueue) == 0) {
                printf("\033[95m [  BARBER  ]\033[0m\t\033[1;90mGoes to sleep\033[0m\n");
            }

        } else {
            // This should not happen if using portMAX_DELAY
            printf("\033[93m[!]\033[0m\t\033[41mError receiving customers in waiting room queue\033[0m\n");
        }
    }
}

/*-----------------------------------------------------------*/

void vCustomerTask( void *pvParameters )
{
    int customerID = (int)pvParameters;
    BaseType_t xStatus;

    printf("\033[95m[ CUSTOMER %d ]\033[0m\t\033[92mArrived\033[0m at the barber shop\n", customerID);

    // Check if there's a free chair in the waiting room
    if (uxQueueMessagesWaiting(xWaitingRoomQueue) < NUM_SEATS) {
        // Enter the waiting room
        printf("\033[95m[ CUSTOMER %d ]\033[0m\tEntering the \033[93mwaiting room\033[0m\n", customerID);
        xStatus = xQueueSendToBack(xWaitingRoomQueue, &customerID, 0);

        if (xStatus != pdPASS) {
            printf("\033[95m[ CUSTOMER %d ]\033[0m\tLeaves: \033[91mwaiting room is full\033[0m\n", customerID);
        }
    } else {
        // No free chairs, customer leaves
        printf("\033[95m[ CUSTOMER %d ]\033[0m\tLeaves: \033[91mwaiting room is full\033[0m\n", customerID);
    }

    vTaskDelete(NULL); // Customer task is done after arriving.
}

/*-----------------------------------------------------------*/

void vCustomerGeneratorTask(void *pvParameters) {
    (void)pvParameters;
    BaseType_t xReturned;
    int i;

    for (i = 1; i <= NUM_CUSTOMERS; i++) {
        xReturned = xTaskCreate(
                        vCustomerTask,            /* Function that implements the task. */
                        "Customer",               /* Text name for the task. */
                        configMINIMAL_STACK_SIZE, /* Stack size in words, not bytes. */
                        (void *)i,                /* Parameter passed into the task. */
                        tskIDLE_PRIORITY + 1,     /* Priority at which the task is created. */
                        NULL );                   /* Used to pass out the created task's handle. */

        if (xReturned != pdPASS) {
            printf("\033[93m[!]\033[0m\t\033[41mError creating customer %d task\033[0m\n", i);
        }
        // Space out customer arrival delay
        vTaskDelay(pdMS_TO_TICKS(CUSTOMER_ARRIVAL_DELAY_MS)); 
    }
    vTaskDelete(NULL); //Generator finished.
}

/*-----------------------------------------------------------*/

int demoBarber( void )
{
    BaseType_t xReturned;

    /* Create the queues and semaphores. */
    xWaitingRoomQueue = xQueueCreate(NUM_SEATS, sizeof(int));
    xBarberChair = xSemaphoreCreateBinary();

    if (xBarberChair == NULL || xWaitingRoomQueue == NULL) {
        printf( "\033[93m[!]\033[0m\t\033[41mError creating barber semaphore or waiting room queue\033[0m\n" );
        return 1;
    }

    /* Barber is available */
    xSemaphoreGive(xBarberChair);

    /* Create the barber tasks dynamically based on NUM_BARBERS */
    xReturned = xTaskCreate(
        vBarberTask,                  /* Function that generates barber task. */
        "Barber",                     /* Text name for the task. */
        configMINIMAL_STACK_SIZE,     /* Stack size in words, not bytes. */
        NULL,                         /* Parameter passed into the task. */
        tskIDLE_PRIORITY + 2,         /* Priority at which the task is created. */
        NULL );                       /* Used to pass out the created task's handle. */


    if (xReturned != pdPASS) {
        printf("\033[93m[!]\033[0m\t\033[41mError creating barber task\033[0m\n");
        return 1;
    }
    

    /* Create the customer generator task. */
    xReturned = xTaskCreate(
                    vCustomerGeneratorTask,       /* Function that generates customer tasks. */
                    "CustomerGenerator",          /* Text name for the task. */
                    configMINIMAL_STACK_SIZE,     /* Stack size in words, not bytes. */
                    NULL,                         /* Parameter passed into the task. */
                    tskIDLE_PRIORITY + 1,         /* Priority at which the task is created. */
                    NULL );                       /* Used to pass out the created task's handle. */

    if (xReturned == pdPASS) {
        printf(" \033[1;45m[*] BARBER SHOP IS NOW OPEN [*]\033[0m\n");
        printf("\033[95m      - \033[1m%d\033[0m Seats\033[0m\n", NUM_SEATS);
        printf("\033[95m      - \033[1m%d\033[0m Customers\033[0m\n", NUM_CUSTOMERS);
    } else {
        printf("\033[93m[!]\033[0m\t\033[41mError creating Customer Generator task\033[0m\n");
        return 1;
    }

    /* Start the scheduler. */
    vTaskStartScheduler();

    /* Will not reach here unless there was an error starting the
     * scheduler. */
    return 0;
}
