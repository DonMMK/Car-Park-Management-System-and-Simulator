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
 *  Tasks of the Manager
 * 
 * 1. Monitor the status of the LPR sensor 
 * 1.1 Keep track where each car is
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