#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "sharedMemoryOperations.c"

#define SHARE_NAME "PARKING"
#define LOWER 30
#define UPPER 40
#define LEVELS 5
#define ARSIZE 30


// Function declarations
int16_t tempGen(int lower, int upper);
//int usleep(int arg);
int16_t smoothedData(int16_t arr[ARSIZE]);
void fixedTemp(int16_t arr[LEVELS][ARSIZE], int index);
//void RateofRise(int16_t arr[LEVELS][ARSIZE]);
void RateofRise(int16_t arr[LEVELS][ARSIZE], int index);
int16_t tempSensorSimulate(void);
int generateRandom(int lower, int upper);
int16_t Find_median(int16_t array[ARSIZE] , int n);
void Array_sort(int16_t array[ARSIZE] , int n);



// Global Variables
int ALARM = 0;
int16_t rawData[LEVELS][ARSIZE] = {0};
//int16_t rawData[LEVELS][ARSIZE] = {0, 59, 60, 58, 59, 60, 58, 59, 60, 58, 59, 60, 58, 59, 60, 58, 59, 60, 58, 59, 60, 58, 59, 60, 58, 59, 60, 58, 59, 60};
//int16_t smoothData[LEVELS][ARSIZE] = {0, 59, 60, 58, 59, 60, 58, 59, 60, 58, 59, 60, 58, 59, 60, 58, 59, 60, 58, 59, 60, 58, 59, 60, 58, 59, 60, 58, 59, 60};
int16_t smoothData[LEVELS][ARSIZE] = {0};
int fire = 0;
int16_t mostRecentReadings[5];
int16_t currentTemp = 24;
shared_memory_t shm;

//MAIN
int main()
{
    create_shared_object_R(&shm, SHARE_NAME);

    int j = 0;
    int levelOfFire = 0;
    fire = 2;
    while(ALARM == 0) {
        // Loop through levels checking temperatures
        for (int i = 0; i < LEVELS; i++)
        {
            //int16_t randomboy = tempSensorSimulate();
            //printf("in loopskiskiskiski with value %d", randomboy);
            rawData[i][j % ARSIZE] = shm.data->level[i].tempSensor;
            printf("array value = %d\n", rawData[i][j% ARSIZE]);
            //check if possible to smooth data
            //need more than 5 entries
            //printf("nah yeah??\n");

            
            if (j > 4) {
                // Create array of 5 most recent readings
                int index = j % ARSIZE;
                int z = 0;
                for (int currentIndex = index-5; currentIndex < index; currentIndex++) { // NEED TO ADD 1 ONTO THE 'INDEX' HERE????
                    mostRecentReadings[z] = rawData[i][currentIndex];
                    z++;
                }

                int16_t medianReading = smoothedData(mostRecentReadings);
                smoothData[i][j % ARSIZE] = medianReading; // j-4?

                // Run both detection methods to test for fire
                fixedTemp(smoothData, i);           
                RateofRise(smoothData, i);
                levelOfFire = i;
                if (ALARM == 1) {
                    break;
                }
            }
            usleep(2000);    
        }
        j++;
    }
    // Set fire status 
    if (ALARM == 1)
    {
        //printf("ALARM TRIPPED on level %d\n", levelOfFire);
        shm.data->level[levelOfFire].fireAlarm = '1';
        
    }
    // RateofRise(smoothData);
    // printf("Alarm = %d\n", ALARM);
    // Create shared me
}


//tests if 90% of readings are over 58 degrees
//if true actives firealarm
void fixedTemp(int16_t arr[LEVELS][ARSIZE], int index)
{
    //90% of 30 is 27, if see 27 readings over 58 degrees activate fire alarm
	int cnt = 0;
    //for (int i = 0; i < LEVELS; i++) {
        for (int j = 0; j < ARSIZE; j++)
        {
            //testing with raw data for now
            if (arr[index][j] >= 58)
            {
                cnt++;
                	// Check if 90 percent of the readings have exceeded fixed temp
                if (cnt >= ARSIZE * 0.9)
                {
                    //set off alarm
                    ALARM = 1;
                    break;
                }	
            }
        }
    //}
}	

void RateofRise(int16_t arr[LEVELS][ARSIZE], int index){
    //for (int i = 0; i< LEVELS; i++) {
        if (arr[index][0] != 0 && arr[index][ARSIZE-1] - arr[index][0] >= 8) {
            ALARM = 1;
        }
    //}
}

int16_t tempSensorSimulate(void) {
    int16_t temperature = 24;

    //for (;;) {
        usleep(2000);
        if (fire == 1) { // (Fixed temp fire detection data)
            // Generate temperatures to trigger fire alarm via Temps > 58 degrees
            temperature = (int16_t) generateRandom(58, 65);
        }
        else if (fire == 2) { // (Rate-of-rise fire detection data)
            // Generate temperatures to trigger fire alarm via Rate-of-rise (Most recent temp >= 8 degrees hotter than 30th most recent)
            currentTemp = currentTemp + 2;
            //currentTemp = shm.data...tempSensor;
            temperature = (int16_t) generateRandom(currentTemp, currentTemp + 2);
            //shm.data...tempSensor = temperature;
        }
        else {
            // Generate normal temperatures to avoid setting off fire alarm
            temperature = (int16_t) 24;

        }    
    //}
    return temperature;
}

int generateRandom(int lower, int upper)
{    
    int num = (rand() % (upper - lower + 1)) + lower;
    return num;
}

int16_t smoothedData(int16_t arr[5])
{
    // Sort the array in ascending order
    Array_sort(arr, 5);
    // Now pass the sorted array to calculate the median of array.
    int16_t median = Find_median(arr , 5);    
    return median;
}

// Following array sorting and median functions found on https://www.includehelp.com/c-programs/calculate-median-of-an-array.aspx.
void Array_sort(int16_t array[ARSIZE] , int n)
{ 
    // declare some local variables
    int16_t i=0 , j=0 , temp=0;

    for(i=0 ; i<n ; i++)
    {
        for(j=0 ; j<n-1 ; j++)
        {
            if(array[j]>array[j+1])
            {
                temp        = array[j];
                array[j]    = array[j+1];
                array[j+1]  = temp;
            }
        }
    }
}


// function to calculate the median of the array
int16_t Find_median(int16_t array[ARSIZE] , int n)
{
    int16_t median = 0;
    
    // if number of elements are even
    if(n%2 == 0)
        median = (array[(n-1)/2] + array[n/2])/2.0;
    // if number of elements are odd
    else
        median = array[n/2];
    
    return median;
}