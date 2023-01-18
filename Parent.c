/*
The code for the parent thread is in the main method, the two scheduling methods are above it 
Most commenting was done in detail in PPSchedule rather than SRTFSchedule, they both follow the same general method but the sorting is by remaining (time) in SRTF rather than priority in PP
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "structs.h"
#include "FileIO.h"
#include "sorts.h"

/*the number of processes to be read in, could've made a function to read this information in from the source file before IO but used this for simplicity*/
#define NUM_PROCESSES 10

/*global mutexes and condition variables*/
pthread_mutex_t mutexBuf1;  /*for access to the filename given as argument to prog*/
pthread_mutex_t mutexBuf2;  /*for access to thread output of average scheduling times*/
pthread_mutex_t mutexFile;  /*for access to source file with process information*/
pthread_cond_t inputReady;  /*for when an input is recieved by parent (fileName/QUIT)*/
pthread_cond_t PPOutputReady;   /*for when output buffer (buffer2) is written to by PP child*/
pthread_cond_t SRTFOutputReady; /*for when output buffer (buffer2) is written to by SRTF child*/

/*method for scheduling by Priority Preemptive method*/
void* PPSchedule(void* arg)
{
    /*Variable/pointer declarations*/
    int scheduleExitFlag = 0;   /*is kept at 0 while there are processes that still need processing*/
    int timeCounter = 0;    /*for keeping track of time in execution, i.e. for checking against arrival times to know when to accept processes*/
    int numSorted = 0;      /*the number of entries that have arrived in the sorted array*/
    int i, j, k, l, m, n;   /*loop indexes*/
    int nextPriority;       /*used to find next highest priority when all processes of current priority are done processing*/
    int turnTot = 0, waitTot = 0;           /*vars to store the total turnaround and waiting times*/
    double turnAvg = 0.0, waitAvg = 0.0;    /*to store the average turnaround and waiting times, to be put in buffer2 after processing*/ 
    int currProcessID = -1;         /*used in logic with printing "Gantt chart", program needs to know when the current process has changed (i.e. a new block in the Gantt chart)*/
    Process** processArray = NULL;  /*array for processes to be stored in after reading in from source file*/
    Process* sortedArray[NUM_PROCESSES];  /*array for arrived process to be stored into after sorting (by priority)*/
    Process* currProcess = NULL;    /*process to be processed for the current time slice*/
    char fileName[11];              /*string which is the filename of the source file to read the processes from, obtained from buffer1*/
    ThreadInput* tempInput;         /*the pointer for the value of "arg", which is a struct containing buffer1 and buffer2*/
    char timeCounterStr[4];
    char currProcessIDStr[4];
    char currProcessBurstStr[4];
    char currProcessRemainingStr[4];    /*used in formatting output text to terminal for Gantt chart*/
    char* ganttChart = (char*)malloc(sizeof(char) * 1500);  /*Average 70 characters per line of Gantt chart, arbitrary number given*/
    strcpy(ganttChart, "Priority Pre-emptive Scheduling Information:\n");

    /*Accessing buffer1's contents, a piece of shared memory*/
    pthread_mutex_lock(&mutexBuf1);
    /*thread blocks and waits for signal from parent that reading is ok*/
    pthread_cond_wait(&inputReady, &mutexBuf1);
    tempInput = (ThreadInput*)arg;  /*set tempInput pointer to the input ThreadInput struct*/
    strcpy(fileName, tempInput->fileName);   /*access the input filename or QUIT and write value to local variable*/
    pthread_mutex_unlock(&mutexBuf1);

    /*Present user prompt, not needed in Assignment part 3
    printf("PP Simulation: ");
    scanf("%s", fileName);*/

    /*Check if QUIT option was chosen or not*/
    if ((strcmp(fileName, "QUIT") != 0)&&(fileName != NULL))  /*do the main program*/
    {
        /*accessing the source file, a shared resource*/
        pthread_mutex_lock(&mutexFile);
        processArray = readJobs(fileName); /*retrieve jobs from source file and set pointer to an array of them. readJobs mallocs a separate piece of memory for the process arrays for SRTF and PP*/
        pthread_mutex_unlock(&mutexFile);

        while (scheduleExitFlag == 0)   /*set to 1 once all processes are done*/
        {
            for (i = 0; i < NUM_PROCESSES; i++) /*add arrived processes to the sorted array*/
            {
                if (processArray[i]->arrival == timeCounter) /*if the process has arrived*/
                {
                    sortedArray[numSorted] = processArray[i];   /*insert into array for sorting*/
                    numSorted++;
                    prioritySort(sortedArray, numSorted);   /*sort array by priority*/
                }
            }

            if (timeCounter != 0)
            {
                currProcessID = currProcess->ID;
            }

            /*big chunk of code to find which process in the ready queue is next to be processed in this time slice*/
            if ((timeCounter != 0) && (currProcess->remaining == 0))    /*if the current process has finished, the next element in the sorted array with the same/next-highest priority and a remaining processing time that is greater than zero must be chosen*/
            {
                n = 0;

                do
                {
                    if (sortedArray[n]->remaining > 0)   /*ignore processes that are done, try to find process with equal or lesser priority to the one just processed*/
                    {
                        if (sortedArray[n]->priority <= currProcess->priority)
                        {
                            currProcess = sortedArray[n];
                        }
                    }
                    n++;

                    if (n == numSorted) /*it has looked through the whole ready queue and there is none with the same or lesser priority, therefore next highest priority number must be chosen*/
                    {
                        n = 0;
                        
                        nextPriority = currProcess->priority + 1;
                        
                        do
                        {
                            if (sortedArray[n]->remaining > 0)   /*ignore processes that are done*/
                            {
                                if (sortedArray[n]->priority == nextPriority)
                                {
                                    currProcess = sortedArray[n];
                                }
                            }
                            n++;

                            if (n == numSorted) /*steadily increment minimum priority number until something is found*/
                            {
                                n = 0;
                                nextPriority++;
                            }
                            
                        }while (currProcess->ID == currProcessID);  
                    }
                }while (currProcess->ID == currProcessID);
            }
            else    /*probably just going to process the current process again*/  
            {
                for (j = 0; j < numSorted; j++) /*go through sorted processes to do some processing*/
                {
                    if (numSorted == 1) /*i.e. the first scheduler iteration, assuming the first process is not given a burst time of 0*/
                    {
                        currProcess = sortedArray[0];   /*only one process to work on*/
                    }
                    else if (sortedArray[j]->remaining > 0)   /*ignore processes that are done, find highest priority*/
                    {
                        if (sortedArray[j]->priority < currProcess->priority)
                        {
                            currProcess = sortedArray[j];
                        }
                    }
                }
            }
            currProcess->remaining = currProcess->remaining - 1;  /*do processing*/

            /*for keeping track of turnaround time/waiting time*/
            if (currProcess->remaining == 0) /*if finish has been reached, calculate turnaround time*/
            {
                currProcess->turnaround = ((timeCounter + 1) - currProcess->arrival);
            }
            
            /*IF CURRENT PROCESS CHANGES, PRINT THE CURRENT STATS TO SCREEN, gantt chart*/
            if (currProcessID != currProcess->ID)
            {
                /*Construct line with processing information piece by piece, and add to the full output*/
                /*printf("Current Time: %d\tProcess ID: %d\tBurst Time: %d\tRemaining Time: %d", timeCounter, currProcess->ID, currProcess->burst, currProcess->remaining);*/
                strcat(ganttChart, "Current Time: ");
                sprintf(timeCounterStr, "%d", timeCounter);
                strcat(ganttChart, timeCounterStr);
                strcat(ganttChart, "\tProcess ID: ");
                sprintf(currProcessIDStr, "%d", currProcess->ID);
                strcat(ganttChart, currProcessIDStr);
                strcat(ganttChart, "\tBurst Time: ");
                sprintf(currProcessBurstStr, "%d", currProcess->burst);
                strcat(ganttChart, currProcessBurstStr);
                strcat(ganttChart, "\tRemaining Time: ");
                sprintf(currProcessRemainingStr, "%d", currProcess->remaining);
                strcat(ganttChart, currProcessRemainingStr);
                strcat(ganttChart, "\n");
            }

            timeCounter++;

            /*check if all jobs are done processing*/
            scheduleExitFlag = 1;
            if (numSorted != NUM_PROCESSES) /*i.e there are still processes that have not 'arrived' yet*/
            {
                scheduleExitFlag = 0;
            }
            else    /*all processes have arrived, but need to check if there is still processing to do*/
            {
                for (k = 0; k < numSorted; k++)
                {   
                    if (sortedArray[k]->remaining != 0)  /*if any of the processes still need processing*/
                    {
                        scheduleExitFlag = 0;   /*continue processing*/
                    }
                }
            } 
            
            /*scheduling has ended, display gantt chart, calculate average turnaround time and waiting time to display to the user*/
            if (scheduleExitFlag == 1)
            {
                /*print Gantt chart*/
                printf("%s\n", ganttChart);

                /*turnaround time*/
                for (l = 0; l < numSorted; l++)
                {   
                    turnTot += sortedArray[l]->turnaround;
                }
                turnAvg = (double)(turnTot)/(numSorted);

                /*waiting time*/
                for (m = 0; m < numSorted; m++)
                {   
                    waitTot += (((sortedArray[m]->turnaround) + 1) - sortedArray[m]->burst);
                }
                waitAvg = (double)(waitTot)/(numSorted);

                /*store this information in buffer2*/
                pthread_mutex_lock(&mutexBuf2);
                tempInput->results->turn = turnAvg;
                tempInput->results->wait = waitAvg;
                /*SIGNAL TO PARENT TO READ UPDATED RESULTS*/
                pthread_cond_signal(&PPOutputReady);     
                pthread_mutex_unlock(&mutexBuf2);       
            }
        }
    }
    else if (strcmp(fileName, "QUIT") == 0) /*QUIT option chosen*/
    {
        printf("PP: terminate\n");
    }

    /*FREE any heap-allocated memory*/
    free(processArray);
    processArray = NULL;

    return NULL;
}

void* SRTFSchedule(void* arg)
{
    /*Variable/pointer declarations*/
    int scheduleExitFlag = 0;   /*is kept at 0 while processes still need processing*/
    int timeCounter = 0;    /*for keep track of time in execution, i.e. for checking against arrival times*/
    int numSorted = 0;      /*the number of entries in the sorted array*/
    int i, j, k, l, m, n;   /*loop indexes*/
    int nextPriority;       /*used to find next lowest remaining time when current process is done processing*/
    int turnTot = 0, waitTot = 0;           /*vars to store the total turnaround and waiting times*/
    double turnAvg = 0.0, waitAvg = 0.0;    /*to store the average turnaround and waiting times*/ 
    int currProcessID = -1;
    Process** processArray = NULL;    /*array for processes to be stored in after reading in*/
    Process* sortedArray[NUM_PROCESSES];  /*array for arrived process to be stored into after sorting*/
    Process* currProcess = NULL;    /*process to be processed for the current time slice*/
    char fileName[11];              /*string which is the filename of the file to read the processes from*/
    ThreadInput* tempInput;         /*the pointer for the value of "arg", which is a struct containing buffer1*/
    char timeCounterStr[4];
    char currProcessIDStr[4];
    char currProcessBurstStr[4];
    char currProcessRemainingStr[4];    /*used in formatting output text to terminal*/
    char* ganttChart = (char*)malloc(sizeof(char) * 1500);  /*Average 70 characters per line of Gantt chart, arbitrary number given*/
    strcpy(ganttChart, "Shortest Remaining Time First Scheduling Information:\n");

    /*Accessing buffer1's contents, a piece of shared memory*/
    pthread_mutex_lock(&mutexBuf1);
    /*thread blocks and waits for signal from parent that reading is ok*/
    pthread_cond_wait(&inputReady, &mutexBuf1);
    tempInput = (ThreadInput*)arg;
    strcpy(fileName, tempInput->fileName);   /*for the input filename or QUIT*/
    pthread_mutex_unlock(&mutexBuf1);

    /*Present user prompt
    printf("SRTF Simulation: ");
    scanf("%s", fileName);*/

    /*Check if QUIT option was chosen or not*/
    if ((strcmp(fileName, "QUIT") != 0)&&(fileName != NULL))  /*do the main program*/
    {
        pthread_mutex_lock(&mutexFile);
        processArray = readJobs(fileName); /*retrieve jobs from source file*/
        pthread_mutex_unlock(&mutexFile);

        while (scheduleExitFlag == 0)   /*set to 1 once all processes are done*/
        {
            for (i = 0; i < NUM_PROCESSES; i++) /*add arrived processes to the sorted array*/
            {
                if (processArray[i]->arrival == timeCounter) /*if the process has arrived*/
                {
                    sortedArray[numSorted] = processArray[i];   /*insert into array for sorting*/
                    numSorted++;
                    remainingTimeSort(sortedArray, numSorted);   /*sort array by priority*/
                }
            }

            if (timeCounter != 0)
            {
                currProcessID = currProcess->ID;
            }

            /*find next process to process*/
            if ((timeCounter != 0) && (currProcess->remaining == 0))    /*if the current process has finished, the next element in the sorted array with the same/next-highest remaining processing time must be chosen*/
            {
                n = 0;

                do
                {
                    if (sortedArray[n]->remaining > 0)   /*ignore processes that are done, try to find process with equal or lesser remaining time to the one just processed*/
                    {
                        if (sortedArray[n]->remaining <= currProcess->remaining)
                        {
                            currProcess = sortedArray[n];
                        }
                    }
                    n++;

                    if (n == numSorted) /*it has looked through the whole ready queue and there is none with the same or lesser remaining time, therefore next highest remaining time must be chosen*/
                    {
                        n = 0;
                        
                        nextPriority = currProcess->remaining + 1;
                        
                        do
                        {
                            if (sortedArray[n]->remaining > 0)   /*ignore processes that are done*/
                            {
                                if (sortedArray[n]->remaining == nextPriority)
                                {
                                    currProcess = sortedArray[n];
                                }
                            }
                            n++;

                            if (n == numSorted) /*steadily increment minimum remaining time until something is found*/
                            {
                                n = 0;
                                nextPriority++;
                            }
                            
                        }while (currProcess->ID == currProcessID);  
                    }
                }while (currProcess->ID == currProcessID);
            }
            else    /*probably just going to process the current process again*/  
            {
                for (j = 0; j < numSorted; j++) /*go through sorted processes to do some processing*/
                {
                    if (numSorted == 1) /*i.e. the first scheduler iteration, assuming the first process is not given a burst time of 0*/
                    {
                        currProcess = sortedArray[0];   /*only one process to work on*/
                    }
                    else if (sortedArray[j]->remaining > 0)   /*ignore processes that are done, find highest priority*/
                    {
                        if (sortedArray[j]->remaining < currProcess->remaining)
                        {
                            currProcess = sortedArray[j];
                        }
                    }
                }
            }
            currProcess->remaining = currProcess->remaining - 1;  /*do processing*/

            /*for keeping track of turnaround time/waiting time*/
            if (currProcess->remaining == 0) /*if finish has been reached, calculate turnaround time*/
            {
                currProcess->turnaround = ((timeCounter + 1) - currProcess->arrival);
            }
            
            /*IF CURRENT PROCESS CHANGES, PRINT THE CURRENT STATS TO SCREEN, gantt chart*/
            if (currProcessID != currProcess->ID)
            {
                /*Construct line with processing information piece by piece, and add to the full output*/
                /*printf("Current Time: %d\tProcess ID: %d\tBurst Time: %d\tRemaining Time: %d", timeCounter, currProcess->ID, currProcess->burst, currProcess->remaining);*/
                strcat(ganttChart, "Current Time: ");
                sprintf(timeCounterStr, "%d", timeCounter);
                strcat(ganttChart, timeCounterStr);
                strcat(ganttChart, "\tProcess ID: ");
                sprintf(currProcessIDStr, "%d", currProcess->ID);
                strcat(ganttChart, currProcessIDStr);
                strcat(ganttChart, "\tBurst Time: ");
                sprintf(currProcessBurstStr, "%d", currProcess->burst);
                strcat(ganttChart, currProcessBurstStr);
                strcat(ganttChart, "\tRemaining Time: ");
                sprintf(currProcessRemainingStr, "%d", currProcess->remaining);
                strcat(ganttChart, currProcessRemainingStr);
                strcat(ganttChart, "\n");
            }

            timeCounter++;

            /*check if all jobs are done processing*/
            scheduleExitFlag = 1;
            if (numSorted != NUM_PROCESSES) /*i.e there are still processes that have not 'arrived' yet*/
            {
                scheduleExitFlag = 0;
            }
            else    /*all processes have arrived, but need to check if there is still processing to do*/
            {
                for (k = 0; k < numSorted; k++)
                {   
                    if (sortedArray[k]->remaining != 0)  /*if any of the processes still need processing*/
                    {
                        scheduleExitFlag = 0;   /*continue processing*/
                    }
                }
            }  

            /*calculate average turnaround time and waiting time to display to the user*/
            if (scheduleExitFlag == 1)
            {
                /*print Gantt chart*/
                printf("%s\n", ganttChart);

                /*turnaround time*/
                for (l = 0; l < numSorted; l++)
                {   
                    turnTot += sortedArray[l]->turnaround;
                }
                turnAvg = (double)(turnTot)/(numSorted);

                /*waiting time*/
                for (m = 0; m < numSorted; m++)
                {   
                    waitTot += (((sortedArray[m]->turnaround) + 1) - sortedArray[m]->burst);
                }
                waitAvg = (double)(waitTot)/(numSorted);

                /*store this information in buffer2*/
                pthread_mutex_lock(&mutexBuf2);
                tempInput->results->turn = turnAvg;
                tempInput->results->wait = waitAvg;
                /*SIGNAL TO PARENT TO READ UPDATED RESULTS*/
                pthread_cond_signal(&SRTFOutputReady);
                pthread_mutex_unlock(&mutexBuf2);
            }
        }
    }
    else if (strcmp(fileName, "QUIT") == 0) /*QUIT option chosen*/
    {
        printf("SRTF: terminate\n");
    }

    /*FREE any heap-allocated memory*/
    free(processArray);
    processArray = NULL;

    return NULL;
}

int main(int argc, char** argv)
{
    /*variable declaration*/
    int exitflag;

    /*define two child threads*/
    pthread_t threadA, threadB;
    /*bounded buffer for program input (source filename)*/
    char* buffer1 = (char*)malloc(sizeof(char) * 11);

    /*create struct for information to be passed to threads (buffers 1 and 2), to be shared between threads A and B and Parent (structs in structs.h)*/
    ThreadInput* input = (ThreadInput*)malloc(sizeof(ThreadInput));
    input->results = (Averages*)malloc(sizeof(Averages));

    exitflag = 0;   /*for looping the program until "QUIT" option is chosen*/
 
    while (exitflag == 0)
    {
        /*initialise mutexes/conds*/
        pthread_mutex_init(&mutexBuf1, NULL);
        pthread_mutex_init(&mutexBuf2, NULL);
        pthread_mutex_init(&mutexFile, NULL);
        pthread_cond_init(&inputReady, NULL);
        pthread_cond_init(&PPOutputReady, NULL);
        pthread_cond_init(&SRTFOutputReady, NULL);

        /*initialise two child threads, with simple error handling*/
        if (pthread_create(&threadA, NULL, &PPSchedule, input) != 0)    /*thread A runs priority preemptive scheduling*/
        {
            perror("Failed to create thread A.");
        } 
        if (pthread_create(&threadB, NULL, &SRTFSchedule, input) != 0)  /*thread B runs shortest remaining time first scheduling*/
        {
            perror("Failed to create thread B.");
        } 

        /*Present user prompt for filename input or QUIT option*/
        printf("Scheduling Simulation: ");
        scanf("%s", buffer1);       /*store input in buffer 'buffer1' (which can only store one filename at a time)*/
        input->fileName = buffer1;  /*set the filename field of the input struct to the value of buffer1*/
        pthread_cond_broadcast(&inputReady);   /*give the child threads the all clear to read the filename/QUIT*/

        /*ensure threads run until completion
        if (pthread_join(threadA, NULL) != 0)
        {
            perror("Failed to join thread.");
        }
        if (pthread_join(threadB, NULL) != 0)
        {
            perror("Failed to join thread.");
        }*/

        /*IF "QUIT" RECEIVED, TELLS A AND B TO TERMINATE AND TERMINATES ITSELF*/
        if (strcmp("QUIT", input->fileName) == 0)
        {
            exitflag = 1;   /*termination message handled in scheduling algorithms methods*/
        }
        else    /*filename given*/
        {
            /*waits for results from the child threads to be written to buffer2 (the results field of input struct) for each scheduling method and displays these results*/
            pthread_cond_wait(&PPOutputReady, &mutexBuf2);
            printf("PP: the average turnaround time =  %f, the average waiting time = %f\n", input->results->turn, input->results->wait);
            pthread_mutex_unlock(&mutexBuf2);

            pthread_cond_wait(&SRTFOutputReady, &mutexBuf2);
            printf("SRTF: the average turnaround time =  %f, the average waiting time = %f\n", input->results->turn, input->results->wait);
        }

        /*destroy mutex/conds*/
        pthread_mutex_destroy(&mutexBuf1);
        pthread_mutex_destroy(&mutexBuf2);
        pthread_mutex_destroy(&mutexFile);
        pthread_cond_destroy(&inputReady);
        pthread_cond_destroy(&PPOutputReady);
        pthread_cond_destroy(&SRTFOutputReady);
    }

    /*free any heap-allocated memory*/
    free(buffer1);
    buffer1 = NULL;
    free(input->results);
    input->results = NULL;
    free(input);
    input = NULL;

    return 0;
}

