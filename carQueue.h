#include <stdio.h>
#include <string.h>

#define MAX_CAPACITY 1000
#define STORAGE_CAPACITY 10

typedef struct carQueue {
	// The current number of elements in the vector
	int size;

	// The content of the vector.
	char plateQueue[MAX_CAPACITY][STORAGE_CAPACITY];
} carQueue_t;

void plateInit(carQueue_t *carQueue);
void addPlate(carQueue_t *carQueue, char *plate);
void popPlate(carQueue_t* carQueue); 
void popRandom(carQueue_t* carQueue, int index); 
void printPlate(carQueue_t* carQueue);

void plateInit(carQueue_t *carQueue){
    carQueue->size = 0;
    for (int i = 0; i < MAX_CAPACITY; i++){
        strcpy(carQueue->plateQueue[i], "empty");
    }
}

void addPlate(carQueue_t* carQueue, char * plate){
    int old_size = carQueue->size;
    strcpy(carQueue->plateQueue[old_size], plate);
    carQueue->size = old_size + 1;
}

void popPlate(carQueue_t* carQueue){
    int old_size = carQueue->size;
    char old_data[MAX_CAPACITY][STORAGE_CAPACITY];
    for (int i = 0; i < old_size; i++){
        strcpy(old_data[i], carQueue->plateQueue[i]);
    }
    for (int i = 0; i < old_size - 1; i++){
        strcpy(carQueue->plateQueue[i], old_data[i + 1]);
    }
    carQueue->size = old_size - 1;
}

void popRandom(carQueue_t* carQueue, int index){
    int old_size = carQueue->size;
    char old_data[MAX_CAPACITY][STORAGE_CAPACITY];
    for (int i = 0; i < old_size; i++){
        strcpy(old_data[i], carQueue->plateQueue[i]);
    }
    for (int i = 0; i < index; i++){
        strcpy(carQueue->plateQueue[i], old_data[i]);
    }

    for (int i = index; i < old_size - 1; i++){
        strcpy(carQueue->plateQueue[i], old_data[i + 1]);
    }
    carQueue->size = old_size - 1;
}

void printPlate(carQueue_t* carQueue){
    printf("NUMBER OF PLATES ARE: %d\n", carQueue->size); 
    for (int i = 0; i < carQueue->size; i++){
        printf("Plate number %d is: %s\n", i, carQueue->plateQueue[i]);
    }
}

// int main(void){
//     carQueue_t carQueue;
//     plateInit(&carQueue);
//     printf("Adding 3 plates...\n");
//     addPlate(&carQueue, "jactoe");
//     addPlate(&carQueue, "jactoa");
//     addPlate(&carQueue, "jactor");
//     addPlate(&carQueue, "penisl");    
//     printPlate(&carQueue);
//     printf("\n");

//     printf("Popping a plate...\n");
//     popPlate(&carQueue);
//     printPlate(&carQueue);
//     printf("\n");

//     printf("Popping a plate...\n");
//     popPlate(&carQueue);
//     printPlate(&carQueue);
//     printf("\n");

//     printf("Popping a plate...\n");
//     popPlate(&carQueue);
//     printPlate(&carQueue);
//     printf("\n");
// }

