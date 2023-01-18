#include <stdio.h>
#include "structs.h"

/*Simple method used to swap two processes*/
void swap(Process* x, Process* y)
{
    Process temp = *x;
    *x = *y;
    *y = temp;
}

/*Sorts the elements of an array (Processes) by their priority value ascending, using selection sort*/
void prioritySort(Process** inArray, int n)
{
    int i, j, minIndx;
    
    for (i = 0; i < (n - 1); i++)
    {
        minIndx = i;

        for (j = (i + 1); j < n; j++)
        {
            if (inArray[j]->priority < inArray[minIndx]->priority)
            {
                minIndx = j;
            }
        }

        swap(inArray[minIndx], inArray[i]);
    }
}

/*Sorts the elements of an array (Processes) by their remaining processing time ascending, using selection sort*/
void remainingTimeSort(Process** inArray, int n)
{
    int i, j, minIndx;
    
    for (i = 0; i < (n - 1); i++)
    {
        minIndx = i;

        for (j = (i + 1); j < n; j++)
        {
            if (inArray[j]->remaining < inArray[minIndx]->remaining)
            {
                minIndx = j;
            }
        }

        swap(inArray[minIndx], inArray[i]);
    }
}
