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
void *entranceLPR(void *arg);
void *levelLPR(void *arg);
void *levelController(void *arg);
void *exitLPR(void *arg);
void *exitController(void *arg);
void *checkTimes(void *arg);
void *entranceBoomgate(void *arg);
void *exitBoomgate(void *arg);
void *informationSign(void *arg);

double generateRandom(int lower, int upper);
double createThreads();
void generateBill(char*);
void readFile(char *filename);


int determineLevel(void);

// --------------------------------------- PUBLIC VARIABLES --------------------------------------- // 
shared_memory_t shm;
carQueue_t levelQueue[LEVELS];
carQueue_t exitQueue[EXITS];
carStorage_t carStorage;

pthread_mutex_t levelQueueMutex[LEVELS];
pthread_mutex_t exitQueueMutex[EXITS];
pthread_mutex_t carStorageMutex;

int levelCapacity[LEVELS];
char allowedPlates[100][7];
char statusDisp[ENTRANCES];
char exitDisplay[EXITS];
char entrDisplay[ENTRANCES];
double bill;

// --------------------------------------------- MAIN --------------------------------------------- // 
int main()
{    
    // Initialise random seed
    time_t t;
    srand((unsigned) time(&t));

    // Read the number plates 
    readFile("plates.txt");

    create_shared_object_R(&shm, SHARE_NAME);
    
    createThreads();
}

// --------------------------------------- HELPER FUNCTUONS --------------------------------------- // 

void *entranceLPR(void *arg){
    int i = *(int*) arg;
    for (;;){ 
        // Empty LPR
        pthread_mutex_lock(&shm.data->entrance[i].LPRSensor.LPRmutex);
        strcpy(shm.data->entrance[i].LPRSensor.plate, "000000");
        pthread_mutex_unlock(&shm.data->entrance[i].LPRSensor.LPRmutex);

        // When entrance LPR is free, signal the simulator thread asking for a plate
        pthread_cond_signal(&shm.data->entrance[i].LPRSensor.LPRcond);

        pthread_mutex_lock(&shm.data->entrance[i].LPRSensor.LPRmutex);
        while (!strcmp(shm.data->entrance[i].LPRSensor.plate, "000000")){
            pthread_cond_wait(&shm.data->entrance[i].LPRSensor.LPRcond, &shm.data->entrance[i].LPRSensor.LPRmutex);
        }
        pthread_mutex_unlock(&shm.data->entrance[i].LPRSensor.LPRmutex);
        // Plate recieved 
        // printf("NLRP - After recieiving data. Plate is: %s\n", shm.data->entrance[0].LPRSensor.plate);

        // Information sign stuff here. For now, send to level 1.
            
        // Set information sign display to 0 to show its ready
        pthread_mutex_lock(&shm.data->entrance[i].informationSign.ISmutex);
        shm.data->entrance[i].informationSign.display = 'O';
        pthread_mutex_unlock(&shm.data->entrance[i].informationSign.ISmutex);

        // Signal info sign to change value on display
        pthread_cond_signal(&shm.data->entrance[i].informationSign.IScond);
        
        // Wait for response from information sign
        pthread_mutex_lock(&shm.data->entrance[i].informationSign.ISmutex);
        while (shm.data->entrance[i].informationSign.display == 'O') {
            pthread_cond_wait(&shm.data->entrance[i].informationSign.IScond, &shm.data->entrance[i].informationSign.ISmutex);
        }
        pthread_mutex_unlock(&shm.data->entrance[i].informationSign.ISmutex);

        // printf("NLPR - Value shown on display is %c\n", shm.data->entrance[0].informationSign.display);

        pthread_mutex_lock(&shm.data->entrance[i].informationSign.ISmutex);       
        statusDisp[i] = shm.data->entrance[i].informationSign.display;
        pthread_mutex_unlock(&shm.data->entrance[i].informationSign.ISmutex);

        // NEED STATEMENT HERE TO CHECK WHAT THE DISPLAY CHARACTER IS AND WHETHER TO SEND INTO CARPARK OR TO YEET
        if (statusDisp[i] != 'F' && statusDisp[i] != 'X'){
            // Boomgate open/closed? Timings 
            pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
            entrDisplay[i] = shm.data->entrance[i].gate.status;
            pthread_mutex_unlock(&shm.data->entrance[i].gate.gatemutex);

            if (entrDisplay[i] != 'O')
            {
                pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
                shm.data->entrance[i].gate.status = 'R';
                pthread_mutex_unlock(&shm.data->entrance[i].gate.gatemutex);

                pthread_cond_signal(&shm.data->entrance[i].gate.gatecond);
                pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
                while(shm.data->entrance[i].gate.status != 'O') {
                    pthread_cond_wait(&shm.data->entrance[i].gate.gatecond, &shm.data->entrance[i].gate.gatemutex);
                }
                pthread_mutex_unlock(&shm.data->entrance[i].gate.gatemutex);

            }
            // Add plate to LPR queues
            pthread_mutex_lock(&levelQueueMutex[i]);
            addPlate(&levelQueue[i], shm.data->entrance[i].LPRSensor.plate);
            pthread_mutex_unlock(&levelQueueMutex[i]);

            clock_t parkTime = (clock_t) generateRandom(100,10000) * 1000;
            printf("NLPR - Time spent parked will be %0.2f\n", (double)parkTime/CLOCKS_PER_SEC);
            // printf("NLPR - Car will be heading to floor %d\n", i + 1);

            // printf("NLPR - Entrance time spent parked will be %f\n", entranceTime);

            pthread_mutex_lock(&carStorageMutex);
            addCar(&carStorage, shm.data->entrance[i].LPRSensor.plate, clock(), parkTime,i);
            pthread_mutex_unlock(&carStorageMutex);
        }
        else{
            printf("REJECTED!\n");
        }
    }
}

void *levelLPR(void *arg){
    int i = *(int*) arg;
    for (;;){ 
        while(levelQueue[i].size <= 0);
        // Empty LPR
        pthread_mutex_lock(&shm.data->level[i].LPRSensor.LPRmutex);
        strcpy(shm.data->level[i].LPRSensor.plate, "000000");
        pthread_mutex_unlock(&shm.data->level[i].LPRSensor.LPRmutex);

        // When level LPR is free, signal the simulator thread asking for a plate
        pthread_cond_signal(&shm.data->level[i].LPRSensor.LPRcond);

        pthread_mutex_lock(&shm.data->level[i].LPRSensor.LPRmutex);
        while (!strcmp(shm.data->level[i].LPRSensor.plate, "000000")){
            pthread_cond_wait(&shm.data->level[i].LPRSensor.LPRcond, &shm.data->level[i].LPRSensor.LPRmutex);
        }
        pthread_mutex_unlock(&shm.data->level[i].LPRSensor.LPRmutex);
 
        // Plate recieved 
        // printf("LLRP - After recieiving data. Plate is: %s\n", shm.data->level[0].LPRSensor.plate);

        // Check if its the cars first or second time passing through level LPR
        if (carStorage.car[findIndex(&carStorage,shm.data->level[i].LPRSensor.plate)].LPRcount == 0) {
            // Send car to parking spot (Add to capacity)
            // printf("LLRP - Cars 1st time in parking lot\n");
            carStorage.car[findIndex(&carStorage,shm.data->level[i].LPRSensor.plate)].LPRcount++;
            levelCapacity[i]++;
        }
        else {
            // printf("LLRP - CAR %s LEAVING THE PARKING LOT\n", shm.data->level[0].LPRSensor.plate);
            pthread_mutex_lock(&exitQueueMutex[i]);
            addPlate(&exitQueue[i],shm.data->level[i].LPRSensor.plate);
            pthread_mutex_unlock(&exitQueueMutex[i]);
            // printf("Exit queue now: %d\n", exitQueue.size);
            levelCapacity[i]--;
        }
    }
}


void *exitLPR(void *arg){
    int i = *(int*) arg;
    for (;;){ 
        while(exitQueue[i].size <= 0);
        // Empty LPR
        pthread_mutex_lock(&shm.data->exit[i].LPRSensor.LPRmutex);
        strcpy(shm.data->exit[i].LPRSensor.plate, "000000");
        pthread_mutex_unlock(&shm.data->exit[i].LPRSensor.LPRmutex);

        // printf("I AM IN EXIT FUNCTION 2\n");

        // When exit LPR is free, signal the simulator thread asking for a plate
        pthread_cond_signal(&shm.data->exit[i].LPRSensor.LPRcond);

        pthread_mutex_lock(&shm.data->exit[i].LPRSensor.LPRmutex);
        while (!strcmp(shm.data->exit[i].LPRSensor.plate, "000000")){
            pthread_cond_wait(&shm.data->exit[i].LPRSensor.LPRcond, &shm.data->exit[i].LPRSensor.LPRmutex);
        }
        pthread_mutex_unlock(&shm.data->exit[i].LPRSensor.LPRmutex);

        //printf("XLPR - After recieiving data. Plate is: %s\n", shm.data->exit[0].LPRSensor.plate);
        // Boomgate 
        pthread_mutex_lock(&shm.data->exit[i].gate.gatemutex);
        exitDisplay[i] = shm.data->exit[i].gate.status;
        pthread_mutex_unlock(&shm.data->exit[i].gate.gatemutex);

        if (exitDisplay[i] != 'O')
        {
            pthread_mutex_lock(&shm.data->exit[i].gate.gatemutex);
            shm.data->exit[i].gate.status = 'R';
            pthread_mutex_unlock(&shm.data->exit[i].gate.gatemutex);

            pthread_cond_signal(&shm.data->exit[i].gate.gatecond);
            pthread_mutex_lock(&shm.data->exit[i].gate.gatemutex);
            while(shm.data->exit[i].gate.status != 'O') {
                pthread_cond_wait(&shm.data->exit[i].gate.gatecond, &shm.data->exit[i].gate.gatemutex);
            }
            pthread_mutex_unlock(&shm.data->exit[i].gate.gatemutex);
        }
        // Bill the car
        pthread_mutex_lock(&carStorageMutex);
        generateBill(carStorage.car[findIndex(&carStorage, shm.data->exit[i].LPRSensor.plate)].plate); 
        pthread_mutex_unlock(&carStorageMutex);
        // Remove from system 
        pthread_mutex_lock(&carStorageMutex);
        removeCar(&carStorage, shm.data->exit[i].LPRSensor.plate);
        pthread_mutex_unlock(&carStorageMutex);

    }
}

void *exitController(void *arg){
    int i = *(int*) arg;
    for (;;){
        // printf("I AM IN exit contorller part 1\n");
        // Wait for exit LPR thread to signal that LPR is free
        pthread_mutex_lock(&shm.data->exit[i].LPRSensor.LPRmutex);
        while (strcmp(shm.data->exit[i].LPRSensor.plate, "000000")){ 
            pthread_cond_wait(&shm.data->exit[i].LPRSensor.LPRcond, &shm.data->exit[i].LPRSensor.LPRmutex);
        }
        pthread_mutex_unlock(&shm.data->exit[i].LPRSensor.LPRmutex);
        // LRP is clear 
        // printf("I AM IN exit contorller part 2\n");

        // Make sure plate is in queue
        while(exitQueue[i].size <= 0);

        // Copy a plate into memory
        pthread_mutex_lock(&shm.data->exit[i].LPRSensor.LPRmutex);
        strcpy(shm.data->exit[i].LPRSensor.plate, exitQueue[i].plateQueue[i]);
        pthread_mutex_unlock(&shm.data->exit[i].LPRSensor.LPRmutex);
        
        pthread_mutex_lock(&exitQueueMutex[i]);
        popPlate(&exitQueue[i]);
        pthread_mutex_unlock(&exitQueueMutex[i]);

        // Signal manager thread with new plate
        pthread_cond_signal(&shm.data->exit[i].LPRSensor.LPRcond);
    }
}

void *levelController(void *arg){
    int i = *(int*) arg;
    for (;;){
        // Wait for manager LPR thread to signal that LPR is free
        pthread_mutex_lock(&shm.data->level[i].LPRSensor.LPRmutex);
        while (strcmp(shm.data->level[i].LPRSensor.plate, "000000")){ 
            pthread_cond_wait(&shm.data->level[i].LPRSensor.LPRcond, &shm.data->level[i].LPRSensor.LPRmutex);
        }
        pthread_mutex_unlock(&shm.data->level[i].LPRSensor.LPRmutex);
        // LRP is now clear

        // Make sure plate is in queue
        while(levelQueue[i].size <= 0);

        // Copy a plate into memory
        pthread_mutex_lock(&shm.data->level[i].LPRSensor.LPRmutex);
        strcpy(shm.data->level[i].LPRSensor.plate, levelQueue[i].plateQueue[i]);
        pthread_mutex_unlock(&shm.data->level[i].LPRSensor.LPRmutex);
        
        pthread_mutex_lock(&levelQueueMutex[i]);
        popPlate(&levelQueue[i]);
        pthread_mutex_unlock(&levelQueueMutex[i]);

        // Signal manager thread with new plate
        pthread_cond_signal(&shm.data->level[i].LPRSensor.LPRcond); 
    }
}

void *checkTimes(void *arg){
    int i = *(int*) arg;
    for(;;){
        while (carStorage.size <= 0);
        for (int i = 0; i < carStorage.size; i++){
            if (carStorage.car[i].exitStatus != true && (double) carStorage.car[i].entranceTime + carStorage.car[i].parkTime <= (double) clock()){
                // printf("end time is %0.2f and current time is %f\n",exitTime,currentTime);
                // printf("exit time of %d is larger than %d \n", exitTime, currentTime);
                // wait with car somewhere and then get it back in level queue when done queue
                pthread_mutex_lock(&levelQueueMutex[carStorage.car[i].level]);
                addPlate(&levelQueue[carStorage.car[i].level],carStorage.car[i].plate);
                pthread_mutex_unlock(&levelQueueMutex[carStorage.car[i].level]);
                carStorage.car[i].exitStatus = true;
            }
        }
    }
}

void *entranceBoomgate(void *arg) {
    int i = *(int*) arg;
    for (;;){
        pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
        while(shm.data->entrance[i].gate.status == 'C') {
            pthread_cond_wait(&shm.data->entrance[i].gate.gatecond, &shm.data->entrance[i].gate.gatemutex);
        }
        pthread_mutex_unlock(&shm.data->entrance[i].gate.gatemutex);

        // Set gate to raising
        pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
        shm.data->entrance[i].gate.status = 'R';
        pthread_mutex_unlock(&shm.data->entrance[i].gate.gatemutex);
        // printf("NG - Gate status set to %s\n", &shm.data->entrance[0].gate.status);
    
        // Wait 10 ms
        usleep(10000);
        // Set gate to open
        pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
        shm.data->entrance[i].gate.status = 'O';
        pthread_mutex_unlock(&shm.data->entrance[i].gate.gatemutex);
        // printf("NG - Gate status set to %s\n", &shm.data->entrance[0].gate.status);
        
        // Signal gate is open
        pthread_cond_signal(&shm.data->entrance[i].gate.gatecond);

        // Wait for car to automatically close gate
        usleep(20000);
        
        // Set gate to lowering
        pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
        shm.data->entrance[i].gate.status = 'L';
        pthread_mutex_unlock(&shm.data->entrance[i].gate.gatemutex);
        // printf("NG - Gate status set to %s\n", &shm.data->entrance[0].gate.status);
        usleep(10000);
        // Set gate to closed   
        pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
        shm.data->entrance[i].gate.status = 'C';
        pthread_mutex_unlock(&shm.data->entrance[i].gate.gatemutex);
        // printf("NG - Gate status set to %s\n", &shm.data->entrance[0].gate.status);
    }
}

void *exitBoomgate(void *arg) {
    int i = *(int*) arg;
    for (;;){
        // Waiting for status of boom gate to change 
        pthread_mutex_lock(&shm.data->exit[i].gate.gatemutex);
        while(shm.data->exit[i].gate.status == 'C') {
            pthread_cond_wait(&shm.data->exit[i].gate.gatecond, &shm.data->exit[i].gate.gatemutex);
        }
        pthread_mutex_unlock(&shm.data->exit[i].gate.gatemutex);
        // printf("XG - Gate status set to %s\n", &shm.data->exit[0].gate.status);
    
        // Wait 10 ms
        usleep(10000);
        // Set gate to open
        pthread_mutex_lock(&shm.data->exit[i].gate.gatemutex);
        shm.data->exit[i].gate.status = 'O';
        pthread_mutex_unlock(&shm.data->exit[i].gate.gatemutex);
        // printf("XG - Gate status set to %s\n", &shm.data->exit[0].gate.status);
        
        // Signal gate is open
        pthread_cond_signal(&shm.data->exit[i].gate.gatecond);

        // Wait for car to automatically close gate
        usleep(20000);
        
        // Set gate to lowering
        pthread_mutex_lock(&shm.data->exit[i].gate.gatemutex);
        shm.data->exit[i].gate.status = 'L';
        pthread_mutex_unlock(&shm.data->exit[i].gate.gatemutex);
        // printf("XG - Gate status set to %s\n", &shm.data->exit[0].gate.status);

        usleep(10000);
        // Set gate to closed
        pthread_mutex_lock(&shm.data->exit[i].gate.gatemutex);
        shm.data->exit[i].gate.status = 'C';
        pthread_mutex_unlock(&shm.data->exit[i].gate.gatemutex);
        // printf("XG - Gate status set to %s\n", &shm.data->exit[0].gate.status);
    }
}

// // Determines a level for the car to go to
int determineLevel(void) {
    int level = 0;
    for (int i = 1; i <= LEVELS; i++) {
        if (levelCapacity[i] < CAPACITY) {
            level = i;
            break;
            // Increment level capacity
        }
        else {
            level = 0; // in another func, if level = 0 display F?
        }
    }
    return level;
}

void *informationSign(void *arg) {
    int i = *(int*) arg;
    for (;;){
        bool allowed = false;
        char toDisplay;
        // Wait for information sign to be signalled
        pthread_mutex_lock(&shm.data->entrance[i].informationSign.ISmutex);
        while (shm.data->entrance[i].informationSign.display != 'O') {
            pthread_cond_wait(&shm.data->entrance[i].informationSign.IScond, &shm.data->entrance[i].informationSign.ISmutex);
        }
        pthread_mutex_unlock(&shm.data->entrance[i].informationSign.ISmutex);

        for (int j = 0; j < 100; j++) {
            if (strcmp(shm.data->entrance[i].LPRSensor.plate, allowedPlates[j])) { // if plate in LPR or entrance queue?? is not in allowed plates then display 'X' and yeet the plate
                //printf("infoSign - Plate is NOT GABBA\n");
                toDisplay = 'X';
                allowed = false;
            }    
            else {          
                //printf("infoSign - Plate is in fact GABBA\n");
                allowed = true;
                break;
            }
        }

        if (allowed) {
            //toDisplay = (char)determineLevel() + '0';  UNCOMMENT 
            // FOR THE MOMENT TO TEST
            toDisplay = (char)generateRandom(0, LEVELS) + '0';
            toDisplay = '1';
            //printf("infoSign - Randomly Generated number = %c\n", toDisplay);
            if (toDisplay == '0') {
                toDisplay = 'F';
            }
        }

        // Set information sign display to 'toDisplay' to show its ready
        pthread_mutex_lock(&shm.data->entrance[i].informationSign.ISmutex);
        shm.data->entrance[i].informationSign.display = toDisplay;
        pthread_mutex_unlock(&shm.data->entrance[i].informationSign.ISmutex);

        //Signal entrance LPR to say that we have changed the display
        pthread_cond_signal(&shm.data->entrance[i].informationSign.IScond);
    }

}

void generateBill(char* numPlate) {
    FILE *billingFile;
    bill = (double)(clock() - carStorage.car[findIndex(&carStorage,numPlate)].entranceTime)/ CLOCKS_PER_SEC * 50; // FOR NOW
    //double bill = (carStorage.car[findIndex(&carStorage, plate)].exitTime - entryTime) * 0.05; // Need to add delay timings and entryTime or time_spent to car struct?
    // printf("B - numplate = %s\n", numPlate);
    printf("B - %s bill $%.2f\n", numPlate, bill);


    if (billingFile = fopen("billing.txt", "r")) 
    {
        // printf("B - file exists\n");
        billingFile = fopen("billing.txt", "a");
        fprintf(billingFile, "%s  -  $%.2f\n", numPlate, bill);
        fclose(billingFile);
    }
    else
    {
        printf("B - file doesn't exist - creating billing.txt file...\n");
        billingFile = fopen("billing.txt", "w");
        fprintf(billingFile, "%s  -  $%.2f\n", numPlate, bill);
        fclose(billingFile);
    }
}


// Generates random numbers in range [lower, upper]. 
// https://www.geeksforgeeks.org/generating-random-number-range-c/?fbclid=IwAR1a4I7mqxidG7EHit34MRmTLgge9xMfBQtw8TcCXVlYC9_QqrATtfESm94
double generateRandom(int lower, int upper)
{    
    double num = (rand() % (upper - lower + 1)) + lower;
    return num;
}

double createThreads(){
    pthread_mutex_init(&carStorageMutex, NULL);
    storageInit(&carStorage);

    pthread_t entranceLPR_thread[ENTRANCES];
    pthread_t levelLPR_thread[LEVELS];
    pthread_t levelController_thread[LEVELS];
    pthread_t exitLPR_thread[EXITS];
    pthread_t exitController_thread[EXITS];
    pthread_t checkTimes_thread[LEVELS];
    pthread_t exitBoomgate_thread[EXITS];
    pthread_t entranceBoomgate_thread[ENTRANCES];
    pthread_t informationSign_thread[ENTRANCES];

    int i;
    for (i = 0; i < ENTRANCES; i++){
        int* a = malloc(sizeof(int));
        *a = i;
        pthread_create(&entranceLPR_thread[i], NULL, &entranceLPR, a);   
        pthread_create(&entranceBoomgate_thread[i],NULL, &entranceBoomgate, a);
        pthread_create(&informationSign_thread[i],NULL, &informationSign, a);
    }

    for (i = 0; i < EXITS; i++){
        int* b = malloc(sizeof(int));
        *b = i;        
        plateInit(&exitQueue[i]); 
        pthread_mutex_init(&exitQueueMutex[i], NULL);
        pthread_create(&exitLPR_thread[i], NULL, &exitLPR, b);
        pthread_create(&exitController_thread[i], NULL, &exitController, b);
        pthread_create(&exitBoomgate_thread[i], NULL, &exitBoomgate, b);
    }

    for (i = 0; i < LEVELS; i++){
        int* c = malloc(sizeof(int));
        *c = i;        
        plateInit(&levelQueue[i]);
        pthread_mutex_init(&levelQueueMutex[i], NULL);
        pthread_create(&checkTimes_thread[i], NULL, &checkTimes, c);
        pthread_create(&levelLPR_thread[i], NULL, &levelLPR, c);
        pthread_create(&levelController_thread[i], NULL, &levelController, c);
    }


    for (i = 0; i < ENTRANCES; i++){     
        pthread_join(entranceLPR_thread[i], NULL);
        pthread_join(entranceBoomgate_thread[i], NULL);
        pthread_join(informationSign_thread[i], NULL);
    }

    for (i = 0; i < EXITS; i++){
        pthread_join(exitLPR_thread[i], NULL);
        pthread_join(exitController_thread[i], NULL);
        pthread_join(exitBoomgate_thread[i], NULL);
    }

    for (i = 0; i < LEVELS; i++){
        pthread_join(checkTimes_thread[i], NULL);
        pthread_join(levelLPR_thread[i], NULL);
        pthread_join(levelController_thread[i], NULL);
    }
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