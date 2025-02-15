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
    int serviceTime;
    int hairLength; // Hair length affecting service time
} CustomerData_t;

QueueHandle_t xWaitingRoomQueue;
SemaphoreHandle_t xBarberChair;
TaskHandle_t xSchedulerTaskHandle;
int servedCustomers = 0, lostCustomers = 0;

CustomerData_t customers[NUM_CUSTOMERS] = {
    {1, 1, 10, 2},
    {2, 3, 8, 4},
    {3, 5, 12, 1},
    {4, 1, 7, 1},
    {5, 4, 15, 5},
};

void vBarberTask(void *pvParameters) {
    ( void ) pvParameters;
    CustomerData_t customer;
    for (;;) {
        if (xQueueReceive(xWaitingRoomQueue, &customer, portMAX_DELAY) == pdTRUE) {
            printf("[Barber] Serving Customer %d\n", customer.id);
            xSemaphoreTake(xBarberChair, portMAX_DELAY);
            vTaskDelay(pdMS_TO_TICKS(customer.serviceTime));
            printf("[Barber] Finished Customer %d\n", customer.id);
            xSemaphoreGive(xBarberChair);
            servedCustomers++;
        }
    }
}

void vCustomerTask(void *pvParameters) {
    CustomerData_t *customer = (CustomerData_t *)pvParameters;
    if (uxQueueMessagesWaiting(xWaitingRoomQueue) < NUM_SEATS) {
        xQueueSendToBack(xWaitingRoomQueue, customer, 0);
        printf("[Customer %d] Waiting in room\n", customer->id);
    } else {
        printf("[Customer %d] Left: No seats\n", customer->id);
        lostCustomers++;
    }
    vTaskDelete(NULL);
}



void vCustomerGeneratorCallback(TimerHandle_t xTimer) {
    int customerIndex = (int)pvTimerGetTimerID(xTimer);
    xTaskCreate(vCustomerTask, "Customer", configMINIMAL_STACK_SIZE, &customers[customerIndex], tskIDLE_PRIORITY + 1, NULL);
}

void vCustomerGeneratorTask(void *pvParameters) {
    ( void ) pvParameters;
    TimerHandle_t xTimers[NUM_CUSTOMERS];
    for (int i = 0; i < NUM_CUSTOMERS; i++) {
        xTimers[i] = xTimerCreate("CustomerTimer", pdMS_TO_TICKS(customers[i].arrivalTime), pdFALSE, (void *)i, vCustomerGeneratorCallback);
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

    xTaskCreate(vBarberTask, "Barber", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL);
    xTaskCreate(vCustomerGeneratorTask, "CustomerGen", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(vEndStatistics, "EndStats", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL);

    vTaskStartScheduler();
    return 0;
}