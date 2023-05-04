#ifndef HACKENROLLMENT_HACKENROLLMENT_H
#define HACKENROLLMENT_HACKENROLLMENT_H

#include <stdio.h>

typedef struct EnrollmentSys_t * EnrollmentSystem;
typedef struct course * Course;
typedef struct student * Student;

//do we need it?
//typedef enum {ENROLLMENTSYS_SUCCESS,ENROLLMENTSYS_ALLOCFAILED,ENROLLMENTSYS_BAD_PARAM,ENROLLMENTSYS_ERROR} EnrollmentSystemError;

/**creates a new EnrollmentSystem object including 4 FILE* - students, courses, hackers, queues.
 *students - contains a list of the students
 * courses - contains name of courses and size
 * hackers - contains the hackers names, desired courses, their friends and rivals*/
EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers);

/** Reads the files from the EnrollmentSys and returns FILE* courses queues.
 * @param sys: an EnrollmentsSystem which contains files as previously mentioned
 * @param queues: an FILE* which includes in the EnrollmentSystem struct.
 * @return the EnrollmentSys which contains the FILE* queues.*/
EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queues);

/** Returns a FILE* output which contains new queues including the hackers, based on IsraeliQueue
 * if it  fails - returns null. */
void hackEnrollment(EnrollmentSystem sys, FILE* out);

/**in case we get a flag - returns the same enrollmentSys with lowerCase names in student array */
EnrollmentSystem lowerCase(EnrollmentSystem);

/**Free all the allocated parameters of sys, then free sys itself */
void EnrollmentSysDestroy(EnrollmentSystem);

#endif //HACKENROLLMENT_HACKENROLLMENT_H
