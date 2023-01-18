This folder exists if you wanted to see the two schedulers working by themselves.
Right now the files are configured to run the PPScheduler with the input file "smlInput".
Just make and run ./PPScheduler and then enter "smlInput" when the prompt appears.
To test other sized files, make sure to edit the constant "NUM_PROCESSES" in PP.c/SRTF.c and FileIO.c
To make the SRTF scheduler, just replace every instance of PP with SRTF in the makefile
    :%s/PP/SRTF/g   in vim
