#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include <stdlib.h>

#define PATIENT 9

TaskHandle_t xSchedulerTaskHandle;
/* Patient data structure */
typedef struct {
    int patientCode;       // Unique identifier for the patient
    int arrivalTime;       // Time when the patient arrived
    int operationDuration; // Expected duration of the operation 
    int criticalTime;      // Time limit after which the patient's condition worsens
    int priority;          // Could be assigned
} PatientInfo_t;

PatientInfo_t patients[] = {
    {1, 2, 3, 9, 0},
    {2, 2, 4, 6, 0}, 
    {3, 6, 4, 6, 0},  
    {4, 8, 1, 2, 0}
};

int comparePatients(const void *a, const void *b) {
    PatientInfo_t *patientA = (PatientInfo_t *)a;
    PatientInfo_t *patientB = (PatientInfo_t *)b;
    return patientA->priority - patientB->priority;
}

void taskPazient(void *pvParameter) {
    PatientInfo_t *patient = (PatientInfo_t *)pvParameter;
    TickType_t xStart = xTaskGetTickCount();
    TickType_t xDelay = patient->operationDuration * configTICK_RATE_HZ;

    printf(" Patient: %d start operation:%d\n",patient->patientCode,xStart/configTICK_RATE_HZ);
    // Waiting loop
    while((xTaskGetTickCount() - xStart) < xDelay) {
        // Doing nothing, just waiting for some time to pass
    }

    printf(" Pazient: %d end operation:%d \n",patient->patientCode,xTaskGetTickCount()/configTICK_RATE_HZ);    
    // Notify the scheduler upon completion
    xTaskNotifyGive(xSchedulerTaskHandle);
    vTaskDelete(NULL);
}

void eliminaPazientePrimaDellOperazione(PatientInfo_t *patient,int patientCode, int patientNum){
    for(int i = 0; i < patientNum; i++){
        if(patient[i].patientCode == patientCode){
            for(int j = i; j< patientNum -1 ; j++)
                patient[j]=patient[j+1];
            break;
        }
    }
}

void eliminaPaziente(int patientCode, int patientNum){
    for(int i = 0; i < patientNum; i++){
        if(patients[i].patientCode == patientCode){
            for(int j = i; j< patientNum -1 ; j++)
                patients[j]=patients[j+1];
            break;
        }
    }
}

// Simulation of the hospital switchboard
void taskScheduler(void *pvParameter){
    int i;
    int j=0;
    int patientNum = 4;
    PatientInfo_t patientArrived[patientNum];

    // Starting time
    TickType_t xNow;

    while(1){

        // Until the current time is greater or equal than the patient arrival time 
            for(i = 0; i < patientNum; i++){
                xNow = xTaskGetTickCount();
                        // printf("Il primo paziente arriverà a %d ora sono le %d \n", patients[i].arrivalTime,xNow/configTICK_RATE_HZ );
                // Insert patient in a vector
                if((patients[i].arrivalTime * configTICK_RATE_HZ ) > xNow)
                    break;
                else{
                    patientArrived[j] = patients[i];
                    j++;
                }
            }

        // If a patient arrived
        if(j > 0){
            printf("At time %d, there are %d patient wating:\n",xNow/configTICK_RATE_HZ, j);

            // Compute the priority of the arrived patients
            for (int k = 0; k < j; k++) {
                patientArrived[k].priority = patientArrived[k].criticalTime - patientArrived[k].operationDuration + patientArrived[k].arrivalTime - xNow/configTICK_RATE_HZ;
                if(patientArrived[k].priority >= 0)
                    printf(" -  Patient %d will die if not managed until %d \n",patientArrived[k].patientCode,patientArrived[k].priority);
                else{
                    printf(" ALLERT: Patient %d died\n",patientArrived[k].patientCode);
                    eliminaPaziente(patientArrived[k].patientCode, patientNum);
                    eliminaPazientePrimaDellOperazione(patientArrived, patientArrived[k].patientCode, patientNum);
                    patientNum--;
                }
            }

            // Sorting the vector based on the priority
            qsort(patientArrived, j, sizeof(PatientInfo_t), comparePatients);

            // Taking the patient with the highest priority
            printf("Starting the operation of patient %d \n",patientArrived[0].patientCode);

            // Starting its patient task
            xTaskCreate(taskPazient, "Pazient", configMINIMAL_STACK_SIZE, (void *)&patientArrived[0], 3, NULL);    
            
            // Waiting for its end
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

            // Removing the patient from the vector
            eliminaPaziente(patientArrived[0].patientCode, patientNum);

            // Update number of waiting patient vector
            patientNum--;

            // Reset arrived patient counter
            j=0;
        }
    }
}

int demoHospital2(){

    // Creating the taskScheduler with the highest priority
    xTaskCreate(taskScheduler, "Scheduler", ( ( unsigned short ) 1000 ), NULL, configMAX_PRIORITIES - 1, &xSchedulerTaskHandle);
    vTaskStartScheduler();
    for(;;);
}

