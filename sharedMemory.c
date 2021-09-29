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
    entrance_t entrance1;
    entrance_t entrance2;
    entrance_t entrance3;
    entrance_t entrance4;
    entrance_t entrance5;

    exit_t exit1;
    exit_t exit2;
    exit_t exit3;
    exit_t exit4;
    exit_t exit5;

    level_t level1;
    level_t level2;
    level_t level3;
    level_t level4;
    level_t level5;

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