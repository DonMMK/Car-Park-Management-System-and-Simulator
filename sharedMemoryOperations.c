#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#include "sharedMemory.c"

/**
 * Controller: initialise a shared_object_t, creating a block of shared memory
 * with the designated name, and setting its storage capacity to the size of a
 * shared data block.
 */
bool create_shared_object_RW( shared_memory_t* shm, const char* share_name ) {
    // Remove any previous instance of the shared memory object, if it exists.
    shm_unlink(share_name);

    // Assign share name to shm->name.
    shm->name = share_name;

    // Create the shared memory object, allowing read-write access
    if ((shm->fd = shm_open(share_name, O_CREAT | O_RDWR, 0666)) < 0){
        shm->data = NULL;
        return false;
    }

    // Set the capacity of the shared memory object via ftruncate.
    if (ftruncate(shm->fd,sizeof(shared_data_t)) == -1){
        shm->data = NULL;
        return false;
    }

    // Map memory segment
    if ((shm->data = mmap(0, sizeof(shared_data_t), PROT_WRITE, MAP_SHARED, shm->fd, 0)) == (void *)-1)
    {
        return false;
    }

    // If we reach this point we should return true.
    return true;
}

/**
 * Controller: destroys the shared memory object managed by a shared memory
 * control block.
 */
void destroy_shared_object( shared_memory_t* shm ) {
    // Remove the shared memory object.
    munmap(shm, 48);
    shm->data = NULL;
    shm->fd = -1;
    shm_unlink(shm->name);
}


/**
 * Controller: initialise a shared_object_t, creating a block of shared memory
 * with the designated name, and setting its storage capacity to the size of a
 * shared data block.
 */
bool create_shared_object_R( shared_memory_t* shm, const char* share_name ) {
    // Assign share name to shm->name.
    shm->name = share_name;

    // Create the shared memory object, allowing read-write access
    if ((shm->fd = shm_open(share_name, O_RDWR, 0)) < 0){
        shm->data = NULL;
        return false;
    }

    // Map memory segment
    if ((shm->data = mmap(0, sizeof(shared_data_t), PROT_WRITE, MAP_SHARED, shm->fd, 0)) == (void *)-1)
    {
        return false;
    }
    return true;
}

/**
 * Init: Establishes values for shared memory segment
 */
void initialiseSharedMemory(shared_memory_t shm){
    pthread_mutexattr_t attr_m;
    pthread_condattr_t attr_c;
    pthread_mutexattr_init(&attr_m);
    pthread_condattr_init(&attr_c);
    pthread_mutexattr_setpshared(&attr_m, PTHREAD_PROCESS_SHARED);
    pthread_condattr_setpshared(&attr_c, PTHREAD_PROCESS_SHARED);

    for (int i = 0;i < ENTRANCES;i++){
        // Intilise Mutexs and Condition variables for LPR sensors
        pthread_mutex_init(&shm.data->entrance[i].LPRSensor.LPRmutex, &attr_m);
        pthread_cond_init(&shm.data->entrance[i].LPRSensor.LPRcond, &attr_c);
        // Intilise Mutexs and Condition variables for Gates
        pthread_mutex_init(&shm.data->entrance[i].gate.gatemutex, &attr_m);
        pthread_cond_init(&shm.data->entrance[i].gate.gatecond, &attr_c);
        // Intilise Mutexs and Condition variables for Information signs
        pthread_mutex_init(&shm.data->entrance[i].informationSign.ISmutex, &attr_m);
        pthread_cond_init(&shm.data->entrance[i].informationSign.IScond, &attr_c);

        // Initiliase status of the gates to 'closed'
        shm.data->entrance[i].gate.status = 'C';
        strcpy(shm.data->entrance[i].LPRSensor.plate, "xxxxxx");
    }

    for (int i = 0;i < EXITS;i++){
        // Intilise Mutexs and Condition variables for LPR sensors
        pthread_mutex_init(&shm.data->exit[i].LPRSensor.LPRmutex, &attr_m);
        pthread_cond_init(&shm.data->exit[i].LPRSensor.LPRcond, &attr_c);
        // Intilise Mutexs and Condition variables for Gates
        pthread_mutex_init(&shm.data->exit[i].gate.gatemutex, &attr_m);
        pthread_cond_init(&shm.data->exit[i].gate.gatecond, &attr_c);

        // Initiliase status of the gates to 'closed'
        shm.data->exit[i].gate.status = 'C';
        strcpy(shm.data->exit[i].LPRSensor.plate, "xxxxxx");
    }

    for (int i = 0;i < LEVELS;i++){
        // Intilise Mutexs and Condition variables for LPR sensors
        pthread_mutex_init(&shm.data->level[i].LPRSensor.LPRmutex, &attr_m);
        pthread_cond_init(&shm.data->level[i].LPRSensor.LPRcond, &attr_c);
        shm.data->level[i].fireAlarm = '0';
        shm.data->level[i].tempSensor = 24;

        // Initiliase number plate to be xxxxxx
        strcpy(shm.data->level[i].LPRSensor.plate, "xxxxxx");
    }

}