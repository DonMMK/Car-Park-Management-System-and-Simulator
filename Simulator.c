#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "sharedMemory.c"
/**
 * A car object
 */
typedef struct car {
    int entrance; 
    const char* plate; 
} car_t;

int shm_fd;
volatile void *shm;
int allowedPlatesNum;

int generateRandom(int lower, int upper);

int main()
{
    shm_fd = shm_open("PARKING", O_RDWR, 0);
	shm = (volatile void *) mmap(0, 2920, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    car_t car;

    FILE* file = fopen("plates.txt", "r");
    char line[10];

    // Generates array size
    while (fgets(line, sizeof(line), file)) {
        allowedPlatesNum++;
    }
    
    char *allowedPlates[allowedPlatesNum];
    int counter = 0;
    while (fgets(line, sizeof(line), file)) {
        allowedPlates[counter] = line;
    }

    for (int i = 0; i < allowedPlatesNum; i++){
        printf("%s",allowedPlates[i]);
    }

    for (int i = 1;i < 1;i++){
        int time = generateRandom(0,100) * 1000;
        usleep(time);

        // printf("Jack is weird at time: %d\n",time);

        car.entrance = generateRandom(1,ENTRANCES);
    }
}


// Generates random numbers in range [lower, upper]. 
// https://www.geeksforgeeks.org/generating-random-number-range-c/?fbclid=IwAR1a4I7mqxidG7EHit34MRmTLgge9xMfBQtw8TcCXVlYC9_QqrATtfESm94
int generateRandom(int lower, int upper)
{
    int i;
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
}