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

#include "shm_ops.c"

#define SHARE_NAME "PARKING"

// ------------------------------------ FILE OVERVIEW --------------------------------------------- // 
/**
 *  Tasks of the Manager
 * 
 * 1. Monitor the status of the LPR (license plate recognition) sensor 
 * 1.1 Keep track where each car is <- linkedlist or array of cars
 * 
 * 2. Tell the boom gates to open and close +(automatic close after fixed time)
 * 
 * 3. Room allocation for cars
 * 3.1 Continous check for (number of cars < number of levels * number of cars per level)
 * 
 * 4. Keep track of how long car has been in park
 * 4.1 Calculate bill at the end
 * 
 * 5. Display current status (capacity of each level, current status of boom gate, signs, temperature sensor and alarms, revenue for carpark)
 */

/** Method of achieving tasks
 * 
 * 
 */

// ------------------------------------ STRUCTURE DECLERATIONS ------------------------------------ // 

// ------------------------------------ FUNCTION DECLERATIONS ------------------------------------- // 
void StatusLPR();


// --------------------------------------- PUBLIC VARIABLES --------------------------------------- // 
int LevelCapacity;
char BoomGateStatus;
char SignStatus;
char TempSensorStatus;
char AlarmStatus;
int CarparkRevenue;



// --------------------------------------------- MAIN --------------------------------------------- // 
int main()
{    
    shared_memory_t shm;
    create_shared_object_R(&shm, SHARE_NAME);

    //printf("Data stored entrace gate: %c \n", shm.data->entrance[0].gate.status);

    


}

// --------------------------------------- HELPER FUNCTUONS --------------------------------------- // 

/**
 * This function acts as the GUI for the manager file system
 * the function displays the status of the capacity, Boomgate, signs, Temperature sensor
 * Alarm, Car park revenue. */
void StatusLPR(){
    printf("\n----------------------------------------------------------------");
    printf("\nCapacity:");
    printf("\nBoomgate:");
    printf("\nSigns");
    printf("\nTemperature");
    printf("\n----------------------------------------------------------------");
    printf("\n");
    return; //
}

