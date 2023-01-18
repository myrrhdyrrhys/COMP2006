/*
File to handle reading in of job information from input file
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"

#define BUFFER_LENGTH 10
#define NUM_PROCESSES 5     /*set to the number of lines in the input file*/

Process** readJobs(char* fileName)
{
    FILE *sourceFile;
    char sourcePath[9];
    char buffer[BUFFER_LENGTH];
    int processCounter = 0;

    int i;
    Process** returnArray = (Process**)malloc(sizeof(Process*) * NUM_PROCESSES); /*allocate memory to container*/
    for (i = 0; i < NUM_PROCESSES; i++)
    {
        returnArray[i] = (Process*)malloc(sizeof(Process));
    }

    strcpy(sourcePath, fileName);
    sourceFile = fopen(sourcePath, "r");
    
    if (sourceFile == NULL)
    {
        perror("Error in opening file, perhaps specify another one.\n");
    }
    else
    {
        while (fgets(buffer, BUFFER_LENGTH, sourceFile) != NULL)    /*loop through each line*/
        {
            sscanf(buffer, "%d %d %d\n", &returnArray[processCounter]->arrival, &returnArray[processCounter]->burst, &returnArray[processCounter]->priority);

            returnArray[processCounter]->remaining = returnArray[processCounter]->burst;  /*set remaining process time to initially be full burst time*/

            returnArray[processCounter]->ID = processCounter;    /*fill ID field for identification in chart*/
    
            processCounter++;
            
            if (ferror(sourceFile))
            {
                perror("Error reading from source file!\n");
            }
        }
        
        fclose(sourceFile);
    }

    return returnArray;
}
