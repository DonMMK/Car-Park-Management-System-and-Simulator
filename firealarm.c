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
#include <ctype.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

//#include "carQueue.c"
//#include "sharedMemoryOperations.h"

#define SHARE_NAME "PARKING"
#define LOWER 30
#define UPPER 40
#define LEVELS 5
#define ARSIZE 30
#define FIRETOLERANCE 8
#define LOOPLIM 1e9

//HELPER FUNCTIONS
double tempGen(int lower, int upper);
double smoothedData(double arr[][ARSIZE], int i);
void fixedTemp(double arr[][ARSIZE]);
void rateRise(double prevVal, double arr[][ARSIZE]);
void loopLim(int i);

//GLOBALS
int ALARM = 0;
int SWITCH = 0;
//shared_memory_t shm;

//MAIN
int main()
{
    //shared_memory_t shm;
    
    //declare data
    //1st row has raw data
    //2nd row has smoothed data
    //init all to 0
	double dataL1[2][ARSIZE] = {0};
	double dataL2[2][ARSIZE] = {0};
	double dataL3[2][ARSIZE] = {0};
	double dataL4[2][ARSIZE] = {0};
	double dataL5[2][ARSIZE] = {0};

    double prevVal [LEVELS] = {-1};
    int i = 0;
    while(i < LOOPLIM)
    {
        //get temp data
        //index always between 0 - (ARSIZE - 1)
        dataL1[0][i % ARSIZE] = tempGen(LOWER,UPPER);
        dataL2[0][i % ARSIZE] = tempGen(LOWER,UPPER);
        dataL3[0][i % ARSIZE] = tempGen(LOWER,UPPER);
        dataL4[0][i % ARSIZE] = tempGen(LOWER,UPPER);
        dataL5[0][i % ARSIZE] = tempGen(LOWER,UPPER);
            
        //check if possible to smooth data
        //need 5 or more data entries
        if (i > 3)
        {
            //calculate and saved the smoothed value
            double T1 = smoothedData(dataL1, i % ARSIZE);
            double T2 = smoothedData(dataL2, i % ARSIZE);
            double T3 = smoothedData(dataL3, i % ARSIZE);
            double T4 = smoothedData(dataL4, i % ARSIZE);
            double T5 = smoothedData(dataL5, i % ARSIZE);

            //store into the second row of data arrays
            dataL1[1][i % 30] = T1;
            dataL2[1][i % 30] = T2;
            dataL3[1][i % 30] = T3;
            dataL4[1][i % 30] = T4;
            dataL5[1][i % 30] = T5;

            //fixed temp fire test
            fixedTemp(dataL1);           
            fixedTemp(dataL2);
            fixedTemp(dataL3);
            fixedTemp(dataL4);
            fixedTemp(dataL5);           
        }
        
        //get previous values for rate of rise fire testing
        //want to compare with full array so wait until that
        if (i % ARSIZE == 0)
        {
            prevVal[0] = dataL1[1][ARSIZE - 1];
            prevVal[1] = dataL2[1][ARSIZE - 1];
            prevVal[2] = dataL3[1][ARSIZE - 1]; 
            prevVal[3] = dataL4[1][ARSIZE - 1];
            prevVal[4] = dataL5[1][ARSIZE - 1];
            
            //rate of rise fire testing
            rateRise(prevVal[0], dataL1);
            rateRise(prevVal[1], dataL2);
            rateRise(prevVal[2], dataL3);
            rateRise(prevVal[3], dataL4);
            rateRise(prevVal[4], dataL5);
        }

        i++;
        loopLim(i);
        usleep(2000);
        
        //alarm activated
        if (ALARM == 1)
        {
            //open all gates
            for (int i = 0; i < LEVELS; i++)
            {/*
                //open exit
                pthread_mutex_lock(&shm.data->exit[i].gate.gatemutex);
                shm.data->exit[i].gate.status = 'O';
                
                //open entrance
                pthread_mutex_lock(&shm.data->entrance[i].gate.gatemutex);
                shm.data->entrance[i].gate.status = 'O';
                */
                printf("Entrance and Exit %d Open\n", i + 1);
            }
                
            //print evacuate on all signs
            char evacuate[] = {"EVACUATE"};
            int size = sizeof(evacuate) / sizeof(evacuate[0]);
            
            //loop runs endlessly
            //displaying evac message to all screens
            while(1)
            {
                for (int i = 0; i < size; i++)
                {
                    for(int j = 0; j < LEVELS; j++)
                    {
                        //shm.data->entrance[i].informationSign.display = evacuate[j]; 
                        printf("%c",evacuate[i]);
                    }

                usleep(20000);
                }
            }
        }
        
    }
    
    return 0;
}

//generates temp values randomly with upper and lower limits
double tempGen(int lower, int upper)
{
    double num = (rand() %  (upper - lower + 1)) + lower;
	return num;    
}

//uses raw data to create an average of the 5 most previous temperatures
double smoothedData(double arr[][ARSIZE], int index)
{
    double sum = 0;
    for(int i = index; index - 5 < i; i--)
    {
        if (i < 0)
        {
            int j = ARSIZE + i;
            sum += arr[0][j];
        }
        else
        {
            sum += arr[0][i];
        }
    }
    
    double ret = sum / 5.00;
    return ret;
}

//tests if 90% of readings are over 58 degrees
//if true actives firealarm
void fixedTemp(double arr[][ARSIZE])
{
    assert(ALARM == 0); //make sure alarm is not active
    //90% of 30 is 27, if see 27 readings over 58 degrees activate fire alarm
	int cnt = 0;
	for (int i = 0; i < ARSIZE; i++)
	{
		if (arr[1][i] > 57)
		{
			cnt++;
		}
	}
		
	//is 27?
	if (cnt > 26)
	{
	    //set off alarm
		ALARM = 1;
	}	
}	

//takes last value of the previous array and compares with 
//the next array, alarm is activated if the differnce is greater than
// 8 degree
void rateRise(double prevVal, double arr[][ARSIZE])
{
    for (int i = 0; i < ARSIZE; i++)
    {
        if ((arr[1][i] - prevVal) >= FIRETOLERANCE && prevVal != -1)
        {
            ALARM = 1;
        }
    }
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