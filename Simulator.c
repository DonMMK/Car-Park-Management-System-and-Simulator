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

#include "carQueue.c"
#include "sharedMemoryOperations.h"

#define SHARE_NAME "PARKING"
#define CAR_LIMIT 5
// ------------------------------------ FUNCTION DECLERATIONS ------------------------------------- // 
int generateRandom(int lower, int upper);
void readFile(char *filename);
void printFile();
char *generatePlate(int probability);
char *randomPlate();
void initialiseSharedMemory(shared_memory_t shm);
void *spawnCar(void *args);
void *entranceSimulate(void *arg);

// --------------------------------------- PUBLIC VARIABLES --------------------------------------- // 
char allowedPlates[100][7];
shared_memory_t shm;
carQueue_t entranceQueue[ENTRANCES];
pthread_mutex_t entranceQueueMutex[ENTRANCES];
int selector;

// --------------------------------------------- MAIN --------------------------------------------- // 
int main()
{
    selector = 0;
    pthread_t carSpawner;
    pthread_t entranceThread[ENTRANCES];

    
    for (int i = 0; i < ENTRANCES; i++){
        pthread_mutex_init(&entranceQueueMutex[ENTRANCES], NULL);
    }
    
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
    int i;
    pthread_create(&carSpawner, NULL, &spawnCar, NULL);
    for (i = 0; i < ENTRANCES; i++){
        int* p = malloc(sizeof(int));
        *p = i;
        pthread_create(&entranceThread[i], NULL, &entranceSimulate, p);
    }
    
    // Join threads 
    pthread_join(carSpawner,NULL);
    for (i = 0; i < ENTRANCES; i++){
        pthread_join(entranceThread[i], NULL);
    }
}


// ------------------------------------------- THREADS ------------------------------------------- // 
void *spawnCar(void *args) {
    char* plate;  
    int waitTime;
    for (int i = 0; i < ENTRANCES; i++){
        plateInit(&entranceQueue[i]);
    }
    
    for (int i = 0;i < CAR_LIMIT;i++){
        // Generate numberplate (from list/random)
        plate = generatePlate(100);
        selector++;
        // printf("S - Car has plate number: %s\n", plate); 

        // Generate car every 1 - 100 milliseconds
        waitTime = generateRandom(1,100) * 1000;
        usleep(waitTime);

        int entranceCar = generateRandom(0,4);
        // entranceCar = 0;

        printf("The plate %s is arriving at entrance %d\n",plate,entranceCar + 1);
        // SPAWN CAR THREAD 
        pthread_mutex_lock(&entranceQueueMutex[entranceCar]);
        addPlate(&entranceQueue[entranceCar], plate);
        pthread_mutex_unlock(&entranceQueueMutex[entranceCar]);
    }
} 

void *entranceSimulate(void *arg) {
    int i = *(int*) arg;
    for (;;){
        // Wait for manager LPR thread to signal that LPR is free
        pthread_mutex_lock(&shm.data->entrance[i].LPRSensor.LPRmutex);
        while (strcmp(shm.data->entrance[i].LPRSensor.plate, "000000")){ 
            pthread_cond_wait(&shm.data->entrance[i].LPRSensor.LPRcond, &shm.data->entrance[i].LPRSensor.LPRmutex);
        }
        pthread_mutex_unlock(&shm.data->entrance[i].LPRSensor.LPRmutex);
        // LRP has been cleared 
        
        // Make sure plate is in queue
        while(entranceQueue[i].size <= 0);

        // Copy a plate into memory
        pthread_mutex_lock(&shm.data->entrance[i].LPRSensor.LPRmutex);
        strcpy(shm.data->entrance[i].LPRSensor.plate, entranceQueue[i].plateQueue[0]);
        pthread_mutex_unlock(&shm.data->entrance[i].LPRSensor.LPRmutex);

        // Signal manager thread with new plate
        pthread_cond_signal(&shm.data->entrance[i].LPRSensor.LPRcond);
        pthread_mutex_lock(&entranceQueueMutex[i]);
        popPlate(&entranceQueue[i]);
        pthread_mutex_unlock(&entranceQueueMutex[i]);
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
        // printf("SimGP - It is an allowed plate (%s)\n", allowedPlates[selector]);
        return allowedPlates[selector];
    }
    else{
        char *p = randomPlate();
        // printf("SimGP - It is a random plate (%s)\n", p);
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