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
#include "sharedMemory.c"

#define SHMSZ 2920


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



// --------------------------------------- PUBLIC VARIABLES --------------------------------------- // 
char allowedPlates[100][10];



// --------------------------------------------- MAIN --------------------------------------------- // 
int main()
{
    // Create variables 
    int waitTime;
    int shm_fd;
    const char *key = "PARKING";
    car_t car;
    shared_memory_t shm;

    // Initialise random seed
    time_t t;
    srand((unsigned) time(&t));

    // Create the segment.
    if ((shm.fd = shm_open(key, O_CREAT | O_RDWR, 0666)) < 0) {
        perror("shm_open");
        exit(1);
    }
    
    // Configure the size of the shared memory segment
    ftruncate(shm_fd, SHMSZ);

    // Attach the segment to our data space
    if ((shm.data  = mmap(0, SHMSZ, PROT_WRITE, MAP_SHARED, shm_fd, 0)) == (char *)-1)
    {
        perror("mmap");
        exit(1);
    }

    // Read the number plates 
    readFile("plates.txt");
    // printFile();

    // BEGINING SIMULATION
    printf("\nStarting simulation...\n");

    for (int i = 1;i < 2;i++){
        // Generate car every 1 - 100 milliseconds
        waitTime = generateRandom(1,100) * 1000;
        usleep(waitTime);

        // Spawn car at random entrance
        car.entrance = generateRandom(1,ENTRANCES);
        printf("Car arriving at entrance: %d\n", car.entrance);

        // Generate numberplate (from list/random)
        car.plate = generatePlate(80);
        printf("Car has plate number: %s\n", car.plate);    

        // wait 2ms to trigger LPR
        usleep(2000);

        // TRIGGER LRP AT ENTRANCE
        // thread_cond_signal(shm.data.entrance[1].LPRSensor.LRPcond);

        // Read digital sign
        int carLevel = generateRandom(1,LEVELS);
        printf("Car will be heading to level: %d\n", carLevel);    

        // Wait for boom gate to open (10ms)
        usleep(10000);

        // SIGNAL BOOMGATE AS RAISING/OPEN/CLOSING/CLOSED

        // drive to park (10ms)
        usleep(10000);

        //SET OFF LPR ON FLOOR?

        // park for random time (100-10000ms)
        usleep(10000);
        waitTime = generateRandom(100,10000) * 1000;
        printf("Parked for %d seconds...\n", waitTime/1000000);
        usleep(time);
    
        //SET OFF LPR ON FLOOR?

        // drive to exit (10ms)
        usleep(10000);
        car.exit = generateRandom(1,ENTRANCES);
        printf("Car going to exit: %d\n\n", car.exit);

        // TRIGGER LRP AT EXIT

        // Wait for boom gate to open (10ms)
        usleep(10000);

        // SIGNAL BOOMGATE AS RAISING/OPEN/CLOSING/CLOSED   
    }
}



// --------------------------------------- HELPER FUNCTUONS --------------------------------------- // 
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
