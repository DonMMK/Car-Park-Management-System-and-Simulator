// -------------------------------------------- HEADER -------------------------------------------- //
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

#include "sharedMemoryOperations.h"

#define SHARE_NAME "PARKING"

// ------------------------------------ FILE OVERVIEW --------------------------------------------- // 
/**
 *  Tasks of the Manager
 * 
 * 1. Monitor the status of the LPR (license plate recognition) sensor 
 * 1.1 Keep track where each car is <- linkedlist or array of cars
 * 
 * 2. Tell the boom gates to open and close +(automatic close after fixed time)
 * 
 * 3. Room allocation for cars
 * 3.1 Continous check for (number of cars < number of levels * number of cars per level)
 * 
 * 4. Keep track of how long car has been in park
 * 4.1 Calculate bill at the end
 * 
 * 5. Display current status (capacity of each level, current status of boom gate, signs, temperature sensor and alarms, revenue for carpark)
 */

/** Method of achieving tasks
 * 
 * 
 */

// ------------------------------------ STRUCTURE DECLERATIONS ------------------------------------ // 

// ------------------------------------ FUNCTION DECLERATIONS ------------------------------------- // 
void StatusLPR();
void *entranceSimulate(void *arg);

// --------------------------------------- PUBLIC VARIABLES --------------------------------------- // 
int LevelCapacity;
char BoomGateStatus;
char SignStatus;
char TempSensorStatus;
char AlarmStatus;
int CarparkRevenue;
shared_memory_t shm;


// --------------------------------------------- MAIN --------------------------------------------- // 
int main()
{    
    create_shared_object_R(&shm, SHARE_NAME);
    
    pthread_t entranceLPR_thread[ENTRANCES];

    for (int i = 0;i < ENTRANCES;i++){
        pthread_create(&entranceLPR_thread[i], NULL, entranceLPR, i);
    }

    for (int i = 0;i < ENTRANCES;i++){
        // Join entrance threads
        pthread_join(entranceLPR_thread[i],NULL);
    }
}

// --------------------------------------- HELPER FUNCTUONS --------------------------------------- // 

/**
 * This function acts as the GUI for the manager file system
 * the function displays the status of the capacity, Boomgate, signs, Temperature sensor
 * Alarm, Car park revenue. */
void StatusLPR(){
    printf("\n----------------------------------------------------------------");
    printf("\nCapacity      :");
    printf("\nBoomgate      :");
    printf("\nSigns         :");
    printf("\nTemperature   :");
    printf("\n----------------------------------------------------------------");
    printf("\n");
    return; //
}

void entranceLPR(int i){
    // When entrance LPR is free, signal the simulator thread asking for a plate
    while (1) {
        pthread_mutex_lock(&shm.data->entrance[i].LPRSensor.LPRmutex);
        pthread_cond_signal(&shm.data->entrance[i].LPRSensor.LPRcond);
        pthread_mutex_unlock(&shm.data->entrance[i].LPRSensor.LPRmutex);
        
        pthread_cond_wait(&shm.data->entrance[i].LPRSensor.LPRcond, &shm.data->entrance[i].LPRSensor.LPRmutex);
    }

    // 



    
    // pthread_mutex_lock(&shm.data->entrance[0].LPRSensor.LPRmutex);
    while(!pthread_cond_wait(&shm.data->entrance[0].LPRSensor.LPRcond, &shm.data->entrance[0].LPRSensor.LPRmutex))
    // pthread_mutex_unlock(&shm.data->entrance[0].LPRSensor.LPRmutex);
    printf("Completed wait\n");
    //printf("Plate stored on entrance LPR 1: %s \n", shm.data->entrance[0].LPRSensor.plate);
    return 0;
}

