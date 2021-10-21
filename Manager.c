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

#include "shm_ops.c"

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
    // pthread_t carThreads;
    // printf("Char stored entrace gate: %c \n", shm.data->entrance[0].gate.status);
    // printf("Char stored info sign: %c \n", shm.data->entrance[0].informationSign.display);
    // printf("Char stored fire alarm: %c \n", shm.data->level[0].fireAlarm);

    printf("Plate stored on entrance LPR 1: %s \n", shm.data->entrance[0].LPRSensor.plate);
    // printf("Plate stored on entrance LPR 2: %s \n", shm.data->entrance[1].LPRSensor.plate);
    // printf("Plate stored on entrance LPR 3: %s \n", shm.data->entrance[2].LPRSensor.plate);
    // printf("Plate stored on entrance LPR 4: %s \n", shm.data->entrance[3].LPRSensor.plate);
    // printf("Plate stored on entrance LPR 5: %s \n", shm.data->entrance[4].LPRSensor.plate);
    // pthread_create(&carThreads, NULL, entranceSimulate, NULL);
    // pthread_join(carThreads,NULL);

    
}

// --------------------------------------- HELPER FUNCTUONS --------------------------------------- // 

/**
 * This function acts as the GUI for the manager file system
 * the function displays the status of the capacity, Boomgate, signs, Temperature sensor
 * Alarm, Car park revenue. */
void StatusLPR(){
    printf("\n----------------------------------------------------------------");
    printf("\nCapacity:");
    printf("\nBoomgate:");
    printf("\nSigns");
    printf("\nTemperature");
    printf("\n----------------------------------------------------------------");
    printf("\n");
    return; //
}

void *entranceSimulate(void *arg){
    pthread_cond_wait(&shm.data->entrance[0].LPRSensor.LPRcond, &shm.data->entrance[0].LPRSensor.LPRmutex);
    printf("Completed wait\n");

    return 0;
}

