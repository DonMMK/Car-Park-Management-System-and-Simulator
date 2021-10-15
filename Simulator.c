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



// ------------------------------------ STRUCTURE DECLERATIONS ------------------------------------ // 
typedef struct car {
    int entrance; 
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
    // Initialise random
    time_t t;
    srand((unsigned) time(&t));

    // Setup car object and shared memory
    car_t car;
    int shm_fd = shm_open("PARKING", O_RDWR, 0);
	volatile void *shm = (volatile void *) mmap(0, 2920, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    // Read the number plates 
    readFile("plates.txt");
    // printFile();

    // car.plate = generatePlate(80);
    // printf("%s\n", car.plate);

    for (int i = 1;i < 2;i++){
        // Generate car every 1 - 100 milliseconds
        int time = generateRandom(1,100) * 1000;
        usleep(time);

        // Spawn car at random entrance 
        car.entrance = generateRandom(1,ENTRANCES);
        printf("Car arriving at entrance: %d\n", car.entrance);

        // Generate numberplate (from list/random)
        car.plate = generatePlate(80);
        printf("Car has plate number: %s\n", car.plate);
        
        
    }
}



// --------------------------------------- HELPER FUNCTUONS --------------------------------------- // 
// Generates random numbers in range [lower, upper]. 
// https://www.geeksforgeeks.org/generating-random-number-range-c/?fbclid=IwAR1a4I7mqxidG7EHit34MRmTLgge9xMfBQtw8TcCXVlYC9_QqrATtfESm94
int generateRandom(int lower, int upper)
{    
    int i;
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

// Prints contents of previous file
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
