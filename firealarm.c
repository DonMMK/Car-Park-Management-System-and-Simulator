#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>


#define LOWER 30
#define UPPER 40
#define LEVELS 5
#define ARSIZE 30


//HELPER FUNCTIONS
int16_t tempGen(int lower, int upper);
int usleep(int arg);
double smoothedData(double arr[][ARSIZE], int i);
void fixedTemp(double arr[][ARSIZE]);


//GLOBAL
int ALARM = 0;

//MAIN
int main()
{
    //declare data
    //init all to 0
	double rawL1[2][ARSIZE] = {0};
	double rawL2[2][ARSIZE] = {0};
	double rawL3[2][ARSIZE] = {0};
	double rawL4[2][ARSIZE] = {0};
	double rawL5[2][ARSIZE] = {0};

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
        if (i > 4)
        {
            double L1 = smoothedData(rawL1, i % ARSIZE);
            rawL1[1][i] = L1;

            //test for fire
            fixedTemp(rawL1);           
            //fixedTemp(rawL2);
            //fixedTemp(rawL3);
            //fixedTemp(rawL4);
            //fixedTemp(rawL5);
            
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
    
    //OPEN ALL GATES
    if (ALARM == 1)
    {
        printf("ALARM TRIPPED");
    }
    return 0;
}

//generates temp values randomly with upper and lower limits
int16_t tempGen(int lower, int upper)
{
    int16_t num = (rand() %  (upper - lower + 1)) + lower;
	return num;    
}

double smoothedData(double arr[][ARSIZE], int index)
{
    //sum the previous 5 values of array
    int sum = 0;
    for (int i = index; index - 5 < i ; i--)
    {
        sum += arr[0][i];
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

// Logic for the fire alarm:
//int difference = 0;
//void RateofRise(int arr[][]){
//	if(difference >= 8){
//		ALARM = 1;
//	}	
//}

// This function 
// if the most recent temperature is 8°C (or more) hotter than the 30th most recent temperature,
// the temperature is considered to be growing at a fast enough rate that there must be a fire.
void RateofRise(double arr[][30]){
    int i = 0;
    for (i = 0; i < 30; i++){
        if ( abs ( arr[0][i] - arr[0][i+1]) >= 8){
            ALARM = 1;;
        }
    }

}




