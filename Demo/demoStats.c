#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

/* Ensure these are defined in FreeRTOSConfig.h */
#ifndef configGENERATE_RUN_TIME_STATS
#error "configGENERATE_RUN_TIME_STATS should be defined in FreeRTOSConfig.h"
#endif

#ifndef configUSE_STATS_FORMATTING_FUNCTIONS
#error "configUSE_STATS_FORMATTING_FUNCTIONS should be defined in FreeRTOSConfig.h"
#endif

/* Buffer size for the runtime stats */
#define RUNTIME_STATS_BUFFER_SIZE  1024

/* Task handles */
TaskHandle_t Task1Handle, Task2Handle, TaskEmptyHandle;

/* Task function prototypes */
void SimpleCounter();
void Task2();

int demoStats(void){
    /* Create tasks */
    xTaskCreate(SimpleCounter, "Task1", configMINIMAL_STACK_SIZE, NULL, 1, &Task1Handle);
    xTaskCreate(Task2, "Task2", configMINIMAL_STACK_SIZE, NULL, 1, &Task2Handle);

    /* Starting FreeRTOS scheduler */
    vTaskStartScheduler();

    for(;;);

    return 0;
}

void SimpleCounter(){
        int i = 0;
        while(1){
            printf("Task1 Counter: %d\n", i);
            i++;
            vTaskDelay(pdMS_TO_TICKS(1000));
            if (i >= 99999) {
                i = 0;
        }
    }
}

void TaskEmpty() {
    vTaskDelay(pdMS_TO_TICKS(10000));
    vTaskDelete(NULL);
}

void Task2(){
    while(1){
        /* Wait for a period to gather sufficient runtime statistics */
        vTaskDelay(pdMS_TO_TICKS(5000));

        size_t freeHeapSize = xPortGetFreeHeapSize();
        printf("Free heap size: %u bytes\n", freeHeapSize);
        xTaskCreate(TaskEmpty, "TaskEmpty", configMINIMAL_STACK_SIZE, NULL, 1, &TaskEmptyHandle);
    }
}
