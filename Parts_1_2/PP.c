/*
NAME: PP.c
PURPOSE: A program to simulate a Priority Pre-emptive CPU scheduler.
AUTHOR: Lachlan Murray (19769390)
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "structs.h"
#include "FileIO.h"
#include "sorts.h"

#define NUM_PROCESSES 10

ThreadInput* tempInput;

int main(int argc, char** argv)
{
    /*Variable/pointer declarations*/
    int scheduleExitFlag = 0;   /*is kept at 0 while processes still need processing*/
    int timeCounter = 0;    /*for keep track of time in execution, i.e. for checking against arrival times*/
    int numSorted = 0;      /*the number of entries in the sorted array*/
    int i, j, k, l, m, n;   /*loop indexes*/
    int nextPriority;       /*used to find next highest priority when all processes of current priority are done processing*/
    int turnTot = 0, waitTot = 0;           /*vars to store the total turnaround and waiting times*/
    double turnAvg = 0.0, waitAvg = 0.0;    /*to store the average turnaround and waiting times*/ 
    int currProcessID = -1;
    Process** processArray = NULL;  /*array for processes to be stored in after reading in*/
    Process* sortedArray[NUM_PROCESSES];  /*array for arrived process to be stored into after sorting*/
    Process* currProcess = NULL;    /*process to be processed for the current time slice*/
    char fileName[11];              /*string which is the filename of the file to read the processes from*/
    ThreadInput* tempInput;         /*the pointer for the value of "arg", which is a struct containing buffer1*/
    char timeCounterStr[4];
    char currProcessIDStr[4];
    char currProcessBurstStr[4];
    char currProcessRemainingStr[4];    /*used in formatting output text to terminal*/
    char* ganttChart = (char*)malloc(sizeof(char) * 1500);  /*Average 70 characters per line of Gantt chart, arbitrary number given*/
    strcpy(ganttChart, "");
    
    /*Allocate memory*/
    tempInput = (ThreadInput*)malloc(sizeof(ThreadInput));
    tempInput->results = (Averages*)malloc(sizeof(Averages));

    /*Present user prompt*/
    printf("PP Simulation: ");
    scanf("%s", fileName);

    /*Check if QUIT option was chosen or not*/
    if ((strcmp(fileName, "QUIT") != 0)&&(fileName != NULL))  /*do the main program*/
    {
        processArray = readJobs(fileName); /*retrieve jobs from source file*/

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

            /*check if all jobs are done processing, and there aren't any left to process*/
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

                tempInput->results->turn = turnAvg;
                tempInput->results->wait = waitAvg;   

                printf("PP: the average turnaround time =  %f, the average waiting time = %f\n", tempInput->results->turn, tempInput->results->wait);
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

    return 0;
}
            
    
