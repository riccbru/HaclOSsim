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
        printf("Philosopher %d is thinking\n", id);
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Pick up forks
        xSemaphoreTake(forks[firstFork], portMAX_DELAY);
        printf("Philosopher %d picked up fork %d\n", id, firstFork);

        xSemaphoreTake(forks[secondFork], portMAX_DELAY);
        printf("Philosopher %d picked up fork %d\n", id, secondFork);

        // Eat
        printf("Philosopher %d is eating\n", id);
        vTaskDelay(pdMS_TO_TICKS(2000));

        // Put down forks
        xSemaphoreGive(forks[firstFork]);
        xSemaphoreGive(forks[secondFork]);

        printf("Philosopher %d finished eating and put down forks\n", id);
    }
}

void demoDiningPhilosophers() {
    // Initialize semaphores for forks
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        forks[i] = xSemaphoreCreateBinary();
        xSemaphoreGive(forks[i]);
    }

    // Create philosopher tasks
    for (int i = 0; i < NUM_PHILOSOPHERS; i++) {
        static int philosopherId[NUM_PHILOSOPHERS];
        philosopherId[i] = i;
        xTaskCreate(philosopherTask, "Philosopher", configMINIMAL_STACK_SIZE, &philosopherId[i], 1, NULL);
    }

    // Start scheduler
    vTaskStartScheduler();
    for (;;);
}


