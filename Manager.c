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
#include "carQueue.h"
#include "carStorage.h"

#define SHARE_NAME "PARKING"

// ------------------------------------ FUNCTION DECLERATIONS ------------------------------------- // 
void StatusLPR();
void *entranceSimulate(void *arg);
void *entranceLPR(void *args);
void *levelLPR(void *args);
void *levelController(void *args);
void *exitLPR(void *args);
void *exitController(void *args);
void *checkTimes(void *args);
double generateRandom(int lower, int upper);
double parkTime;

int determineLevel(void);

// --------------------------------------- PUBLIC VARIABLES --------------------------------------- // 
int LevelCapacity;
char BoomGateStatus;
char SignStatus;
char TempSensorStatus;
char AlarmStatus;
int CarparkRevenue;
shared_memory_t shm;
carQueue_t levelQueue;
carQueue_t exitQueue;
carStorage_t carStorage;
int levelCapacity;
double time_spent;
clock_t begin;
clock_t end;

// --------------------------------------------- MAIN --------------------------------------------- // 
int main()
{    
    // Initialise random seed
    time_t t;
    srand((unsigned) time(&t));
    
    time_spent = 0.0;
    begin = clock();

    levelCapacity = 0;
    usleep(1 * 1000000);
    create_shared_object_R(&shm, SHARE_NAME);

    pthread_t entranceLPR_thread;
    pthread_t levelLPR_thread;
    pthread_t levelController_thread;
    pthread_t exitLPR_thread;
    pthread_t exitController_thread;
    pthread_t checkTimes_thread;

    plateInit(&levelQueue); 
    plateInit(&exitQueue);     
    storageInit(&carStorage);
 
    pthread_create(&entranceLPR_thread, NULL, entranceLPR, NULL);
    pthread_create(&levelLPR_thread, NULL, levelLPR, NULL);
    pthread_create(&levelController_thread, NULL, levelController, NULL);
    pthread_create(&exitLPR_thread, NULL, exitLPR, NULL);
    pthread_create(&exitController_thread, NULL, exitController, NULL);
    pthread_create(&checkTimes_thread, NULL, checkTimes, NULL);

    pthread_join(entranceLPR_thread, NULL);
    pthread_join(levelLPR_thread, NULL);
    pthread_join(levelController_thread, NULL);
    pthread_join(exitLPR_thread, NULL);
    pthread_join(exitController_thread, NULL);
    pthread_join(checkTimes_thread, NULL);
}

// --------------------------------------- HELPER FUNCTUONS --------------------------------------- // 

void *entranceLPR(void *args){
    // Empty LPR
    printf("M - Before emptying LPR. Plate value is: %s\n", shm.data->entrance[0].LPRSensor.plate);

    pthread_mutex_lock(&shm.data->entrance[0].LPRSensor.LPRmutex);
    strcpy(shm.data->entrance[0].LPRSensor.plate, "000000");
    pthread_mutex_unlock(&shm.data->entrance[0].LPRSensor.LPRmutex);

    // When entrance LPR is free, signal the simulator thread asking for a plate
    printf("M - After emptying LPR. Plate value is: %s\n", shm.data->entrance[0].LPRSensor.plate);
    pthread_cond_signal(&shm.data->entrance[0].LPRSensor.LPRcond);

    usleep(1000);

    pthread_mutex_lock(&shm.data->entrance[0].LPRSensor.LPRmutex);
    while (!strcmp(shm.data->entrance[0].LPRSensor.plate, "000000")){
        pthread_cond_wait(&shm.data->entrance[0].LPRSensor.LPRcond, &shm.data->entrance[0].LPRSensor.LPRmutex);
    }
    pthread_mutex_unlock(&shm.data->entrance[0].LPRSensor.LPRmutex);

    printf("M - After recieiving data. Plate is: %s\n", shm.data->entrance[0].LPRSensor.plate);

    // Information sign stuff here. For now, send to level 1.

    // Boomgate open/closed? Timings 

    // Add plate to LPR queue
    addPlate(&levelQueue, shm.data->entrance[0].LPRSensor.plate);

    // Add car to storage 
    end = clock();
    parkTime = generateRandom(100,10000)/1000;
    time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Time spent in carpark is %f while exit time will be %f and time_spent is %f\n", parkTime, time_spent + parkTime, time_spent);
    addCar(&carStorage, shm.data->entrance[0].LPRSensor.plate, parkTime + time_spent, 0);
}

void *levelLPR(void *args){
    for (;;){ 
        // Empty LPR
        printf("LLRP - Before emptying LPR. Plate value is: %s\n", shm.data->level[0].LPRSensor.plate);

        pthread_mutex_lock(&shm.data->level[0].LPRSensor.LPRmutex);
        strcpy(shm.data->level[0].LPRSensor.plate, "000000");
        pthread_mutex_unlock(&shm.data->level[0].LPRSensor.LPRmutex);

        // When level LPR is free, signal the simulator thread asking for a plate
        printf("LLPR - After emptying LPR. Plate value is: %s\n", shm.data->level[0].LPRSensor.plate);
        pthread_cond_signal(&shm.data->level[0].LPRSensor.LPRcond);

        usleep(1000);

        pthread_mutex_lock(&shm.data->level[0].LPRSensor.LPRmutex);
        while (!strcmp(shm.data->level[0].LPRSensor.plate, "000000")){
            pthread_cond_wait(&shm.data->level[0].LPRSensor.LPRcond, &shm.data->level[0].LPRSensor.LPRmutex);
        }
        pthread_mutex_unlock(&shm.data->level[0].LPRSensor.LPRmutex);

        printf("LLRP - After recieiving data. Plate is: %s\n", shm.data->level[0].LPRSensor.plate);

        // Check if its the cars first or second time passing through level LPR
        if (carStorage.car[findIndex(&carStorage,shm.data->level[0].LPRSensor.plate)].LPRcount == 0) {
            // Send car to parking spot (Add to capacity)
            printf("LLRP - Cars 1st time in parking lot\n");
            carStorage.car[findIndex(&carStorage,shm.data->level[0].LPRSensor.plate)].LPRcount++;
            levelCapacity++;
        }
        else {
            printf("LLRP - CAR LEAVING THE PARKING LOT\n");
            levelCapacity--;
            addPlate(&exitQueue,shm.data->level[0].LPRSensor.plate);
        }
    }
}

void *levelController(void *args){
    for (;;){
        printf("LC - Waiting for clear level LPR. Plate value is: %s\n", shm.data->level[0].LPRSensor.plate);
        // Wait for manager LPR thread to signal that LPR is free

        pthread_mutex_lock(&shm.data->level[0].LPRSensor.LPRmutex);
        while (strcmp(shm.data->level[0].LPRSensor.plate, "000000")){ 
            printf("in loop\n");
            pthread_cond_wait(&shm.data->level[0].LPRSensor.LPRcond, &shm.data->level[0].LPRSensor.LPRmutex);
        }
        pthread_mutex_unlock(&shm.data->level[0].LPRSensor.LPRmutex);
        printf("LC - Level LPR is clear. Value in there is: %s\n", shm.data->level[0].LPRSensor.plate);

        // Make sure plate is in queue
        while(levelQueue.size <= 0);

        // Copy a plate into memory
        pthread_mutex_lock(&shm.data->level[0].LPRSensor.LPRmutex);
        strcpy(shm.data->level[0].LPRSensor.plate, levelQueue.plateQueue[0]);
        pthread_mutex_unlock(&shm.data->level[0].LPRSensor.LPRmutex);
        popPlate(&levelQueue);

        // Signal manager thread with new plate
        pthread_cond_signal(&shm.data->level[0].LPRSensor.LPRcond); 
    }
}

void *exitLPR(void *args){
   // Empty LPR
    printf("ELPR - Before emptying LPR. Plate value is: %s\n", shm.data->exit[0].LPRSensor.plate);

    pthread_mutex_lock(&shm.data->exit[0].LPRSensor.LPRmutex);
    strcpy(shm.data->exit[0].LPRSensor.plate, "000000");
    pthread_mutex_unlock(&shm.data->exit[0].LPRSensor.LPRmutex);

    // When exit LPR is free, signal the simulator thread asking for a plate
    printf("ELPR - After emptying LPR. Plate value is: %s\n", shm.data->exit[0].LPRSensor.plate);
    pthread_cond_signal(&shm.data->exit[0].LPRSensor.LPRcond);

    usleep(1000);

    pthread_mutex_lock(&shm.data->exit[0].LPRSensor.LPRmutex);
    while (!strcmp(shm.data->exit[0].LPRSensor.plate, "000000")){
        pthread_cond_wait(&shm.data->exit[0].LPRSensor.LPRcond, &shm.data->exit[0].LPRSensor.LPRmutex);
    }
    pthread_mutex_unlock(&shm.data->exit[0].LPRSensor.LPRmutex);

    printf("ELPR - After recieiving data. Plate is: %s\n", shm.data->exit[0].LPRSensor.plate);
}


void *exitController(void *args){
    printf("EC - Waiting for clear exit exit. Plate value is: %s\n", shm.data->exit[0].LPRSensor.plate);
    // Wait for manager LPR thread to signal that LPR is free

    pthread_mutex_lock(&shm.data->exit[0].LPRSensor.LPRmutex);
    while (strcmp(shm.data->exit[0].LPRSensor.plate, "000000")){ 
        pthread_cond_wait(&shm.data->exit[0].LPRSensor.LPRcond, &shm.data->exit[0].LPRSensor.LPRmutex);
    }
    pthread_mutex_unlock(&shm.data->exit[0].LPRSensor.LPRmutex);
    printf("EC - Exit LPR is clear. Value in there is: %s\n", shm.data->exit[0].LPRSensor.plate);

    // Make sure plate is in queue
    while(exitQueue.size <= 0);

    // Copy a plate into memory
    pthread_mutex_lock(&shm.data->exit[0].LPRSensor.LPRmutex);
    strcpy(shm.data->exit[0].LPRSensor.plate, exitQueue.plateQueue[0]);
    pthread_mutex_unlock(&shm.data->exit[0].LPRSensor.LPRmutex);
    popPlate(&exitQueue);

    // Signal manager thread with new plate
    pthread_cond_signal(&shm.data->exit[0].LPRSensor.LPRcond);
}

void *checkTimes(void *args){
    for(;;){
        for (int i = 0; i < carStorage.size; i++){
            end = clock();
            time_spent = 0.0;
            time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
            if (carStorage.car[i].exitTime <= time_spent && carStorage.car[i].exitStatus != true){
                // printf("end time is %f and time spent is %f\n",end,time_spent);
                // wait with car somewhere and then get it back in level queue when done queue
                addPlate(&levelQueue,carStorage.car[i].plate);
                carStorage.car[i].exitStatus = true;
            }
        }
    }
}

// // Determines a level for the car to go to
// int determineLevel(void) {
//     int level = 0;
    
//     for (int i = 1; i <= LEVELS; i++) {
//         if (levelList.size < CAPACITY) {
//             level = i;
//         }
//         else {
//             level = 0;
//         }
//     }
//     return level;
// }

// void entranceBoomgate(void *args) {
//     // After checking its a valid car and 
//     if (true) {// ITS AN ALLOWED CAR AND THERES SPACE IN CARPARK
//         pthread_mutex_lock(&shm.data->entrance[0].gate.gatemutex);
//         shm.data->entrance[0].gate.status = 'O';
//         pthread_mutex_unlock(&shm.data->entrance[0].gate.gatemutex);
//     }
//     else {

//     }
// }

// void exitBoomgate(void *args) {
//     // If this thread is signalled that a car has been send to exit and has a plate in its exit.LPR.plate
//     // Open and baddabing baddaboom.

//     // If boom gates threads are broadcast to in fire emergency, set all gate.status to 'O'.
// }

// Generates random numbers in range [lower, upper]. 
// https://www.geeksforgeeks.org/generating-random-number-range-c/?fbclid=IwAR1a4I7mqxidG7EHit34MRmTLgge9xMfBQtw8TcCXVlYC9_QqrATtfESm94
double generateRandom(int lower, int upper)
{    
    double num = (rand() % (upper - lower + 1)) + lower;
    return num;
}