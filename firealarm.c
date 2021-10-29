/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/
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

//#include "carQueue.h"
//#include "sharedMemoryOperations.h"

#define SHARE_NAME "PARKING"
#define LOWER 30
#define UPPER 40
#define LEVELS 5
#define ARSIZE 8

//HELPER FUNCTIONS
int16_t tempGen(int lower, int upper);
double smoothedData(double arr[][ARSIZE], int i, double arg[4]);
void fixedTemp(double arr[][ARSIZE]);

//GLOBAL
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

    //finite loop for testing
    int i = 0;
    while(i < 70)
    {
        //get temp data
        //index always between 0 - 29
        rawL1[0][i % ARSIZE] = tempGen(LOWER,UPPER);
        rawL2[0][i % ARSIZE] = tempGen(LOWER,UPPER);
        rawL3[0][i % ARSIZE] = tempGen(LOWER,UPPER);
        rawL4[0][i % ARSIZE] = tempGen(LOWER,UPPER);
        rawL5[0][i % ARSIZE] = tempGen(LOWER,UPPER);
            
        //check if possible to smooth data
        //need more than 5 entries
        if (i > 4 || SWITCH == 1)
        {
            //get previous array date
            //last 4 elements for next set of data from smoothedData
            double prevData[4] = {rawL1[1][26],rawL1[1][27], rawL1[1][28], rawL1[1][29]}; 
            double L1 = smoothedData(rawL1, i % ARSIZE, prevData);
            rawL1[1][i % 30] = L1;

            //test for fire
            fixedTemp(rawL1);           
            //fixedTemp(rawL2);
            //fixedTemp(rawL3);
            //fixedTemp(rawL4);
            //fixedTemp(rawL5);
            SWITCH = 1;
            
        }

        i++;
        usleep(2000);
        
        if (ALARM == 1)
        {
            break;
        }
    }
    
    //testing 
    for (int i = 0; i < ARSIZE; i++)
    {
        printf("%d. %f\n",i + 1,rawL1[0][i]);
    }
    
    //testing
    printf("--------------------\n");
    for (int i = 0; i < ARSIZE; i++)
    {
        printf("%d. %f\n",i+1,rawL1[1][i]);
    }
    
    //open all gates
    for (int i = 0; i < LEVELS; i++)
    {
     /*   //open exit
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
    
    while(1)
    {
        for (int i = 0; i < size; i++)
        {
            for(int j = 0; j < LEVELS; j++)
            {
                //shm.data->entrance[i].informationSign.display = evacuate[j]; 
            }
        usleep(20000);
        }
    }
    
    
    return 0;
}

//generates temp values randomly with upper and lower limits
int16_t tempGen(int lower, int upper)
{
    int16_t num = (rand() %  (upper - lower + 1)) + lower;
	return num;    
}

double smoothedData(double arr[][ARSIZE], int index, double prevData[4])
{
    //see if index is less than 1 
    //from loop around
    int sum = 0;
    if (index < 5)
    {
        //see if index is less than 0 from second time around
        //can use prev data to find average for first 4 values
        //sum values from new array
        for (int i = 0; index < i ; i++)
        {
            sum += arr[0][i];
        }
        
        //sum values from old array
        for (int i = 30 - (5 - index); i < 30; i++)
        {
            sum += prevData[i];
        }
    }
    else
    {
        //sum the previous 5 values of array
        for (int i = index; index - 5 < i ; i--)
        {
            sum += arr[0][i];
        }
    }
    
    //average the values
    double ret = sum / 5.00;
    return ret;
}

//tests if 90% of readings are over 58 degrees
//if true actives firealarm
void fixedTemp(double arr[][ARSIZE])
{
    //90% of 30 is 27, if see 27 readings over 58 degrees activate fire alarm
	int cnt = 0;
	for (int i = 0; i < ARSIZE; i++)
	{
	    //testing with raw data for now
		if (arr[0][i] > 57)
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






