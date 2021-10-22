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
void *entranceSimulate(void *entranceNumber);

// --------------------------------------- PUBLIC VARIABLES --------------------------------------- // 
char allowedPlates[100][10];
shared_memory_t shm;
carQueue_t entranceQueue[ENTRANCES];


// --------------------------------------------- MAIN --------------------------------------------- // 
int main()
{
    pthread_t carSpawner;
    pthread_t entranceThread[ENTRANCES];

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
    printf("...STARTING SIMULATION...\n");    

    // Create threads 
    pthread_create(&carSpawner, NULL, &spawnCar, NULL);
    for (int i = 0;i < ENTRANCES;i++){
        int* a = malloc(sizeof(int));
        *a = i;
        pthread_create(&entranceThread[i], NULL, &entranceSimulate, a);
    }

    // Join threads 
    pthread_join(carSpawner,NULL);
    for (int i = 0;i < ENTRANCES;i++){
        // Join entrance threads

        pthread_join(entranceThread[i],NULL);
    }
}


// ------------------------------------------- THREADS ------------------------------------------- // 
void *spawnCar(void *args) {
    int entrance; 
    char* plate;  
    int waitTime;

    for (int i = 0; i < ENTRANCES; i++){
        plateInit(&entranceQueue[i]);
    }

    // Generate 10 cars (for now)
    for (int i = 0; i < 10; i++) {
        // Generate car every 1 - 100 milliseconds
        waitTime = generateRandom(1,100) * 1000;
        usleep(waitTime);

        // Spawn car at random entrance
        entrance = generateRandom(1,ENTRANCES);
        printf("Car arriving at entrance: %d\n", entrance);

        // Generate numberplate (from list/random)
        plate = generatePlate(80);
        printf("Car has plate number: %s\n", plate); 

        addPlate(&entranceQueue[entrance], plate);
    }
}

void *entranceSimulate(void *args) {
    int index = *(int*)args;
    while (1) {
        // Wait for manager LPR thread to signal that LPR is free
        pthread_mutex_lock(&shm.data->entrance[index].LPRSensor.LPRmutex);
        pthread_cond_wait(&shm.data->entrance[index].LPRSensor.LPRcond, &shm.data->entrance->LPRSensor.LPRmutex);
        pthread_mutex_unlock(&shm.data->entrance[index].LPRSensor.LPRmutex);
        
        // Make sure plate is in queue
        while(entranceQueue[index].size <= 0);

        // Copy a plate into memory
        pthread_mutex_lock(&shm.data->entrance[index].LPRSensor.LPRmutex);
        strcpy(shm.data->entrance[index].LPRSensor.plate, entranceQueue[index].plateQueue[0]);
        pthread_mutex_unlock(&shm.data->entrance[index].LPRSensor.LPRmutex);
        popPlate(&entranceQueue[index]);

        // Signal manager thread with new plate
        pthread_cond_signal(&shm.data->entrance[index].LPRSensor.LPRcond);
    }
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
        printf("It is an allowed plate\n");
        return allowedPlates[selector];
    }
    else{
        printf("It is a random plate\n");
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

