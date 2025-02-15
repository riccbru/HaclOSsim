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
#define MAX_CUSTOMERS       5   // Max customers that can wait in the waiting room
#define NUM_CUSTOMERS_TOTAL 10  // Total number of customers arriving at the shop
#define HAIR_CUT_DURATION_MS 2000 // Time for a haircut (milliseconds)
#define CUSTOMER_ARRIVAL_DELAY_MS 500 //Delay between customer arrivals

/* Queue and Semaphore Handles (Shared Objects) */
QueueHandle_t xWaitingRoomQueue;  // Queue for customers waiting in the waiting room
SemaphoreHandle_t xBarberChair;    // Binary semaphore representing the barber chair (available/unavailable)

/* Function Prototypes */
void vBarberTask( void *pvParameters );
void vCustomerTask( void *pvParameters );
void vCustomerGeneratorTask(void *pvParameters); // New task to create customers

/*-----------------------------------------------------------*/

void vBarberTask( void *pvParameters )
{
    int customerID;
    BaseType_t xStatus;

    ( void ) pvParameters; // Suppress unused parameter warning

    printf( "Barber: Shop is open!\n" );

    for ( ;; ) {
        // Wait for a customer to enter the waiting room
        xStatus = xQueueReceive(xWaitingRoomQueue, &customerID, portMAX_DELAY);

        if (xStatus == pdPASS) {
            // Customer found in the waiting room
            printf("Barber: Waking up. Customer %d is here.\n", customerID);

            // Take the barber chair (wait until it's available)
            xSemaphoreTake(xBarberChair, portMAX_DELAY);
            printf("Barber: Cutting hair for Customer %d\n", customerID);

            // Simulate haircut duration
            vTaskDelay(pdMS_TO_TICKS(HAIR_CUT_DURATION_MS));

            printf("Barber: Finished haircut for Customer %d\n", customerID);

            // Release the barber chair
            xSemaphoreGive(xBarberChair);

        } else {
            // This should not happen if using portMAX_DELAY
            printf("Barber: Error receiving customer from waiting room!\n");
        }
    }
}

/*-----------------------------------------------------------*/

void vCustomerTask( void *pvParameters )
{
    int customerID = (int)pvParameters;
    BaseType_t xStatus;

    printf("Customer %d: Arrived at the barber shop.\n", customerID);

    // Check if there's a free chair in the waiting room
    if (uxQueueMessagesWaiting(xWaitingRoomQueue) < MAX_CUSTOMERS) {
        // Enter the waiting room
        printf("Customer %d: Entering the waiting room.\n", customerID);
        xStatus = xQueueSendToBack(xWaitingRoomQueue, &customerID, 0); // Non-blocking send

        if (xStatus != pdPASS) {
            printf("Customer %d: Waiting room is full, leaving.\n", customerID);
        }
    } else {
        // No free chairs, customer leaves
        printf("Customer %d: No free chairs. Leaving the shop.\n", customerID);
    }

    vTaskDelete(NULL); // Customer task is done after arriving.
}

/*-----------------------------------------------------------*/

void vCustomerGeneratorTask(void *pvParameters) {
    (void)pvParameters;
    BaseType_t xReturned;
    int i;

    for (i = 1; i <= NUM_CUSTOMERS_TOTAL; i++) {
        xReturned = xTaskCreate(
                        vCustomerTask,      /* Function that implements the task. */
                        "Customer",          /* Text name for the task. */
                        configMINIMAL_STACK_SIZE,  /* Stack size in words, not bytes. */
                        (void *)i,          /* Parameter passed into the task. */
                        tskIDLE_PRIORITY + 1, /* Priority at which the task is created. */
                        NULL );            /* Used to pass out the created task's handle. */

        if (xReturned != pdPASS) {
            printf("Error creating customer %d task\n", i);
        }
        vTaskDelay(pdMS_TO_TICKS(CUSTOMER_ARRIVAL_DELAY_MS));  // Space out customer arrival times.
    }
    vTaskDelete(NULL); //Generator finished.
}


/*-----------------------------------------------------------*/

int demoBarber( void )
{
    BaseType_t xReturned;

    /* Create the queues and semaphores. */
    xWaitingRoomQueue = xQueueCreate(MAX_CUSTOMERS, sizeof(int));
    xBarberChair = xSemaphoreCreateBinary();

    if (xWaitingRoomQueue == NULL || xBarberChair == NULL) {
        printf( "Error creating queues or semaphores\n" );
        return 1;
    }

    /* Give the barber chair semaphore, indicating it's initially available. */
    xSemaphoreGive(xBarberChair);

    /* Create the barber task. */
    xReturned = xTaskCreate(
                    vBarberTask,       /* Function that implements the task. */
                    "Barber",           /* Text name for the task. */
                    configMINIMAL_STACK_SIZE,   /* Stack size in words, not bytes. */
                    NULL,               /* Parameter passed into the task. */
                    tskIDLE_PRIORITY + 2,/* Priority at which the task is created. */
                    NULL );             /* Used to pass out the created task's handle. */

    if (xReturned == pdPASS) {
        printf("Barber task created successfully\n");
    } else {
        printf("Error creating barber task\n");
        return 1;
    }


    /* Create the customer generator task. */
    xReturned = xTaskCreate(
                    vCustomerGeneratorTask,       /* Function that generates customer tasks. */
                    "CustomerGenerator",           /* Text name for the task. */
                    configMINIMAL_STACK_SIZE,   /* Stack size in words, not bytes. */
                    NULL,               /* Parameter passed into the task. */
                    tskIDLE_PRIORITY + 1,/* Priority at which the task is created. */
                    NULL );             /* Used to pass out the created task's handle. */

    if (xReturned == pdPASS) {
        printf("Customer Generator task created successfully\n");
    } else {
        printf("Error creating Customer Generator task\n");
        return 1;
    }

    /* Start the scheduler. */
    vTaskStartScheduler();

    /* Will not reach here unless there was an error starting the
     * scheduler. */
    return 0;
}
