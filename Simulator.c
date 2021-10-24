// -------------------------------------------- HEADa -------------------------------------------- //
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
#include <ctype.h>
#include <stdint.h>

#include "carQueue.h"
#include "sharedMemoryOperations.h"

#define SHARE_NAME "PARKING"


// ------------------------------------ FUNCTION DECLERATIONS ------------------------------------- // 
int generateRandom(int lower, int upper);
void readFile(char *filename);
void printFile();
char *generatePlate(int probability);
char *randomPlate();
void initialiseSharedMemory(shared_memory_t shm);
void *spawnCar(void *args);
void *entranceSimulate(void *args);

// --------------------------------------- PUBLIC VARIABLES --------------------------------------- // 
char allowedPlates[100][7];
shared_memory_t shm;
carQueue_t entranceQueue;

// --------------------------------------------- MAIN --------------------------------------------- // 
int main()
{
    pthread_t carSpawner;
    pthread_t entranceThread;

    // Initialise random seed
    time_t t;
    srand((unsigned) time(&t));

    // Create shared memory object
    create_shared_object_RW(&shm, SHARE_NAME);

    // Initialise mutexs and condition variables in shared memory
    initialiseSharedMemory(shm);

    // Read the number plates 
    readFile("plates.txt");

    // BEGINING SIMULATION
    printf("S - STARTING SIMULATION\n");    

    // Create threads 
    pthread_create(&carSpawner, NULL, &spawnCar, NULL);
    pthread_create(&entranceThread, NULL, &entranceSimulate, NULL);

    // Join threads 
    pthread_join(carSpawner,NULL);
    pthread_join(entranceThread,NULL);
}


// ------------------------------------------- THREADS ------------------------------------------- // 
void *spawnCar(void *args) {
    int entrance; 
    char* plate;  
    int waitTime;

    plateInit(&entranceQueue);
    
    // Generate numberplate (from list/random)
    plate = generatePlate(80);
    printf("S - Car has plate number: %s\n", plate); 

    addPlate(&entranceQueue, plate);
} 

void *entranceSimulate(void *args) {
    // Wait for manager LPR thread to signal that LPR is free
    pthread_mutex_lock(&shm.data->entrance[0].LPRSensor.LPRmutex);
    while (strcmp(shm.data->entrance[0].LPRSensor.plate, "000000")){ 
        pthread_cond_wait(&shm.data->entrance[0].LPRSensor.LPRcond, &shm.data->entrance[0].LPRSensor.LPRmutex);
    }
    pthread_mutex_unlock(&shm.data->entrance[0].LPRSensor.LPRmutex);
    // LRP has been cleared 
    
    // Make sure plate is in queue
    while(entranceQueue.size <= 0);

    // Copy a plate into memory
    pthread_mutex_lock(&shm.data->entrance[0].LPRSensor.LPRmutex);
    strcpy(shm.data->entrance[0].LPRSensor.plate, entranceQueue.plateQueue[0]);
    pthread_mutex_unlock(&shm.data->entrance[0].LPRSensor.LPRmutex);
    popPlate(&entranceQueue);

    // Signal manager thread with new plate
    pthread_cond_signal(&shm.data->entrance[0].LPRSensor.LPRcond);
}


// --------------------------------------- HELPER FUNCTIONS --------------------------------------- // 
// Generates random numbers in range [lower, upper]. 
// https://www.geeksforgeeks.org/generating-random-number-range-c/?fbclid=IwAR1a4I7mqxidG7EHit34MRmTLgge9xMfBQtw8TcCXVlYC9_QqrATtfESm94
int generateRandom(int lower, int upper)
{    
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
}

// Reads contents of supplied file 
void readFile(char *filename){
    FILE* file = fopen(filename, "r");

    // Fill array
    int i = 0;
    while (fgets(allowedPlates[i], 10, file)) {
        allowedPlates[i][strlen(allowedPlates[i]) - 1] = '\0';
        i++;
    }
}

// Prints contents of numberplate file (USED FOR TESTING)
void printFile(){
	printf("\n The content of the file  are : \n");    
    for(int i = 0; i < 100; i++)
    {
        printf("%s, ", allowedPlates[i]);
    }
    printf("\n");
}

// generates a random numberplate 
char* generatePlate(int probability){
    int random = generateRandom(0, 100);
    if (random <= probability){
        int selector = generateRandom(0, 99);
        // printf("It is an allowed plate\n");
        return allowedPlates[selector];
    }
    else{
        // printf("It is a random plate\n");
        char *p = randomPlate();
        return p;
    }
}

// Constructs a random plate
char* randomPlate(){
    int first = generateRandom(0, 9);
    int second = generateRandom(0, 9);
    char third = generateRandom(0, 9);

    char randomletter1 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[generateRandom(0, 25)];
    char randomletter2 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[generateRandom(0, 25)];
    char randomletter3 = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[generateRandom(0, 25)];

    char *finstr = NULL;
    finstr = malloc(10);
    sprintf(&finstr[0], "%d", first);
    sprintf(&finstr[1], "%d", second);
    sprintf(&finstr[2], "%d", third);    
    finstr[3] = randomletter1;
    finstr[4] = randomletter2;
    finstr[5] = randomletter3;
    finstr[6] = '\0';

    return finstr;
}

