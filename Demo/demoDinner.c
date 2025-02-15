#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define NUM_PHILOSOPHERS 5

// Semaphores for forks
SemaphoreHandle_t forks[NUM_PHILOSOPHERS];

void philosopherTask(void *param) {
    int id = *(int *)param;
    int leftFork = id;
    int rightFork = (id + 1) % NUM_PHILOSOPHERS;

    // Apply asymmetry: even philosophers pick up left fork first, odd pick up right first
    int firstFork = (id % 2 == 0) ? leftFork : rightFork;
    int secondFork = (id % 2 == 0) ? rightFork : leftFork;

    while (1) {
        printf("\033[1;90m[~]\033[0m  Philosopher %d is \033[1;90mthinking\033[0m\n", id);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Pick up forks
        xSemaphoreTake(forks[firstFork], portMAX_DELAY);
        printf("\033[1;91m[/]\033[0m  Philosopher %d \033[91mpicked up fork %d\033[0m\n", id, firstFork);

        xSemaphoreTake(forks[secondFork], portMAX_DELAY);
        printf("\033[1;91m[X]\033[0m  Philosopher %d \033[91mpicked up fork %d\033[0m\n", id, secondFork);

        // Eat
        printf("\033[1;93m[~]\033[0m  Philosopher %d is \033[93meating\033[0m\n", id);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Put down forks
        xSemaphoreGive(forks[firstFork]);
        xSemaphoreGive(forks[secondFork]);

        printf("\033[1;92m[*]\033[0m  Philosopher %d \033[92mfinished eating\033[0m\n", id);
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


