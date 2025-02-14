/* 
    In this demo, three tasks are created, which cyclically 
    access a critical section and, when a task is operating 
    in this section, the others continue to sleep, until 
    one of the two is woken up.
*/

/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

#define MAX_TASKS 3
#define MAX_COUNT 1

SemaphoreHandle_t xSemaphores[MAX_TASKS];

void vTask3() {
    while(1) {
        /* Task3 wakes up */
        if (xSemaphoreTake(xSemaphores[2], portMAX_DELAY) == pdTRUE) {
            /* Task3 waits for 3 seconds */
            printf("After 3 seconds...\n");
            vTaskDelay(pdMS_TO_TICKS(3000));
            printf("Wake up: task 1\n");
            /* Task1 is woken up */
            xSemaphoreGive(xSemaphores[0]);
        }

    }
}

void vTask2() {
    while(1) {
        /* Task2 wakes up */
        if (xSemaphoreTake(xSemaphores[1], portMAX_DELAY) == pdTRUE) {
            /* Task2 waits for 2 seconds */
            printf("After 2 seconds...\n");
            vTaskDelay(pdMS_TO_TICKS(2000));
            printf("Wake up: task 3\n");
            /* Task3 is woken up */
            xSemaphoreGive(xSemaphores[2]);
        }

    }
}

void vTask1() {
    while(1) {
        /* Task1 wakes up */
        if (xSemaphoreTake(xSemaphores[0], portMAX_DELAY) == pdTRUE) {
            /* Task1 waits for 1 second */
            printf("After 1 second...\n");
            vTaskDelay(pdMS_TO_TICKS(1000));
            printf("Wake up: task 2\n");
            /* Task2 is woken up */
            xSemaphoreGive(xSemaphores[1]);
        }

    }
}

void demoSemaphores() {
    /* Mutex semaphores are created */
    for (int i = 0; i < MAX_TASKS; i++) {
        xSemaphores[i] = xSemaphoreCreateCounting(MAX_COUNT, 0);
        if (xSemaphores[i] == NULL) {
            printf("ERROR: bad semaphore creation.\n");
        }
    }
    xSemaphoreGive(xSemaphores[0]);
    
    /* Task creation */
    xTaskCreate(vTask1, "Semaphore", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(vTask2, "Semaphore", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    xTaskCreate(vTask3, "Semaphore", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

    /* Starting FreeRTOS scheduler */
    vTaskStartScheduler();
    
    for(;;);
}
