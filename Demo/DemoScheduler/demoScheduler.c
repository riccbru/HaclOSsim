#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include <stdlib.h>

#define PATIENT_NUMBER 4

typedef struct {
    int patientCode;            // Unique identifier for the patient
    int arrivalTime;            // Time when the patient arrived
    int operationDuration;      // Expected duration of the operation 
    int criticalTime;           // Time limit after which the patient's condition worsens
    int priority;               // Could be assigned
} PatientInfo_t;

PatientInfo_t patients[PATIENT_NUMBER] = {
    {1, 2, 3, 9, 0},
    {2, 2, 4, 8, 0}, 
    {3, 6, 4, 12, 0},  
    {4, 8, 1, 10, 0}
};
int ids[] = {0, 1, 2};

void taskPazient(void *pvParameter) {
    PatientInfo_t *patient = (PatientInfo_t *)pvParameter;
    TickType_t xStart = xTaskGetTickCount();
    TickType_t xDelay = patient->operationDuration * configTICK_RATE_HZ;

    printf("    Pazient: %d Starting operation at:%d\n",patient->patientCode,xStart/configTICK_RATE_HZ);
    // Busy wating cycle
    TickType_t xCurrentTick = xTaskGetTickCount();
    while(xCurrentTick < xDelay + xStart) {
        if(patient->criticalTime * configTICK_RATE_HZ <= xCurrentTick) {
            printf("    Pazient: %d died at %d\n",patient->patientCode,xCurrentTick/configTICK_RATE_HZ);
            vTaskDelete(NULL);
        }
        xCurrentTick = xTaskGetTickCount();
    }

    printf("    Pazient: %d Ending operation:%d\n",patient->patientCode,xTaskGetTickCount()/configTICK_RATE_HZ);    
    vTaskDelete(NULL);
}

void taskArrival(void *pvParameter) {
    (void) pvParameter;
    int startIndex = 0;
    TickType_t xPeriod = 1 * configTICK_RATE_HZ;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    TaskHandle_t xTaskHandle;
    int index;

    while(startIndex < PATIENT_NUMBER) {

        for(index = startIndex; index < PATIENT_NUMBER; index++) {
            if(patients[index].arrivalTime * configTICK_RATE_HZ > xLastWakeTime) {
                break;
            }
            printf("Patient %d arrived at time %d\n", patients[index].patientCode, xLastWakeTime/configTICK_RATE_HZ);
            #if ( configUSE_POLLING_SERVER == 1 )
            xTaskCreateAperiodic( taskPazient,
                                "Patient Task",
                                (( unsigned short) 100),
                                (void *) &patients[index],
                                configMAX_PRIORITIES - patients[index].priority - 1,
                                patients[index].operationDuration,
                                patients[index].criticalTime * configTICK_RATE_HZ,
                                &xTaskHandle);
            #else // ( configUSE_POLLING_SERVER == 1)
                xTaskCreate(taskPazient,
                        "Patient Task",
                        (( unsigned short ) 100 ),
                        &patients[index],
                        configMAX_PRIORITIES - patients[index].priority - 2,
                        &xTaskHandle);
            #endif // ( configUSE_POLLING_SERVER == 1)
        }
        if(index != startIndex) startIndex = index;

        xTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
    vTaskDelete(NULL);
}

int main_scheduler(){

    TaskHandle_t xTaskPeriodicHandle;
    #if ( configUSE_POLLING_SERVER == 1 )
        xTaskCreatePeriodic(taskArrival, "Task Arrival", (( unsigned short ) 100 ), NULL, configMAX_PRIORITIES - 2, 1*configTICK_RATE_HZ, &xTaskPeriodicHandle);
    #else // ( configUSE_POLLING_SERVER == 1 )
        xTaskCreate(taskArrival, "Task Arrival", (( unsigned short ) 100 ), NULL, configMAX_PRIORITIES - 2, &xTaskPeriodicHandle);
    #endif // ( configUSE_POLLING_SERVER == 1 )

    vTaskStartScheduler();
    for(;;);
}
