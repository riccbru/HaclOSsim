#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define NUM_PHILOSOPHERS 5

// Array of binary semaphores for forks
SemaphoreHandle_t forks[NUM_PHILOSOPHERS];

void philosopherTask(void *param) {
    int id = *(int *)param;
    int leftFork = id;
    int rightFork = (id + 1) % NUM_PHILOSOPHERS;

    // Even philosophers pick up left fork first, odd pick up right first
    // 
    int firstFork = (id % 2 == 0) ? leftFork : rightFork;
    int secondFork = (id % 2 == 0) ? rightFork : leftFork;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        printf("\033[95m[ Philosopher %d ]\033[0m   is \033[1;90mthinking\033[0m\n", id + 1);

        // Pick up forks
        xSemaphoreTake(forks[firstFork], portMAX_DELAY);
        printf("\033[95m[ Philosopher %d ]\033[0m   picked up \033[91mfork %d\033[0m (%s)\n", id + 1, firstFork + 1, (firstFork == leftFork) ? "SX" : "DX");
        
        xSemaphoreTake(forks[secondFork], portMAX_DELAY);
        printf("\033[95m[ Philosopher %d ]\033[0m   picked up \033[91mfork %d\033[0m (%s)\n", id + 1, secondFork + 1, (secondFork == leftFork) ? "SX" : "DX");

        // Eat
        printf("\033[95m[ Philosopher %d ]\033[0m   is \033[93meating\033[0m\n", id + 1);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Put down forks
        xSemaphoreGive(forks[firstFork]);
        xSemaphoreGive(forks[secondFork]);

        printf("\033[95m[ Philosopher %d ]\033[0m   \033[92mfinished eating\033[0m & \033[92mput down forks\033[0m\n", id + 1);
    }
}

void demoDinner() {
    // Initialize semaphores for forks
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        forks[i] = xSemaphoreCreateBinary();
        xSemaphoreGive(forks[i]);
    }

    // Create philosopher tasks
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        static int philosopherId[NUM_PHILOSOPHERS];
        philosopherId[i] = i;
        xTaskCreate(philosopherTask, "Philosopher", configMINIMAL_STACK_SIZE * 4, &philosopherId[i], 1, NULL);
    }

    // Start scheduler
    vTaskStartScheduler();
    for (;;);
}


