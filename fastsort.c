#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "sort.h"

#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>

/*
    Information:

    Operating Systems Fall 2020
    Homework 2, due Oct 11
    Nathan Cheshire, nvc29
*/

/*
    How I wrote this:

    Basically same as the first homework,
    I followed numerous rabit holes on the offical documentation
    for commands to find out what certain calls actually do, return, and require
*/

/*
    Program flow:

    -make sure the arguments passed in are correct
    -if they're correct we can proceed, else we exit and tell error
    -figure out how many lines our input file is and read all those into an array full of record structs
    -call qsort on our array
    -overwrite the old data with our new sorted data
    -exit cleanly
*/

//record type so that we can get the record or the key
typedef struct __record_t {
    unsigned int key;
    unsigned int record[NUMRECS];
} record_t;

//pointer to void means general purpose pointer
//compare function for qsort is the fourth argument
//function must return postivie for first arg and negative for second arg
int cmpfunc(const void *i, const void *j) {
    int *ii = (int *) i;
    int *jj = (int *) j;
    return (*ii - *jj);
}

//how to properly use the program
void usage (char *smp) {
    fprintf(stderr, "usage: %s <-i file -o file>\n",smp);
    exit(1);
}

//number of args and pointer to args (*argv is the program name *(argv + 1) should be input *(argv + 2) should be output)
int main(int argc, char *argv[]) {
    //IO files
    char *in = "";
    char *out = "";

    //stat struct var, used for fstat for info about file
    struct stat buff;

    //check for proper arguments based on code of c
    int comArgs;
    comArgs = getopt(argc, argv, "i:o:");

    //fail since we did not receive the proper parameters
    if (comArgs == -1)
        usage(argv[0]);

    //parse the input and output parameters
    //optarg points to argv memory, sting duplicate in and out files from args
    while (comArgs != -1) {
        switch (comArgs) {
            case 'i':
                in = strdup(optarg);
                break;

            case 'o':
                out = strdup(optarg);
                break;

            default:
                usage(argv[0]);
                //improper arguements
        }

        comArgs = getopt(argc, argv, "i:o:");
    }

    //open unsorted record file for reading only
    int currentFile = open(in, O_RDONLY);
    
    if (currentFile < 0) {
        fprintf(stderr,"Error: cannot open file %s\n",in);
        exit(1);
    }

    int lineCount = 0;

    //fstat gets the status of the passed in file,we use this to figure out how many lines the file is
    fstat(currentFile, &buff);

    //trick to find number of lines in a file
    int numline = buff.st_size / sizeof(record_t); 

    //init array of structs with length number of lines
    record_t recordArr[numline];

    while (1) {
        //read from file into current array spot "sizeof(record_t)" bytes
        int readEr = read(currentFile, &(recordArr[lineCount]), sizeof(record_t));

        if (readEr == 0)
            break;
            //EOF so break

        if (readEr < 0) {
            fprintf(stderr, "Error in loop reading\n");
            exit(1);
        }

        lineCount += 1;
    }

    int closeEr = close(currentFile);

    if (closeEr < 0) {
        fprintf(stderr,"Error: cannot close file %s\n",in);
        exit(1);
    }

    //sort the array (array, size of array, size of structure, compare function)
    qsort(recordArr, numline, sizeof(record_t), cmpfunc);

    //open file for writing only, create if it doesn't exist, only overwrite what we need via O_TRUNC
    currentFile = open(out, O_WRONLY|O_CREAT|O_TRUNC, S_IRWXU);//S_IRWXU is really S_IRUSR, S_IWUSR, S_IXUSR and gives regular rwx permissions

    //negative one so error opening output file
    if (currentFile < 0) {
        fprintf(stderr,"Error: cannot open file %s\n",out);
        exit(1);
    }

    //write whole sorted array to output file
    int writeEr = write(currentFile, &(recordArr), sizeof(recordArr));

    if (writeEr < 0) {
        fprintf(stderr,"Error: cannot write to file %s\n",out);
        exit(1);
    }

    closeEr = close(currentFile);

    if (closeEr < 0) {
        fprintf(stderr,"Error: cannot close file %s\n",out);
        exit(1);
    }

    //why is this common practice actually?
    return 0;
}