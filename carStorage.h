#include <stdio.h>
#include <string.h>
 
#define LEVELS 5
#define CAPACITY 20
#define MAX_CARPARK LEVELS * CAPACITY
#define STORAGE_CAPACITY 7

typedef struct car {
	/// The content of the vector.
	char plate[STORAGE_CAPACITY];

    // Exit time 
    double exitTime;

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
void addCar(carStorage_t *carStorage, char *plate, long time, int LPRcount);
void removeCar(carStorage_t* carQueue, char *plate);
void printCarList(carStorage_t* carStorage);
int findIndex(carStorage_t* carStorage, char *plate);

void storageInit(carStorage_t *carStorage){
    carStorage->size = 0;
    for (int i = 0; i < MAX_CARPARK; i++){
        strcpy(carStorage->car[i].plate, "empty");
        carStorage->car[i].exitTime = 0;
        carStorage->car[i].exitTime = false;
        carStorage->car[i].LPRcount = 0;
    }
}

void addCar(carStorage_t *carStorage, char *plate, long time, int LPRcount){
    int old_size = carStorage->size;
    strcpy(carStorage->car[old_size].plate, plate);
    carStorage->car[old_size].exitTime = time;
    carStorage->car[old_size].LPRcount = LPRcount;
    carStorage->size = old_size + 1;
}

void removeCar(carStorage_t* carStorage, char *plate){
    int old_size = carStorage->size;
    car_t old_car[MAX_CARPARK];
    int loc;

    for (int i = 0; i < old_size; i++){
        strcpy(old_car[i].plate, carStorage->car[i].plate);
        old_car[i].exitTime = carStorage->car[i].exitTime;
        old_car[i].exitStatus = carStorage->car[i].exitStatus;
        old_car[i].LPRcount = carStorage->car[i].LPRcount;
    }

    for (int i = 0; i < old_size; i++){
        if (strcmp(carStorage->car[i].plate, plate) == 0){
            loc = i;
            break;
        }
    }

    for (int i = loc; i < old_size - 1; i++){
        strcpy(carStorage->car[i].plate, old_car[i + 1].plate);
        carStorage->car[i].exitTime = old_car[i + 1].exitTime;
        carStorage->car[i].exitStatus = old_car[i + 1].exitStatus;
        carStorage->car[i].LPRcount = old_car[i + 1].LPRcount;
    }

    carStorage->size = old_size - 1;
}

int findIndex(carStorage_t* carStorage, char *plate){
    int old_size = carStorage->size;
    car_t old_car[MAX_CARPARK];
    int loc;

    for (int i = 0; i < old_size; i++){
        strcpy(old_car[i].plate, carStorage->car[i].plate);
        old_car[i].exitTime = carStorage->car[i].exitTime;
        old_car[i].exitStatus = carStorage->car[i].exitStatus;
        old_car[i].LPRcount = carStorage->car[i].LPRcount;
    }

    for (int i = 0; i < old_size; i++){
        if (strcmp(carStorage->car[i].plate, plate) == 0){
            loc = i;
            break;
        }
    }

    return loc;
}

void printCarList(carStorage_t* carStorage){
    printf("NUMBER OF PLATES ARE: %d\n", carStorage->size); 
    for (int i = 0; i < carStorage->size; i++){
        printf("Plate number %d is: %s\n", i, carStorage->car[i].plate);
        printf("Exit time of car %d is: %f\n", i, carStorage->car[i].exitTime);
        printf("Number of LRP count %d is: %d\n", i, carStorage->car[i].LPRcount);
    }
}

// int main(void){
//     carStorage_t carStorage;
//     storageInit(&carStorage);
//     printf("Adding 4 plates...\n");
//     addCar(&carStorage, "jactoe", 1000, 0);
//     addCar(&carStorage, "jactoa", 950, 1);
//     addCar(&carStorage, "jactor", 923, 40);
//     addCar(&carStorage, "penisl", 5, 2);    
//     printCarList(&carStorage);
//     printf("\n");

//     printf("Removing a plate 'jactor'...\n");
//     removeCar(&carStorage, "jactor");
//     printCarList(&carStorage);
//     printf("\n");

//     printf("Removing a plate 'jacktoa'...\n");
//     removeCar(&carStorage, "jactoa");
//     printCarList(&carStorage);
//     printf("\n");
// }

