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
void *entranceBoomgate(void *args);
void *exitBoomgate(void *args);
void *informationSign(void *args);

double generateRandom(int lower, int upper);
double createThreads();
void generateBill(char*);
void readFile(char *filename);


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
double parkTime;
char allowedPlates[100][7];
char statusDisp;

// --------------------------------------------- MAIN --------------------------------------------- // 
int main()
{    
    // Initialise random seed
    time_t t;
    srand((unsigned) time(&t));
    
    time_spent = 0.0;
    begin = clock();
    levelCapacity = 0;

    // Read the number plates 
    readFile("plates.txt");

    usleep(1 * 1000000);
    create_shared_object_R(&shm, SHARE_NAME);
    createThreads();
}

// --------------------------------------- HELPER FUNCTUONS --------------------------------------- // 

void *entranceLPR(void *args){
    for (;;){ 
        // Empty LPR
        pthread_mutex_lock(&shm.data->entrance[0].LPRSensor.LPRmutex);
        strcpy(shm.data->entrance[0].LPRSensor.plate, "000000");
        pthread_mutex_unlock(&shm.data->entrance[0].LPRSensor.LPRmutex);

        // When entrance LPR is free, signal the simulator thread asking for a plate
        pthread_cond_signal(&shm.data->entrance[0].LPRSensor.LPRcond);

        usleep(1000);

        pthread_mutex_lock(&shm.data->entrance[0].LPRSensor.LPRmutex);
        while (!strcmp(shm.data->entrance[0].LPRSensor.plate, "000000")){
            pthread_cond_wait(&shm.data->entrance[0].LPRSensor.LPRcond, &shm.data->entrance[0].LPRSensor.LPRmutex);
        }
        pthread_mutex_unlock(&shm.data->entrance[0].LPRSensor.LPRmutex);
        // Plate recieved 
        printf("NLRP - After recieiving data. Plate is: %s\n", shm.data->entrance[0].LPRSensor.plate);

        // Information sign stuff here. For now, send to level 1.
            
        // Set information sign display to 0 to show its ready
        pthread_mutex_lock(&shm.data->entrance[0].informationSign.ISmutex);
        shm.data->entrance[0].informationSign.display = 'O';
        pthread_mutex_unlock(&shm.data->entrance[0].informationSign.ISmutex);

        // Signal info sign to change value on display
        pthread_cond_signal(&shm.data->entrance[0].informationSign.IScond);
        
        // Wait for response from information sign
        pthread_mutex_lock(&shm.data->entrance[0].informationSign.ISmutex);
        while (shm.data->entrance[0].informationSign.display == 'O') {
            pthread_cond_wait(&shm.data->entrance[0].informationSign.IScond, &shm.data->entrance[0].informationSign.ISmutex);
        }
        pthread_mutex_unlock(&shm.data->entrance[0].informationSign.ISmutex);

        printf("NLPR - Value shown on display is %c\n", shm.data->entrance[0].informationSign.display);

        pthread_mutex_lock(&shm.data->entrance[0].informationSign.ISmutex);       
        statusDisp = shm.data->entrance[0].informationSign.display;
        pthread_mutex_unlock(&shm.data->entrance[0].informationSign.ISmutex);

        // NEED STATEMENT HERE TO CHECK WHAT THE DISPLAY CHARACTER IS AND WHETHER TO SEND INTO CARPARK OR TO YEET
        if (statusDisp != 'F' && statusDisp != 'X'){
            // Boomgate open/closed? Timings 
            pthread_mutex_lock(&shm.data->entrance[0].gate.gatemutex);
            char display = shm.data->entrance[0].gate.status;
            pthread_mutex_unlock(&shm.data->entrance[0].gate.gatemutex);

            if (!(display == 'O'))
            {
                pthread_mutex_lock(&shm.data->entrance[0].gate.gatemutex);
                shm.data->entrance[0].gate.status = 'R';
                pthread_mutex_unlock(&shm.data->entrance[0].gate.gatemutex);

                pthread_cond_signal(&shm.data->entrance[0].gate.gatecond);
                pthread_mutex_lock(&shm.data->entrance[0].gate.gatemutex);
                while(shm.data->entrance[0].gate.status != 'O') {
                    pthread_cond_wait(&shm.data->entrance[0].gate.gatecond, &shm.data->entrance[0].gate.gatemutex);
                }
                pthread_mutex_unlock(&shm.data->entrance[0].gate.gatemutex);

            }
            // Add plate to LPR queues
            addPlate(&levelQueue, shm.data->entrance[0].LPRSensor.plate);

            // Add car to storage 
            end = clock();
            parkTime = generateRandom(100,10000)/1000;
            time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
            printf("Time spent in carpark is %f while exit time will be %f and time_spent is %f\n", parkTime, time_spent + parkTime, time_spent);
            addCar(&carStorage, shm.data->entrance[0].LPRSensor.plate, parkTime + time_spent, 0);
        }
        else{
            printf("REJECTED!\n");
        }
    }
}

void *levelLPR(void *args){
    for (;;){ 
        // Empty LPR
        pthread_mutex_lock(&shm.data->level[0].LPRSensor.LPRmutex);
        strcpy(shm.data->level[0].LPRSensor.plate, "000000");
        pthread_mutex_unlock(&shm.data->level[0].LPRSensor.LPRmutex);

        // When level LPR is free, signal the simulator thread asking for a plate
        pthread_cond_signal(&shm.data->level[0].LPRSensor.LPRcond);

        usleep(1000);

        pthread_mutex_lock(&shm.data->level[0].LPRSensor.LPRmutex);
        while (!strcmp(shm.data->level[0].LPRSensor.plate, "000000")){
            pthread_cond_wait(&shm.data->level[0].LPRSensor.LPRcond, &shm.data->level[0].LPRSensor.LPRmutex);
        }
        pthread_mutex_unlock(&shm.data->level[0].LPRSensor.LPRmutex);
 
        // Plate recieved 
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
        // Wait for manager LPR thread to signal that LPR is free
        pthread_mutex_lock(&shm.data->level[0].LPRSensor.LPRmutex);
        while (strcmp(shm.data->level[0].LPRSensor.plate, "000000")){ 
            pthread_cond_wait(&shm.data->level[0].LPRSensor.LPRcond, &shm.data->level[0].LPRSensor.LPRmutex);
        }
        pthread_mutex_unlock(&shm.data->level[0].LPRSensor.LPRmutex);
        // LRP is now clear

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
    for (;;){ 
        // Empty LPR
        pthread_mutex_lock(&shm.data->exit[0].LPRSensor.LPRmutex);
        strcpy(shm.data->exit[0].LPRSensor.plate, "000000");
        pthread_mutex_unlock(&shm.data->exit[0].LPRSensor.LPRmutex);

        // When exit LPR is free, signal the simulator thread asking for a plate
        pthread_cond_signal(&shm.data->exit[0].LPRSensor.LPRcond);

        usleep(1000);

        pthread_mutex_lock(&shm.data->exit[0].LPRSensor.LPRmutex);
        while (!strcmp(shm.data->exit[0].LPRSensor.plate, "000000")){
            pthread_cond_wait(&shm.data->exit[0].LPRSensor.LPRcond, &shm.data->exit[0].LPRSensor.LPRmutex);
        }
        pthread_mutex_unlock(&shm.data->exit[0].LPRSensor.LPRmutex);
        // Data recieved
        printf("XLPR - After recieiving data. Plate is: %s\n", shm.data->exit[0].LPRSensor.plate);

        // Boomgate 
        pthread_mutex_lock(&shm.data->exit[0].gate.gatemutex);
        char display = shm.data->exit[0].gate.status;
        pthread_mutex_unlock(&shm.data->exit[0].gate.gatemutex);

        if (!(display == 'O'))
        {
            pthread_mutex_lock(&shm.data->exit[0].gate.gatemutex);
            shm.data->exit[0].gate.status = 'R';
            pthread_mutex_unlock(&shm.data->exit[0].gate.gatemutex);

            pthread_cond_signal(&shm.data->exit[0].gate.gatecond);
            pthread_mutex_lock(&shm.data->exit[0].gate.gatemutex);
            while(shm.data->exit[0].gate.status != 'O') {
                pthread_cond_wait(&shm.data->exit[0].gate.gatecond, &shm.data->exit[0].gate.gatemutex);
            }
            pthread_mutex_unlock(&shm.data->exit[0].gate.gatemutex);
        }
        // Bill the car
        generateBill(exitQueue.plateQueue[findIndex(&carStorage, shm.data->exit[0].LPRSensor.plate)]); // HERE??????
        // Remove from system 
        removeCar(&carStorage, shm.data->exit[0].LPRSensor.plate);
        // Remove from queue
        popPlate(&exitQueue);
    }
}


void *exitController(void *args){
    // Wait for manager LPR thread to signal that LPR is free
    pthread_mutex_lock(&shm.data->exit[0].LPRSensor.LPRmutex);
    while (strcmp(shm.data->exit[0].LPRSensor.plate, "000000")){ 
        pthread_cond_wait(&shm.data->exit[0].LPRSensor.LPRcond, &shm.data->exit[0].LPRSensor.LPRmutex);
    }
    pthread_mutex_unlock(&shm.data->exit[0].LPRSensor.LPRmutex);
    // LRP is clear 

    // Make sure plate is in queue
    while(exitQueue.size <= 0);

    // Copy a plate into memory
    pthread_mutex_lock(&shm.data->exit[0].LPRSensor.LPRmutex);
    strcpy(shm.data->exit[0].LPRSensor.plate, exitQueue.plateQueue[0]);
    pthread_mutex_unlock(&shm.data->exit[0].LPRSensor.LPRmutex);

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

void *entranceBoomgate(void *args) {
    pthread_mutex_lock(&shm.data->entrance[0].gate.gatemutex);
    while(shm.data->entrance[0].gate.status == 'C') {
        pthread_cond_wait(&shm.data->entrance[0].gate.gatecond, &shm.data->entrance[0].gate.gatemutex);
    }
    pthread_mutex_unlock(&shm.data->entrance[0].gate.gatemutex);

    // Set gate to raising
    pthread_mutex_lock(&shm.data->entrance[0].gate.gatemutex);
    shm.data->entrance[0].gate.status = 'R';
    pthread_mutex_unlock(&shm.data->entrance[0].gate.gatemutex);
    printf("NG - Gate status set to %s\n", &shm.data->entrance[0].gate.status);
   
    // Wait 10 ms
    usleep(10000);
    // Set gate to open
    pthread_mutex_lock(&shm.data->entrance[0].gate.gatemutex);
    shm.data->entrance[0].gate.status = 'O';
    pthread_mutex_unlock(&shm.data->entrance[0].gate.gatemutex);
    printf("NG - Gate status set to %s\n", &shm.data->entrance[0].gate.status);
    
    // Signal gate is open
    pthread_cond_signal(&shm.data->entrance[0].gate.gatecond);

    // Wait for car to automatically close gate
    usleep(20000);
    
    // Set gate to lowering
    pthread_mutex_lock(&shm.data->entrance[0].gate.gatemutex);
    shm.data->entrance[0].gate.status = 'L';
    pthread_mutex_unlock(&shm.data->entrance[0].gate.gatemutex);
    printf("NG - Gate status set to %s\n", &shm.data->entrance[0].gate.status);

    // Set gate to closed
    pthread_mutex_lock(&shm.data->entrance[0].gate.gatemutex);
    shm.data->entrance[0].gate.status = 'C';
    pthread_mutex_lock(&shm.data->entrance[0].gate.gatemutex);
    printf("NG - Gate status set to %s\n", &shm.data->entrance[0].gate.status);
}

void *exitBoomgate(void *args) {
    // Waiting for status of boom gate to change 
    pthread_mutex_lock(&shm.data->exit[0].gate.gatemutex);
    while(shm.data->exit[0].gate.status == 'C') {
        pthread_cond_wait(&shm.data->exit[0].gate.gatecond, &shm.data->exit[0].gate.gatemutex);
    }
    pthread_mutex_unlock(&shm.data->exit[0].gate.gatemutex);

    // Set gate to raising
    pthread_mutex_lock(&shm.data->exit[0].gate.gatemutex);
    shm.data->exit[0].gate.status = 'R';
    pthread_mutex_unlock(&shm.data->exit[0].gate.gatemutex);
    printf("XG - Gate status set to %s\n", &shm.data->exit[0].gate.status);
   
    // Wait 10 ms
    usleep(10000);
    // Set gate to open
    pthread_mutex_lock(&shm.data->exit[0].gate.gatemutex);
    shm.data->exit[0].gate.status = 'O';
    pthread_mutex_unlock(&shm.data->exit[0].gate.gatemutex);
    printf("XG - Gate status set to %s\n", &shm.data->exit[0].gate.status);
    
    // Signal gate is open
    pthread_cond_signal(&shm.data->exit[0].gate.gatecond);

    // Wait for car to automatically close gate
    usleep(20000);
    
    // Set gate to lowering
    pthread_mutex_lock(&shm.data->exit[0].gate.gatemutex);
    shm.data->exit[0].gate.status = 'L';
    pthread_mutex_unlock(&shm.data->exit[0].gate.gatemutex);
    printf("XG - Gate status set to %s\n", &shm.data->exit[0].gate.status);

    // Set gate to closed
    pthread_mutex_lock(&shm.data->exit[0].gate.gatemutex);
    shm.data->exit[0].gate.status = 'C';
    pthread_mutex_lock(&shm.data->exit[0].gate.gatemutex);
    printf("XG - Gate status set to %s\n", &shm.data->exit[0].gate.status);
}

// // Determines a level for the car to go to
int determineLevel(void) {
    int level = 0;
    
    for (int i = 1; i <= LEVELS; i++) {
        if (levelQueue.size < CAPACITY) {
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

void *informationSign(void *args) {
    for (;;){
        bool allowed = false;
        char toDisplay;
        // Wait for information sign to be signalled
        pthread_mutex_lock(&shm.data->entrance[0].informationSign.ISmutex);
        while (shm.data->entrance[0].informationSign.display != 'O') {
            pthread_cond_wait(&shm.data->entrance[0].informationSign.IScond, &shm.data->entrance[0].informationSign.ISmutex);
        }
        pthread_mutex_unlock(&shm.data->entrance[0].informationSign.ISmutex);

        for (int i = 0; i < 100; i++) {
            if (strcmp(shm.data->entrance[0].LPRSensor.plate, allowedPlates[i])) { // if plate in LPR or entrance queue?? is not in allowed plates then display 'X' and yeet the plate
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
            toDisplay = (char)generateRandom(0, 1) + '0';
            //printf("infoSign - Randomly Generated number = %c\n", toDisplay);
            if (toDisplay == '0') {
                toDisplay = 'F';
            }
        }

        // Set information sign display to 'toDisplay' to show its ready
        pthread_mutex_lock(&shm.data->entrance[0].informationSign.ISmutex);
        shm.data->entrance[0].informationSign.display = toDisplay;
        pthread_mutex_unlock(&shm.data->entrance[0].informationSign.ISmutex);

        //Signal entrance LPR to say that we have changed the display
        pthread_cond_signal(&shm.data->entrance[0].informationSign.IScond);
    }

}

void generateBill(char* plate) {
    FILE *billingFile;

    char* numPlate = exitQueue.plateQueue[findIndex(&carStorage, plate)];
    clock_t endTime = clock();
    double bill = (double)(endTime - begin) / CLOCKS_PER_SEC; // FOR NOW
    //double bill = (carStorage.car[findIndex(&carStorage, plate)].exitTime - entryTime) * 0.05; // Need to add delay timings and entryTime or time_spent to car struct?
    printf("B - numplate = %s\n", numPlate);
    printf("B - bill = $%.2f\n", bill);


    if (billingFile = fopen("billing.txt", "r")) 
    {
        printf("B - file exists\n");
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
    pthread_t entranceLPR_thread;
    pthread_t levelLPR_thread;
    pthread_t levelController_thread;
    pthread_t exitLPR_thread;
    pthread_t exitController_thread;
    pthread_t checkTimes_thread;
    pthread_t exitBoomgate_thread;
    pthread_t entranceBoomgate_thread;
    pthread_t informationSign_thread;

    plateInit(&levelQueue); 
    plateInit(&exitQueue);     
    storageInit(&carStorage);
 
    pthread_create(&entranceLPR_thread, NULL, entranceLPR, NULL);
    pthread_create(&levelLPR_thread, NULL, levelLPR, NULL);
    pthread_create(&levelController_thread, NULL, levelController, NULL);
    pthread_create(&exitLPR_thread, NULL, exitLPR, NULL);
    pthread_create(&exitController_thread, NULL, exitController, NULL);
    pthread_create(&checkTimes_thread, NULL, checkTimes, NULL);
    pthread_create(&exitBoomgate_thread, NULL, exitBoomgate, NULL);
    pthread_create(&entranceBoomgate_thread, NULL, entranceBoomgate, NULL);
    pthread_create(&informationSign_thread, NULL, informationSign, NULL);

    pthread_join(entranceLPR_thread, NULL);
    pthread_join(levelLPR_thread, NULL);
    pthread_join(levelController_thread, NULL);
    pthread_join(exitLPR_thread, NULL);
    pthread_join(exitController_thread, NULL);
    pthread_join(checkTimes_thread, NULL);
    pthread_join(exitBoomgate_thread, NULL);
    pthread_join(entranceBoomgate_thread, NULL);
    pthread_join(informationSign_thread, NULL);
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