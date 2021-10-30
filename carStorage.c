#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h> 

#define LEVELS 5
#define CAPACITY 20
#define MAX_CARPARK 100
#define STORAGE_CAPACITY 7

typedef struct car {
	/// The content of the vector.
	char plate[STORAGE_CAPACITY];

    // Entrance time 
    clock_t entranceTime;

    // Park time 
    clock_t parkTime;

    // Level
    int level;

    // Exit status 
    bool exitStatus;

    // Number of time passed LPR
    int LPRcount;
} car_t;

typedef struct carStorage {
	/// The current number of elements in the vector
	int size;

	/// The content of the vector.
	car_t car[MAX_CARPARK];
} carStorage_t;

void storageInit(carStorage_t *carStorage);
void addCar(carStorage_t *carStorage, char *plate, clock_t entranceTime, clock_t parkTime, int level);
void removeCar(carStorage_t* carQueue, char *plate);
void printCarList(carStorage_t* carStorage);
int findIndex(carStorage_t* carStorage, char *plate);

// Initialise the storage
void storageInit(carStorage_t *carStorage){
    carStorage->size = 0;
    for (int i = 0; i < MAX_CARPARK; i++){
        strcpy(carStorage->car[i].plate, "empty");
        carStorage->car[i].entranceTime = 0;
        carStorage->car[i].parkTime = 0;
        carStorage->car[i].exitStatus = false;
        carStorage->car[i].level = 1;
        carStorage->car[i].LPRcount = 0;
    }
}

// Add a car to the storage
void addCar(carStorage_t *carStorage, char *plate, clock_t entranceTime, clock_t parkTime, int level){
    int old_size = carStorage->size;
    memcpy(carStorage->car[old_size].plate, plate, 7);
    carStorage->car[old_size].entranceTime = entranceTime;
    carStorage->car[old_size].parkTime = parkTime;
    carStorage->car[old_size].level = level;
    carStorage->car[old_size].exitStatus = false;
    carStorage->car[old_size].LPRcount = 0;
    carStorage->size = old_size + 1;
}

// Remove a car based on its plate
void removeCar(carStorage_t* carStorage, char *plate){
    int old_size = carStorage->size;
    car_t old_car[MAX_CARPARK];
    int loc;

    for (int i = 0; i < old_size; i++){
        memcpy(old_car[i].plate, carStorage->car[i].plate, 7);
        old_car[i].entranceTime = carStorage->car[i].entranceTime;
        old_car[i].parkTime = carStorage->car[i].parkTime;
        old_car[i].level = carStorage->car[i].level;
        old_car[i].exitStatus = carStorage->car[i].exitStatus;
        old_car[i].LPRcount = carStorage->car[i].LPRcount;
    }

    for (int i = 0; i < old_size; i++){
        if (memcmp(carStorage->car[i].plate, plate, 7) == 0){
            loc = i;
            break;
        }
    }

    for (int i = 0; i < loc; i++){
        memcpy(carStorage->car[i].plate, old_car[i].plate,7);
        carStorage->car[i].entranceTime = old_car[i].entranceTime;
        carStorage->car[i].parkTime = old_car[i].parkTime;
        carStorage->car[i].level = old_car[i].level;
        carStorage->car[i].exitStatus = old_car[i].exitStatus;
        carStorage->car[i].LPRcount = old_car[i].LPRcount;
    }

    for (int i = loc; i < old_size - 1; i++){
        memcpy(carStorage->car[i].plate, old_car[i + 1].plate,7);
        carStorage->car[i].entranceTime = old_car[i + 1].entranceTime;
        carStorage->car[i].parkTime = old_car[i + 1].parkTime;
        carStorage->car[i].level = old_car[i + 1].level;
        carStorage->car[i].exitStatus = old_car[i + 1].exitStatus;
        carStorage->car[i].LPRcount = old_car[i + 1].LPRcount;
    }

    carStorage->size = old_size - 1;
}

// Find index of plate (from numberplate)
int findIndex(carStorage_t* carStorage, char *plate){
    int index;
    for (int i = 0; i < carStorage->size; i++){
        if (memcmp(carStorage->car[i].plate, plate,7) == 0){
            index = i;
            return index;
        }
    }
    return 1;
}

// Print cars 
void printCarList(carStorage_t* carStorage){
    printf("NUMBER OF PLATES ARE: %d\n", carStorage->size); 
    for (int i = 0; i < carStorage->size; i++){
        printf("Plate number %d is: %s\n", i, carStorage->car[i].plate);
        printf("Entrance time of car %d is: %ld\n", i, carStorage->car[i].entranceTime);
        printf("Park time of car %d is: %ld\n", i, carStorage->car[i].parkTime);
        printf("Level of car %d is: %d\n", i, carStorage->car[i].level);
        printf("Number of LRP count %d is: %d\n", i, carStorage->car[i].LPRcount);
    }
}

// int main(void){
//     carStorage_t carStorage;
//     storageInit(&carStorage);
//     printf("Adding 4 plates...\n");
//     addCar(&carStorage, "jactoe", 1000, 10, 1);
//     addCar(&carStorage, "jactoa", 950, 10, 2);
//     addCar(&carStorage, "jactor", 923, 10, 3);
//     addCar(&carStorage, "penisl", 5, 10, 4);    
//     // printCarList(&carStorage);
//     // printf("\n");

//     // printf("Removing a plate 'jactor'...\n");
//     // removeCar(&carStorage, "jactor");
//     // printCarList(&carStorage);
//     // printf("\n");
//     int index = findIndex(&carStorage, "jactor");
//     printf("Index of plate before removal is: %d\n", index);


//     printf("Removing a plate 'jacktoa'...\n");
//     removeCar(&carStorage, "jactoa");
//     printCarList(&carStorage);
//     printf("\n");
//     index = findIndex(&carStorage, "jactor");
//     printf("Index of plate before removal is: %d\n", index);
// }