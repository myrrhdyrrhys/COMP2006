#ifndef STRUCTS_H
#define STRUCTS_H

typedef struct{
    int ID;         /*A way of identifying a process for the purpose of printing its processing information in the "Gantt chart". Is currently assigned simply by the line number it sits on in the source file*/
    int arrival;    /*The time that the process arrives in the ready queue*/
    int burst;      /*The time units required to complete the processes processing*/
    int priority;   /*The priority number assigned to this process*/
    int turnaround; /*The total time (including waiting time) the process took to process till completion*/
    int remaining;  /*A field that is updated during processing, the time remaining to finish processing*/
}Process;

typedef struct{
    double turn;    /*overall average turnaround time for the scheduling method*/
    double wait;    /*overall average waiting time for the scheduling method*/
}Averages;

typedef struct{
    char* fileName;     /*i.e. buffer1*/
    Averages* results;  /*buffer2*/
}ThreadInput;

#endif
