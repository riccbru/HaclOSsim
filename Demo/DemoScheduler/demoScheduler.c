#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"

#define NUM_CUSTOMERS 5      // Total number of customers
#define NUM_SEATS 3          // Maximum waiting room seats
#define HAIRCUT_TIME 2000    // Haircut duration in ms
#define CUSTOMER_ARRIVAL 500 // Delay between customer arrivals

/* Customer data structure */
typedef struct {
    int id;         // Customer ID
    int arrivalTime;
    int expirationTime; // Time after which the customer leaves if not served
    int serviceTime; // Hair cut duration
} CustomerData_t;

QueueHandle_t xWaitingRoomQueue;
SemaphoreHandle_t xBarberChair;
TaskHandle_t xSchedulerTaskHandle;
int servedCustomers = 0, lostCustomers = 0;

CustomerData_t customers[NUM_CUSTOMERS] = {
    {1, 1, 10, 2},
    {2, 3, 8, 5},
    {3, 5, 12, 1},
    {4, 1, 7, 1},
    {5, 4, 15, 2},
};

void vBarberTask(void *pvParameters) {
    CustomerData_t currentCustomer;
    int remainingTime = 0;
    int barberBusy = 0; // Flag to track if the barber is cutting hair

    for (;;) {
        if (!barberBusy) {
            // Pick the highest-priority customer from the queue
            if (xQueueReceive(xWaitingRoomQueue, &currentCustomer, portMAX_DELAY) == pdTRUE) {
                TickType_t currentTime = xTaskGetTickCount();
                
                // **Check if the customer's deadline has already expired**
                if (currentTime >= (currentCustomer.expirationTime * configTICK_RATE_HZ)) {
                    printf("[Barber] Customer %d left: Expired at %d seconds\n",
                           currentCustomer.id, currentTime / configTICK_RATE_HZ);
                    lostCustomers++;
                    continue; // Skip to the next customer
                }

                printf("[Barber] Starting haircut for customer %d at %d seconds\n", 
                        currentCustomer.id, currentTime / configTICK_RATE_HZ);
                remainingTime = currentCustomer.serviceTime * configTICK_RATE_HZ;
                barberBusy = 1;
            }
        }

        if (barberBusy) {
            // Check if a more urgent customer has arrived
            CustomerData_t nextCustomer;
            if (xQueuePeek(xWaitingRoomQueue, &nextCustomer, 0) == pdTRUE) {
                
                if (nextCustomer.expirationTime < currentCustomer.expirationTime) {
                    
                    // **Conditional Preemption** based on config flag
                    #if configALLOW_MID_HAIRCUT_PREEMPTION
                        printf("[Barber] Preempting customer %d for customer %d at %d seconds\n", 
                                currentCustomer.id, nextCustomer.id, xTaskGetTickCount() / configTICK_RATE_HZ);

                        // Put the unfinished customer back in the queue
                        xQueueSendToBack(xWaitingRoomQueue, &currentCustomer, 0);

                        // Switch to the new customer immediately
                        barberBusy = 0;
                        taskYIELD();
                        continue; // Restart loop to serve the urgent customer
                    #endif
                }
            }

            // **Check during the haircut if the deadline has expired**
            if (xTaskGetTickCount() >= (currentCustomer.expirationTime * configTICK_RATE_HZ)) {
                printf("[Barber] Customer %d left mid-haircut: Expired at %d seconds\n",
                       currentCustomer.id, xTaskGetTickCount() / configTICK_RATE_HZ);
                lostCustomers++;
                barberBusy = 0;
                continue; // Move on to the next customer
            }

            // Continue haircut
            vTaskDelay(pdMS_TO_TICKS(1000)); // Simulate 1 second of cutting
            remainingTime -= configTICK_RATE_HZ;

            if (remainingTime <= 0) {
                printf("[Barber] Finished haircut for customer %d at %d seconds\n", 
                        currentCustomer.id, xTaskGetTickCount() / configTICK_RATE_HZ);
                servedCustomers++;
                barberBusy = 0;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Prevent CPU overuse
    }
}





void vCustomerTask(void *pvParameters) {
    CustomerData_t *customer = (CustomerData_t *)pvParameters;

    // Calculate priority: Higher priority (lower number) for earlier expiration
    UBaseType_t priority = tskIDLE_PRIORITY + customer->expirationTime;

    // Ensure priority stays within valid bounds
    if (priority < tskIDLE_PRIORITY + 1) priority = tskIDLE_PRIORITY + 1;
    if (priority > configMAX_PRIORITIES - 1) priority = configMAX_PRIORITIES - 1;

    // Set dynamic priority
    vTaskPrioritySet(NULL, priority);

    printf("[Customer %d] Waiting in room, Expiration: %d sec, Priority: %d\n",
           customer->id, customer->expirationTime, priority);

    if (uxQueueMessagesWaiting(xWaitingRoomQueue) < NUM_SEATS) {
        xQueueSendToBack(xWaitingRoomQueue, customer, 0);

        // Force a reschedule to allow higher-priority tasks to take over
        taskYIELD();
    } else {
        printf("[Customer %d] Left: No seats available\n", customer->id);
        lostCustomers++;
    }

    vTaskDelete(NULL);
}







void vCustomerGeneratorCallback(TimerHandle_t xTimer) {
    int customerIndex = (int)pvTimerGetTimerID(xTimer);
    UBaseType_t priority = tskIDLE_PRIORITY + 1 + (NUM_CUSTOMERS - customerIndex); // Assign higher priority to earlier customers
    xTaskCreate(vCustomerTask, "Customer", configMINIMAL_STACK_SIZE, &customers[customerIndex], priority, NULL);
}

void vCustomerGeneratorTask(void *pvParameters) {
    // Sort customers first by arrival time, then by expiration time (EDF)
    for (int i = 0; i < NUM_CUSTOMERS - 1; i++) {
        for (int j = i + 1; j < NUM_CUSTOMERS; j++) {
            if (customers[i].arrivalTime > customers[j].arrivalTime ||
               (customers[i].arrivalTime == customers[j].arrivalTime && customers[i].expirationTime > customers[j].expirationTime)) {
                CustomerData_t temp = customers[i];
                customers[i] = customers[j];
                customers[j] = temp;
            }
        }
    }

    TimerHandle_t xTimers[NUM_CUSTOMERS];
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        xTimers[i] = xTimerCreate("CustomerTimer", pdMS_TO_TICKS(customers[i].arrivalTime * 1000), pdFALSE, (void *)i, vCustomerGeneratorCallback);
        xTimerStart(xTimers[i], 0);
    }
    vTaskDelete(NULL);
}



void vEndStatistics(void *pvParameters) {
    ( void ) pvParameters;
    while (servedCustomers + lostCustomers < NUM_CUSTOMERS) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    printf("--- Barber Shop Closed ---\n");
    printf("Total Customers: %d\n", NUM_CUSTOMERS);
    printf("Served Customers: %d\n", servedCustomers);
    printf("Lost Customers: %d\n", lostCustomers);
    vTaskEndScheduler();
}

int main_scheduler(void) {
    xWaitingRoomQueue = xQueueCreate(NUM_SEATS, sizeof(CustomerData_t));
    xBarberChair = xSemaphoreCreateBinary();
    xSemaphoreGive(xBarberChair);

    xTaskCreate(vBarberTask, "Barber", configMINIMAL_STACK_SIZE * 4, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(vCustomerGeneratorTask, "CustomerGen", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(vEndStatistics, "EndStats", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);

    vTaskStartScheduler();
    return 0;
}