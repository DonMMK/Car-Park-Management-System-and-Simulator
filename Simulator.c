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
#include "sharedMemoryOperations.c"

#define SHARE_NAME "PARKING"
#define CAR_LIMIT 10
#define FIRE 1
// ------------------------------------ FUNCTION DECLERATIONS ------------------------------------- // 
int generateRandom(int lower, int upper);
void readFile(char *filename);
void printFile();
char *generatePlate(int probability);
char *randomPlate();
void initialiseSharedMemory(shared_memory_t shm);
void *spawnCar(void *args);
void *entranceSimulate(void *arg);
void *tempSensorSimulate(void *arg);

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
    pthread_t tempSensorSimulate_thread[LEVELS];
    
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

    for (i = 0; i < LEVELS; i++){
        int* z = malloc(sizeof(int));
        *z = i;
        pthread_create(&tempSensorSimulate_thread[i], NULL, &tempSensorSimulate, z);
    }
    
    // Join threads 
    pthread_join(carSpawner,NULL);
    
    for (i = 0; i < ENTRANCES; i++){
        pthread_join(entranceThread[i], NULL);
    }
    for (i = 0; i < LEVELS; i++){
        pthread_join(tempSensorSimulate_thread[i],NULL);
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
        plate = generatePlate(80);
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
        pthread_mutex_lock(&entranceQueueMutex[i]);
        strcpy(shm.data->entrance[i].LPRSensor.plate, entranceQueue[i].plateQueue[0]);
        pthread_mutex_unlock(&entranceQueueMutex[i]);
        pthread_mutex_unlock(&shm.data->entrance[i].LPRSensor.LPRmutex);

        // Wait 2ms before triggering LPR
        usleep(2000);

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

void *tempSensorSimulate(void *arg) {
    int i = *(int*) arg;
    int16_t temperature;
    int16_t currentTemp;

    for (;;) {
        usleep(2000);
        if (FIRE == 1) { // (Fixed temp fire detection data)
            // Generate temperatures to trigger fire alarm via Temps > 58 degrees
            temperature = (int16_t) generateRandom(58, 65);
            shm.data->level[i].tempSensor = temperature;
        }
        else if (FIRE == 2) { // (Rate-of-rise fire detection data)
            // Generate temperatures to trigger fire alarm via Rate-of-rise (Most recent temp >= 8 degrees hotter than 30th most recent)
            if (shm.data->level[i].tempSensor > 58){
                currentTemp = 24;
            }
            else {
                currentTemp = shm.data->level[i].tempSensor;
            }
            temperature = generateRandom(currentTemp, currentTemp + 2);
            shm.data->level[i].tempSensor = temperature;
        }
        else {
            // Generate normal temperatures to avoid setting off fire alarm
            temperature = (int16_t) 24;
            shm.data->level[i].tempSensor = temperature;
        }    
    }
}