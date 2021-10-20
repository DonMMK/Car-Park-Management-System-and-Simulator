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

#include "shm_ops.c"

#define SHARE_NAME "PARKING"
#define CAR_LIMIT 1


// ------------------------------------ STRUCTURE DECLERATIONS ------------------------------------ // 
typedef struct car {
    int entrance; 
    int exit;
    char* plate; 
} car_t;



// ------------------------------------ FUNCTION DECLERATIONS ------------------------------------- // 
int generateRandom(int lower, int upper);
void readFile(char *filename);
void printFile();
char* generatePlate(int probability);
char* randomPlate();
void *carSimulate(void *arg);


// --------------------------------------- PUBLIC VARIABLES --------------------------------------- // 
char allowedPlates[100][10];
shared_memory_t shm;


// --------------------------------------------- MAIN --------------------------------------------- // 
int main()
{
    // Create variables 
    int waitTime;
    pthread_t carThreads[CAR_LIMIT];

    // Initialise random seed
    time_t t;
    srand((unsigned) time(&t));

    create_shared_object_RW(&shm, SHARE_NAME);

    // Read the number plates 
    readFile("plates.txt");
    // printFile();

    // BEGINING SIMULATION
    printf("...STARTING SIMULATION...\n");    
    
    // shm.data->entrance[0].gate.status = 'c';

    for (int i = 0;i < CAR_LIMIT;i++){
        pthread_create(&carThreads[i], NULL, carSimulate, NULL);
    }

    for (int i = 0;i < CAR_LIMIT;i++){
        // Generate car every 1 - 100 milliseconds
        waitTime = generateRandom(1,100) * 1000;
        usleep(waitTime);

        // SPAWN CAR THREAD 
        pthread_join(carThreads[i],NULL);
    }
}



// --------------------------------------- HELPER FUNCTUONS --------------------------------------- // 

void *carSimulate(void *arg){
    car_t car;
    int waitTime;

    // Spawn car at random entrance
    car.entrance = generateRandom(1,ENTRANCES);
    printf("Car arriving at entrance: %d\n", car.entrance);

    // Generate numberplate (from list/random)
    car.plate = generatePlate(80);
    printf("Car has plate number: %s\n", car.plate);    

    // wait 2ms to trigger LPR
    usleep(2000);

    // Read digital sign
    int carLevel = generateRandom(1,LEVELS);

    // Check if car level is a digit between 1 - LEVELS
    if (carLevel > LEVELS || carLevel <= 0 || isdigit(carLevel) != 0) {
        return 0;
    }
    printf("Car will be heading to level: %d\n", carLevel);  

    // TRIGGER LRP AT ENTRANCE
    shm.data->entrance[car.entrance].LPRSensor.plate = car.plate;
    pthread_cond_signal(&shm.data->entrance[car.entrance].LPRSensor.LRPcond);   

    // Wait for boom gate to open (10ms)
    pthread_cond_wait(&shm.data->exit[car.exit].gate.gatecond, &shm.data->exit[car.exit].gate.gatemutex);

    // drive to park (10ms)
    usleep(10000);

    //SET OFF LPR ON FLOOR
    shm.data->exit->LPRSensor.plate = car.plate;
    pthread_cond_signal(&shm.data->level[carLevel].LPRSensor.LRPcond);

    // park for random time (100-10000ms)
    usleep(10000);
    waitTime = generateRandom(100,10000) * 1000;
    printf("Parked for %d seconds...\n", waitTime/1000000);
    usleep(waitTime);

    //SET OFF LPR ON FLOOR
    pthread_cond_signal(&shm.data->level[carLevel].LPRSensor.LRPcond);

    // drive to exit (10ms)
    usleep(10000);
    car.exit = generateRandom(1,EXITS);
    printf("Car going to exit: %d\n", car.exit);

    // TRIGGER LRP AT EXIT
    pthread_cond_signal(&shm.data->exit[car.exit].LPRSensor.LRPcond);

    // Wait for boom gate to open (10ms)
    pthread_cond_wait(&shm.data->exit[car.exit].gate.gatecond, &shm.data->exit[car.exit].gate.gatemutex);

    // DELETE CAR THREAD 
    return 0;
}




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
