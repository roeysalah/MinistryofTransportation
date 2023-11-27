
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>

#define N 5
#define FIN_PROB 0.1
#define MIN_INTER_ARRIVAL_IN_NS 8000000
#define MAX_INTER_ARRIVAL_IN_NS 9000000
#define INTER_MOVES_IN_NS 100000
#define SIM_TIME 2
#define NUM_OF_CARS (int) ((SIM_TIME * 1000000000)/MIN_INTER_ARRIVAL_IN_NS) // 1.0e9 = 1000000000 for nano sec
#define CAR_CELLS 4*(N-1)
#define NUM_GEN 4


void* GenerateCars(void *arg);
void* DrivingCars(void*arg);
void* PrintCircle(void *arg);

pthread_mutexattr_t attr;
pthread_t print;
pthread_mutex_t car_cell[CAR_CELLS]={0};
pthread_t cars[4][NUM_OF_CARS]={0};
pthread_t generatorCars[4];



struct timespec initial_time;



int generatorFlag[4]={0};
int generatorNum[4]={0};



int main()
{
    srand(time(NULL));
    int i;

    // Init Mutex for the program
    if(pthread_mutexattr_init(&attr)!=0)
    {
        perror("pthread_mutexattr_init error!!");
        exit(1);
    }

    // Init Mutex for program and for car cells !!
    for (i=0;i<CAR_CELLS;i++)
    {
        if(pthread_mutex_init(&car_cell[i],&attr))
        {
            perror("pthread_mutex_init error in cell !!");
            exit(1);
        }
    }

    // Initial Clock for the program !!
    clock_gettime(CLOCK_REALTIME,&initial_time);

    // Init Print Thread !!
    if (pthread_create(&print,NULL,PrintCircle,NULL)!=0)
    {
        perror("error in creating pthread for print !!");
        exit(1);
    }

    // Init Thread for each Generator !!

    for (i=0;i<NUM_GEN;i++)
    {
        generatorNum[i]=i;
        if (pthread_create(&generatorCars[i],NULL,GenerateCars,&generatorNum[i])!=0)
        {
            perror("error in creating pthread for print !!");
            exit(1);
        }
    }

    //Wait for print function to finish execution

    pthread_join(print,NULL);


    //Free Mutex Resources

    for (i=0;i<CAR_CELLS;i++)
    {
        if(pthread_mutex_destroy(&car_cell[i]))
        {
            perror("pthread_mutex_destroy error in cell !!");
            exit(1);
        }
    }

    if(pthread_mutexattr_destroy(&attr)!=0)
    {
        perror("pthread_mutexattr_destroy error!!");
        exit(1);
    }
    return 0;

}



void* GenerateCars(void *arg)
{
    int i;
    int flag=0,carFlag=0,numberOfTheCar=0;
    int indexGenNum = (*(int *)arg);
    struct timespec current_time;
    struct timespec car_time;
    double SimInterval = SIM_TIME * 1.0e9 ;  // we are using the * 1.0e9 because we work with nano second here !!
    double CarInterval;
    double timeSampling ;

    while (!flag)
    {
        clock_gettime(CLOCK_REALTIME,&current_time);
        timeSampling = (double)((current_time.tv_sec - initial_time.tv_sec)) * 1.0e9 +
                (double) (current_time.tv_nsec - initial_time.tv_nsec) ;

        if (timeSampling < SimInterval )     //If the simulation not over yet
        {
            CarInterval = (rand() % (MAX_INTER_ARRIVAL_IN_NS-MIN_INTER_ARRIVAL_IN_NS + 1)) +
                    MIN_INTER_ARRIVAL_IN_NS ;//number = (rand() % (upper - lower + 1)) + lower
            clock_gettime(CLOCK_REALTIME,&car_time);

            // Generate Car Interval Sample - wait until we have reach the time interval to generate new car
            while(!carFlag)
            {
                clock_gettime(CLOCK_REALTIME,&current_time);
                timeSampling = (double)((current_time.tv_sec - car_time.tv_sec)) * 1.0e9 +
                        (double) (current_time.tv_nsec - car_time.tv_nsec) ;

                if (timeSampling >= CarInterval) // we reach the time interval
                {
                    carFlag=1;
                }
            }
            if (pthread_create(&cars[indexGenNum][numberOfTheCar],NULL,DrivingCars,((void *)&indexGenNum))!=0)
            {
                perror("error in creating pthread for car !!");
                exit(1);
            }
            numberOfTheCar++;
            carFlag=0;
        }
        else
        {
            flag = 1 ;
        }
    }
    // Simulation over in this Generator
    generatorFlag[indexGenNum]=1;
    for (i=0;i<numberOfTheCar;i++)
    {
        pthread_join(cars[indexGenNum][i],NULL);
    }

    return 0;

}






