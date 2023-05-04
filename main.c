#include "HackEnrollment.h"
#include <stdio.h>
#include <string.h>
#define NUM_OF_ARG 7
#define FIRST_FILE_IN_ARGV 2
#define FLAG_IN_ARGV 1

int main(int argc, char** argv) {
    if (argc != NUM_OF_ARG){
        return 1;
    }
    int i=FIRST_FILE_IN_ARGV;
    FILE* students = fopen(argv[i++], "r");
    FILE* courses = fopen(argv[i++],"r");
    FILE* hackers = fopen(argv[i++], "r");
    FILE* queues = fopen(argv[i++], "r");
    FILE* target = fopen(argv[i++],"w");
    if(students == NULL || courses == NULL || hackers == NULL || queues == NULL || target == NULL){
        return 1;
    }
    EnrollmentSystem sys = createEnrollment(students,courses,hackers);
    if(!strcmp(argv[FLAG_IN_ARGV],"-i")){
        sys = lowerCase(sys);
    }
    sys = readEnrollment(sys,queues);
    hackEnrollment(sys,target);
    EnrollmentSysDestroy(sys);
    fclose(students);
    fclose(courses);
    fclose(hackers);
    fclose(queues);
    fclose(target);
    return 0;
}