// ------------------------------------------- HEADER --------------------------------------------- // 
#include <assert.h>
#include "sharedMemoryOperations.c"

#define SHARE_NAME "PARKING"
#define FIRETOLERANCE 8
#define ARSIZE 30
#define LOOPLIM 1e9


// ------------------------------------ FUNCTION DECLERATIONS ------------------------------------- // 
int16_t smoothedData(int16_t arr[ARSIZE]);
int16_t findMedian(int16_t array[ARSIZE] , int n);
void arraySort(int16_t array[ARSIZE] , int n);
int fixedTemp(int16_t arr[LEVELS][ARSIZE], int index);
int rateOfRise(int16_t arr[LEVELS][ARSIZE], int index);
void loopLim(int i);


// --------------------------------------- PUBLIC VARIABLES --------------------------------------- // 
int ALARM = 0;
int16_t rawData[LEVELS][ARSIZE] = {0};
int16_t smoothData[LEVELS][ARSIZE] = {0};
shared_memory_t shm;

// --------------------------------------------- MAIN --------------------------------------------- // 
int main()
{ 
    // Initialise Shared memory
    create_shared_object_R(&shm, SHARE_NAME);
    // Declare local variables
    int j = 0, levelOfFire = 0, lim = 0;
    while(lim < LOOPLIM) {
        // Loop through levels checking temperatures
        for (int i = 0; i < LEVELS; i++)
        {
            // Read temperature data from temperature sensor
            rawData[i][j % ARSIZE] = shm.data->level[i].tempSensor;
            printf("Temperature sensor reading from level %d = %d\n", i+1, rawData[i][j% ARSIZE]);
            if (j > 4) {
                // Find 5 most recent temperature readings
                int index = j % ARSIZE, z = 0, numElements = 5;
                int16_t mostRecentReadings[5];
                for (int currentIndex = index - numElements; currentIndex < index; currentIndex++) {
                    mostRecentReadings[z] = rawData[i][currentIndex];
                    z++;
                }
                // Calculate the recent smoothed data
                int16_t medianReading = smoothedData(mostRecentReadings);
                smoothData[i][j % ARSIZE] = medianReading;
                // Check temp readings on every floor for both methods of detection
                if (fixedTemp(smoothData, i) || rateOfRise(smoothData, i)) {
                    levelOfFire = i;
                    break;
                }
            }
            usleep(2000);    
        }
        loopLim(lim);
        j++;
        // Set stauts of fire alarm to '1' in shared memory when activated
        if (ALARM == 1) {
            shm.data->level[levelOfFire].fireAlarm = '1';
            break;
        }
    }
}

// --------------------------------------- HELPER FUNCTIONS --------------------------------------- // 

// Fixed temperature fire detection - Tests if 90% of readings 58 degrees or over
int fixedTemp(int16_t arr[LEVELS][ARSIZE], int index)
{
    // Check alarm is not already active
    assert(ALARM == 0); 
    // Count recent temperature readings over 58 degrees
	int cnt = 0;
    for (int j = 0; j < ARSIZE; j++)
    {
        if (arr[index][j] >= 58)
        {
            cnt++;
            // Check if 90 percent of the readings have exceeded maximum temp
            if (cnt >= ARSIZE * 0.9)
            {
                // Set off alarm
                ALARM = 1;
                return ALARM;
            }	
        }
    }
    return 0;
}	

int rateOfRise(int16_t arr[LEVELS][ARSIZE], int index){
    // Check alarm is not already active
    assert(ALARM == 0);
    // Check if the most recent temperature is 8°C (or more) hotter than the 30th most recent temperature 
    if (arr[index][0] != 0 && arr[index][ARSIZE-1] - arr[index][0] >= FIRETOLERANCE) {
        ALARM = 1;
        return ALARM;
    }
    return 0;
}

int16_t smoothedData(int16_t arr[5])
{
    // Require 5 most recent temperatures as per the spec
    int numElements = 5;
    // Sort the array in ascending order
    arraySort(arr, numElements);
    // Pass the sorted array to calculate the median of array.
    int16_t median = findMedian(arr , numElements);    
    return median;
}

// Following array sorting and median functions found on https://www.includehelp.com/c-programs/calculate-median-of-an-array.aspx.
void arraySort(int16_t array[ARSIZE] , int n)
{ 
    // Declare local variables
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
int16_t findMedian(int16_t array[ARSIZE] , int n)
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

//checks if the upper bound of a loop has been reached
//terminates program with error message if true
void loopLim(int i)
{
	if (i >= LOOPLIM)
	{
		printf("ERROR: Loop Limit Upper Bound Exceeded, Program Will Close");
		exit(1);
	}
}