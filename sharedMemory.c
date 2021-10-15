#include <semaphore.h>

#define ENTRANCES 5
#define EXITS 5
#define LEVELS 5
#define CAPACITY 20

/**
 * LPR sensors
 */
typedef struct LPRSensor {
    pthread_mutex_t LPRmutex;
    pthread_cond_t LRPcond;
    const char* plate;
} LPRSensor_t;

/**
 * Boom gates 
 */
typedef struct gate {
	pthread_mutex_t gatemutex;
	pthread_cond_t gatecond;
	char status;    
} gate_t;

/**
 * Information sign 
 */
typedef struct informationSign {
    pthread_mutex_t ISmutex;
	pthread_cond_t IScond;
	char display;
} informationSign_t;

/**
 * The different entrances
 */
typedef struct entrance {
    LPRSensor_t LPRSensor;
    gate_t gate;
    informationSign_t informationSign;
} entrance_t;

/**
 * The different exits
 */
typedef struct exit {
    LPRSensor_t LPRSensor;
    gate_t gate;
} exit_t;

/**
 * The different levels
 */
typedef struct level {
    LPRSensor_t LPRSensor;
    int16_t tempSensor; 
    char fireAlarm;
} level_t;


/**
 * Our shared data block.
 */
typedef struct shared_data {
    entrance_t entrance[ENTRANCES];

    exit_t exit[ENTRANCES];

    level_t level[LEVELS];
} shared_data_t;

/**
 * A shared memory control structure.
 */
typedef struct shared_memory {
    /// The name of the shared memory object.
    const char* name;

    /// The file descriptor used to manage the shared memory object.
    int fd;

    /// Address of the shared data block. 
    shared_data_t* data;
} shared_memory_t;