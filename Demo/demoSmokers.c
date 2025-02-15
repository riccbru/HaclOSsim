#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Ingredient types */
#define TOBACCO  0
#define PAPER    1
#define MATCHES  2

/* Semaphores for smoker synchronization */
SemaphoreHandle_t xSemaphoreTobaccoSmoker;
SemaphoreHandle_t xSemaphorePaperSmoker;
SemaphoreHandle_t xSemaphoreMatchesSmoker;
SemaphoreHandle_t xSemaphoreSmokerDone;

/* Simple counter for pseudo-random number generation */
static uint32_t next_random = 1;

/* Simple random number generator that doesn't use malloc */
static int custom_rand(void) {
    next_random = next_random * 1103515245 + 12345;
    return (next_random >> 16) & 32767;
}

/* Agent Task: Places two ingredients on the table */
void taskAgent(void *pvParameter) {
    while (1) {
        int choice = custom_rand() % 3;  // Randomly select which two ingredients to place

        printf("\033[1;92m[+]\033[0m  Agent places ");
        switch (choice) {
            case TOBACCO:
                printf("\033[92mPAPER\033[0m & \033[92mMATCHES\033[0m\n");
                xSemaphoreGive(xSemaphoreTobaccoSmoker);  // Tobacco smoker is missing, notify them
                break;
            case PAPER:
                printf("\033[92mTOBACCO\033[0m & \033[92mMATCHES\033[0m\n");
                xSemaphoreGive(xSemaphorePaperSmoker);    // Paper smoker is missing
                break;
            case MATCHES:
                printf("\033[92mTOBACCO\033[0m & \033[92mPAPER\033[0m\n");
                xSemaphoreGive(xSemaphoreMatchesSmoker);  // Matches smoker is missing
                break;
        }

        // Wait for a smoker to finish
        xSemaphoreTake(xSemaphoreSmokerDone, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay before next round
    }
}

/* Smoker Task: Waits for missing ingredient, then smokes */
void taskSmoker(void *pvParameter) {
    int smokerType = *(int *)pvParameter;  // Get the smoker's ingredient type

    while (1) {
        // Wait until agent provides the required ingredients
        switch (smokerType) {
            case TOBACCO:
                xSemaphoreTake(xSemaphoreTobaccoSmoker, portMAX_DELAY);
                break;
            case PAPER:
                xSemaphoreTake(xSemaphorePaperSmoker, portMAX_DELAY);
                break;
            case MATCHES:
                xSemaphoreTake(xSemaphoreMatchesSmoker, portMAX_DELAY);
                break;
        }

        printf("\033[1;90m[~]\033[0m  Smoker with ");
        switch (smokerType) {
            case TOBACCO: printf("TOBACCO "); break;
            case PAPER: printf("PAPER "); break;
            case MATCHES: printf("MATCHES "); break;
        }
        printf("\033[1;90msmokes\033[0m\n");
        printf("\033[1;90m[~]\033[0m\t    \033[1;90m. . .\033[0m\n");

        vTaskDelay(pdMS_TO_TICKS(2000));  // Simulate smoking time

        printf("\033[1;92m[*]\033[0m  Smoker \033[92mfinished\033[0m smoking\n\n");

        // Notify the agent that smoking is done
        xSemaphoreGive(xSemaphoreSmokerDone);
    }
}

void demoSmokers() {
    // Create semaphores
    xSemaphoreTobaccoSmoker = xSemaphoreCreateBinary();
    xSemaphorePaperSmoker = xSemaphoreCreateBinary();
    xSemaphoreMatchesSmoker = xSemaphoreCreateBinary();
    xSemaphoreSmokerDone = xSemaphoreCreateBinary();

    // Create agent task
    xTaskCreate(taskAgent, "Agent", configMINIMAL_STACK_SIZE, NULL, 2, NULL);

    // Create smoker tasks
    static int smoker1 = TOBACCO, smoker2 = PAPER, smoker3 = MATCHES;
    xTaskCreate(taskSmoker, "SmokerTobacco", configMINIMAL_STACK_SIZE, &smoker1, 1, NULL);
    xTaskCreate(taskSmoker, "SmokerPaper", configMINIMAL_STACK_SIZE, &smoker2, 1, NULL);
    xTaskCreate(taskSmoker, "SmokerMatches", configMINIMAL_STACK_SIZE, &smoker3, 1, NULL);

    // Start scheduler
    vTaskStartScheduler();
    for (;;);
}