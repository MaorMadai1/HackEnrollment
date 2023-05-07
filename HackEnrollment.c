#include "IsraeliQueue.h"
#include "HackEnrollment.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#define STRING_LEN 5
#define STUDENTS_SIZE 30
#define NUM_OF_STRING_PARAM 3
#define STUDENTS_FRIENDS_THRESHOLD 20
#define STUDENTS_RIVALS_THRESHOLD 0
#define STUDENTS_FRIENDS_RES 20
#define STUDENTS_RIVALS_RES (-20)
#define STUDENTS_STRANGERS_RES 0
#define TO_LOWER_DIFF ('a'-'A')
//
#define DEFAULT_THRESHOLD 0
#define NUM_OF_COURSE_PARAMS 2
#define MOVE_ROW_DOWN '\n'
#define NON_ID (-2147483600)   //used to be (-1) until announcement that courses can be negative
#define ZERO_CHAR '0'
#define ADD_DECIMAL_PLACE 10
#define SPACE_BETWEEN_INTS ' '
#define NEGATIVE (-1) //note: already defined in israeliqueue.c
#define NUM_OF_NEW_FUNCTIONS 3 //num of new friendship functions added to queues
#define MIN_SUCCESSFUL_REGISTRATIONS 2
#define END_STR '\0'
#define NEGATIVE_SIGN '-'
#define ENOUGH_DIGITS 100000000

typedef struct student {
    int studentID;
    int totalCredits;
    double gpa;
    char* name;
    char* surname;
    char* city;
    char* department;
    int* desiredCourses;
    int* friendIDs;
    int* rivalIDs;
} *Student;

typedef struct course {
    int courseNumber;
    int size;
    IsraeliQueue queue;
} * Course;

typedef struct EnrollmentSys_t {
    Student* studentsArr;
    Student* hackersArr;
    Course* coursesArr;
} * EnrollmentSystem;


//MAOR funcs:
static Course searchCourse(Course* courseArr, int course_num);
int friendFuncFile(void* h1,void* s1);
int friendFuncIDs (void* a1, void* a2);
static Student* createStudentsArr(FILE* students);


//course functions declarations:
Course createCourse(FILE* courses);
Course* createCourseArr(FILE* courses);
static void copyCourseArr(Course* pasteArr, Course* copyArr, int size);

//hacker functions declarations:
Student* updateHackerParams(Student* studentArr, FILE* hackers);
static int* reallocateIntArr(int newInteger, int* oldArr, int numOfInts);
static void copyIntArr(int* pasteArr, int* copyArr, int size);
static Student searchStudent(Student* studentArr, int hackerID);
int* updateArrOfInts(FILE* file);
int buildInteger(char c, int num);

//friendship function name distance declarations:
int friendFuncNameDistance (void* a, void* b);
int calculateASCIIDifference(char* str1, char* str2);
int compareStudents(void* a1,void* a2);

//hackEnrollment functions
IsraeliQueueError modifyIsraeliQueuesInSystem(EnrollmentSystem sys);
IsraeliQueueError enqueueHackers(EnrollmentSystem sys);
Student checkPlacementOfHackers(EnrollmentSystem sys);
static int getPlaceInQueue(IsraeliQueue q, Student s);
void printQueuesIntoFile(EnrollmentSystem sys, FILE* out);

//destruction functions
void destroyCoursesArr(Course* coursesArr);
void destroyStudentsArr(Student* studentArr);
void EnrollmentSysDestroy(EnrollmentSystem sys);


//functions:
EnrollmentSystem createEnrollment(FILE* students, FILE* courses, FILE* hackers) {
    EnrollmentSystem sys = (EnrollmentSystem) malloc(sizeof(*sys));
    if (!sys) {
        return NULL;
    }
    sys->studentsArr = createStudentsArr(students);
    sys->hackersArr = updateHackerParams(sys->studentsArr, hackers);
    sys->coursesArr = createCourseArr(courses);
    return sys;
}

//ROY - takes courses file and creates a single course
Course createCourse(FILE* courses) {
    Course newCourse = (Course) malloc(sizeof(*newCourse));
    if (!newCourse) {
        return NULL;
    }
    if (fscanf(courses, " %d %d", &newCourse->courseNumber, &newCourse->size) < NUM_OF_COURSE_PARAMS) {
        free(newCourse);
        return NULL;
    }
    if ((newCourse->size) < 0) {
        free(newCourse);
        return NULL;
    }
    FriendshipFunction* friendshipFuncArr=(FriendshipFunction*)malloc(sizeof(FriendshipFunction)); //note - neutral function array
    if (friendshipFuncArr==NULL) {
        free(newCourse);
        return NULL;
    }
    friendshipFuncArr[0]=NULL;
    newCourse->queue = IsraeliQueueCreate(friendshipFuncArr, compareStudents, DEFAULT_THRESHOLD, DEFAULT_THRESHOLD);
    free(friendshipFuncArr);
    return newCourse;
}

//ROY - takes courses file and creates array of courses
Course* createCourseArr(FILE* courses) {
    int numOfCourses = 1;
    Course* coursesArr = (Course*) malloc(numOfCourses * sizeof(Course));
    if (!coursesArr) {
        return NULL;
    }
    coursesArr[0] = createCourse(courses);
    while (coursesArr[numOfCourses-1]) {
        Course* tempArr = (Course*) malloc(numOfCourses * sizeof(Course));
        if (!tempArr) {
            destroyCoursesArr(coursesArr); //TODO -make sure doesn't hurt program b/c it kills courses, not just array
            return NULL;
        }
        copyCourseArr(tempArr, coursesArr, numOfCourses);
        free(coursesArr);
        coursesArr = (Course*) malloc((++numOfCourses) * sizeof(Course));
        if (!coursesArr) {
            destroyCoursesArr(tempArr); //TODO -make sure doesn't hurt program b/c it kills the courses, not just array
            return NULL;
        }
        copyCourseArr(coursesArr, tempArr, numOfCourses-1);
        coursesArr[numOfCourses-1] = createCourse(courses);
        free(tempArr); //TODO - don't use the destroy function b/c it destroys the courses structs which are not cloned
    }
    return coursesArr;
}

//ROY - copies copyArr into pasteArr
static void copyCourseArr(Course* pasteArr, Course* copyArr, int size) {
    for (int i = 0; i < size; i++) {
        pasteArr[i] = copyArr[i];
    }
}

//ROY - also builds array of hackers;
Student* updateHackerParams(Student* studentArr, FILE* hackers) {
    int numOfHackers = 0;
    int hackerID = 0;
    int* hackerIDsArr = NULL;
    while (fscanf(hackers, " %d ", &hackerID) == 1) {
        hackerIDsArr = reallocateIntArr(hackerID, hackerIDsArr, ++numOfHackers);
        Student studentToEdit = searchStudent(studentArr, hackerID); //needs NULL-terminated studentArr
        studentToEdit->desiredCourses = updateArrOfInts(hackers); //getline func included
        studentToEdit->friendIDs = updateArrOfInts(hackers);
        studentToEdit->rivalIDs = updateArrOfInts(hackers);
        //printHackerArrays(studentToEdit); --optional debug*** (import func)
    }
    Student* hackersArr = (Student*) malloc((numOfHackers+1) * sizeof(Student)); //builds hackerArr
    if (!hackersArr) {
        return NULL;
    }
    for(int i=0; i<numOfHackers; i++) {
        Student hacker = searchStudent(studentArr, hackerIDsArr[i]);
        hackersArr[i] = hacker;
    }
    hackersArr[numOfHackers] = NULL;
    free(hackerIDsArr);
    return hackersArr;
}

//ROY - takes courses file and creates array of hackers. Frees old array which is given
static int* reallocateIntArr(int newInteger, int* oldArr, int numOfInts) {
    int *newArr = (int*) malloc((numOfInts+1) * sizeof(int)); //space for NULL at end of array
    if (!newArr) {
        if (oldArr != NULL) {
            free(oldArr);
        }
        return NULL;
    }
    newArr[numOfInts-1] = newInteger;
    newArr[numOfInts] = NON_ID;
    copyIntArr(newArr, oldArr, numOfInts-1);
    if (oldArr != NULL) {
        free(oldArr);
    }
    return newArr;
}

//ROY - copies copyArr into pasteArr
static void copyIntArr(int* pasteArr, int* copyArr, int size) {
    for (int i = 0; i < size; i++) {
        pasteArr[i] = copyArr[i];
    }
}

//ROY - finds pointer to student who matches ID. studentArr needs to be NULL terminated
static Student searchStudent(Student* studentArr, int hackerID) {
    if (!studentArr) {
        return NULL;
    }
    int i = 0;
    while (studentArr[i]) {
        if (studentArr[i]->studentID == hackerID) {
            return studentArr[i];
        }
        i++;
    }
    return NULL;
}

//scans row and picks up integers until it spots \n
int* updateArrOfInts(FILE* file) {
    int* paramArr = NULL;
    int sizeOfArr = 0;
    char transition;
    int num;
    while (fscanf(file, "%c", &transition) == 1) {  //try fgetc
        if (transition == MOVE_ROW_DOWN) { //take care of situation with empty row
            break;
        }
        else if (transition == SPACE_BETWEEN_INTS) {
            continue;
        }
        if (fscanf(file, " %d", &num) != 1) { //take in integer and the char after it - ' ' or '\n' //used to be fscanf(file, " %d%c", &num, &transition) != 2
            return NULL;
        }
        sizeOfArr++;
        paramArr = reallocateIntArr(buildInteger(transition, num), paramArr , sizeOfArr);
    }
    return paramArr;
}

//ROY - adds digit char as first digit of new integer
int buildInteger(char c, int num) {
    int multiply = 1;
    while (multiply < num) {
        multiply *= ADD_DECIMAL_PLACE;
    }
    if (c == NEGATIVE_SIGN) {
        return (num * NEGATIVE);
    }
    int firstDigit = (c - ZERO_CHAR);
    return (firstDigit*(multiply) + num);
}


//ROY - friendship function that returns absolute value of the difference in name ascii values
int friendFuncNameDistance (void* a, void* b) {
    if (!a || !b) {
        return NON_ID; //func returns sum of positive #s so negative returned value is valid indicator of fault
    }
    Student student1 = (Student) a;
    Student student2 = (Student) b;
    int nameDifference = (calculateASCIIDifference(student1->name, student2->name));
    int surnameDifference = (calculateASCIIDifference(student1->surname, student2->surname));
    int difference = nameDifference+surnameDifference;
    if (difference < 0) {
        difference *= NEGATIVE;
    }
    return difference;
}

//ROY - takes two string and returns sum of differences in ascii values
int calculateASCIIDifference(char* str1, char* str2) {
    int sum=0;
    if (strlen(str1) > strlen(str2)) {
        int i=0;
        while (str2[i] != END_STR) {
            sum = (str1[i] > str2[i]) ? (sum + (str1[i]-str2[i])) : (sum + (str2[i]-str1[i]));
            i++;
        }
        while (str1[i] != END_STR) {
            sum += str1[i];
            i++;
        }
    }
    else {
        int j=0;
        while (str1[j] != END_STR) {
            sum = (str1[j] > str2[j]) ? (sum + (str1[j]-str2[j])) : (sum + (str2[j]-str1[j]));
            j++;
        }
        while (str2[j] != END_STR) {
            sum += str2[j];
            j++;
        }
    }
    return sum;
}

//ROY - changes and then checks queues to courses in the system and prints out result message into file "out"
void hackEnrollment(EnrollmentSystem sys, FILE* out) {
    IsraeliQueueError functionsAdded = modifyIsraeliQueuesInSystem(sys); //note - check if returns error
    IsraeliQueueError hackersEnqueued = enqueueHackers(sys); //note - check if returns error
    assert(functionsAdded==ISRAELIQUEUE_SUCCESS);
    assert(hackersEnqueued==ISRAELIQUEUE_SUCCESS);
    Student sadHacker = checkPlacementOfHackers(sys); //hacker who did not get demands
    if (sadHacker == NULL) { //success
        printQueuesIntoFile(sys, out);
    }
    else {
        fprintf(out, "Cannot satisfy constraints for %d", (sadHacker->studentID)); //TODO - add \n at end of line??
        //note-  file mode needs to be "w"
    }
}

//ROY - adds friendship functions to the queues in the system and returns success/error
IsraeliQueueError modifyIsraeliQueuesInSystem(EnrollmentSystem sys) {
    int k=0;
    Course currentCourse = sys->coursesArr[k];
    IsraeliQueueError functionStatus[NUM_OF_NEW_FUNCTIONS];
    while (currentCourse)  { //note - need NULL-terminated coursesArr
        int i=0;
        functionStatus[i++] = IsraeliQueueAddFriendshipMeasure(currentCourse->queue, &friendFuncFile);
        functionStatus[i++] = IsraeliQueueAddFriendshipMeasure(currentCourse->queue, &friendFuncNameDistance);
        functionStatus[i] = IsraeliQueueAddFriendshipMeasure(currentCourse->queue, &friendFuncIDs);
        for (int j=0; j < NUM_OF_NEW_FUNCTIONS; j++) {
            if (functionStatus[j] != ISRAELIQUEUE_SUCCESS) {
                return functionStatus[j];
            }
        }
        k++;
        currentCourse=sys->coursesArr[k];
    }
    return ISRAELIQUEUE_SUCCESS;
}

//ROY - enqueues all hackers (in the order in hackers file) into desired courses' queues and returns success/error
IsraeliQueueError enqueueHackers(EnrollmentSystem sys) {
    int i=0; //hacker index
    Student currentHacker = sys->hackersArr[i];
    while (currentHacker) {
        int j=0; //desired course index
        int* courseNumArr = currentHacker->desiredCourses;
        Course currentCourse = searchCourse(sys->coursesArr, courseNumArr[j]);
        while (currentCourse) {
            IsraeliQueueError hackerEnqueueStatus = IsraeliQueueEnqueue(currentCourse->queue, currentHacker);
            if (hackerEnqueueStatus != ISRAELIQUEUE_SUCCESS) {
                return hackerEnqueueStatus;
            }
            j++;
            currentCourse = searchCourse(sys->coursesArr, courseNumArr[j]);
        }
        i++;
        currentHacker = sys->hackersArr[i];
    }
    return ISRAELIQUEUE_SUCCESS;
}

//ROY - checks if hackers got courses. Returns 1st unsatisfied hacker's pointer or NULL if all hackers satisfied
Student checkPlacementOfHackers(EnrollmentSystem sys) {
    int i=0; //hacker index
    Student currentHacker = sys->hackersArr[i];
    while (currentHacker) {
        int j=0; //desired course index
        int successfulRegistrations = 0;
        int* courseNumArr = currentHacker->desiredCourses;
        //TODO - new shit thurs night:
        if (courseNumArr == NULL) { //hacker didn't ask for courses -> happy
            i++;
            currentHacker = sys->hackersArr[i];
            break;
        }
        Course currentCourse = searchCourse(sys->coursesArr, courseNumArr[j]);
        while (currentCourse) {
            IsraeliQueue clonedQueue = IsraeliQueueClone(currentCourse->queue);
            int place = getPlaceInQueue(clonedQueue, currentHacker);
            IsraeliQueueDestroy(clonedQueue);
            assert(place > 0);
            if (place < currentCourse->size) {
                successfulRegistrations++;
            }
            j++;
            currentCourse = searchCourse(sys->coursesArr, courseNumArr[j]);
        }
        //TODO - new shit thurs night:
        if (j == 1) { //check if hacker asked for only one course
            if (successfulRegistrations < 1) { //unhappy hacker
                return currentHacker;
            }
        }
        else if (successfulRegistrations < MIN_SUCCESSFUL_REGISTRATIONS) {
            return currentHacker;
        }
        i++;
        currentHacker = sys->hackersArr[i];
    }
    return currentHacker;
}

//ROY - returns the place of the node in the queue. Assumes hacker is in queue
static int getPlaceInQueue(IsraeliQueue q, Student s) {
    if (!q) {
        return 0;
    }
    int place=1;
    while ((Student)IsraeliQueueDequeue(q) != s) { //q->compare("dequeued item", hacker)
        place++;
    }
    return place;
}

//ROY - prints new queues into out file (in the order in courses file)
void printQueuesIntoFile(EnrollmentSystem sys, FILE* out) { //TODO - printed the first and second hackers twice in queue
    int i=0; //course index
    Course currentCourse = sys->coursesArr[i];
    while (currentCourse) {
        IsraeliQueue clonedQueue = IsraeliQueueClone(currentCourse->queue);
        Student nextInLine = IsraeliQueueDequeue(clonedQueue);
        if (nextInLine==NULL){
            IsraeliQueueDestroy(clonedQueue);
            i++;
            currentCourse = sys->coursesArr[i];
            continue;
        }
        fprintf(out, "%d", currentCourse->courseNumber);
        while (nextInLine) {
            /* --note: print zeros to fill digit places:
            //print # digits of IDs
            if (nextInLine->studentID < ENOUGH_DIGITS) {
                fprintf(out, " ");
                for (int j=1; j*(nextInLine->studentID) < ENOUGH_DIGITS; j*=ADD_DECIMAL_PLACE) {
                    fprintf(out, "%d", 0);
                }
                fprintf(out, "%d", nextInLine->studentID);
            } //end treatment of small-valued IDs
            */
            //else {
            fprintf(out, " %d", nextInLine->studentID);
            //}
            nextInLine = IsraeliQueueDequeue(clonedQueue);
        }
        IsraeliQueueDestroy(clonedQueue);
        fprintf(out, "\n");
        i++;
        currentCourse = sys->coursesArr[i];
    }
}

static char* updateString(char* str, int *size)
{
    char* newStr = (char*)malloc(*size+STRING_LEN*sizeof(char));
    *size = *size+STRING_LEN;
    if(newStr==NULL){
        return NULL;
    }
    strcpy(newStr,str);
    free(str);
    return newStr;
}

static char* stringMaker(FILE* file, char stopChar)
{
    char* string = (char*)malloc(STRING_LEN*sizeof(char));
    if(string == NULL){
        return NULL;
    }
    char c;
    fscanf(file,"%c",&c);
    int i = 0;
    int size = STRING_LEN;
    while(c!=stopChar)
    {
        string[i++]=c;
        fscanf(file,"%c",&c);
        if(i==size-1){
            string[i]='\0';
            string = updateString(string, &size);
        }
    }
    string[i] = '\0';
    return string;
}

static Student createStudent(FILE* students) {
    Student s = (Student) malloc(sizeof(*s));
    if ((fscanf(students, "%d %d %lf ", &(s->studentID), &(s->totalCredits), &(s->gpa))) != NUM_OF_STRING_PARAM) {
        free(s);
        return NULL;
    }
    s->name = stringMaker(students, ' ');
    s->surname = stringMaker(students,' ');
    s->city = stringMaker(students,' ');
    s->department = stringMaker(students,'\n');
    s->friendIDs = NULL; //TODO -make sure okay
    s->rivalIDs = NULL;
    s->desiredCourses = NULL;
    return s;
}

static void copyStudentArr(Student* s1, Student* s2,int size)
{
    for(int i=0;i<size;i++){
        s1[i]=s2[i];
    }
}

static Student* updateStudentArr(Student* studentArr, int* size)
{
    Student* newStudentArr = (Student*) malloc(*size+STUDENTS_SIZE*sizeof(Student));
    if(newStudentArr == NULL){
        return NULL;
    }
    copyStudentArr(newStudentArr,studentArr,*size);
    free(studentArr);
    return newStudentArr;
}

static Student* createStudentsArr(FILE* students) {
    Student* studentArr = (Student*) malloc(STUDENTS_SIZE*sizeof(Student));
    if (!studentArr) {
        return NULL;
    }
    int i = 0;
    int size = STUDENTS_SIZE;
    Student s = createStudent(students);
    while(s != NULL){
        studentArr[i] = s;
        if(i==size-1) {
            studentArr = updateStudentArr(studentArr, &size);
        }
        s = createStudent(students);
        i++;
    }
    studentArr[i]=NULL; //just adding NULL at the end
    return studentArr;
}

static Course searchCourse(Course* courseArr, int course_num)
{
    for(int i=0;courseArr[i]!=NULL;i++){
        if (courseArr[i]->courseNumber==course_num) {
            return courseArr[i];
        }
    }
    return NULL;
}

int compareStudents(void* a1,void* a2)
{
    Student s1 = a1;
    Student s2 = a2;
    if(s1->studentID != s2->studentID || s1->totalCredits != s2->totalCredits ||
       s1->gpa != s2->gpa || s1->name != s2->name || s1->surname != s2->surname ||
       s1->department != s2->department || s1->city != s2->city)
        return 0;
    return 1;
}

char* stringToLower(char* string)
{
    for(int i=0;(string[i])!=0;i++){ //does it mean that string[i] != NULL?
        if(string[i]>='A' && string[i] <='Z')
            string[i]=(string[i]+TO_LOWER_DIFF);
    }
    return string;
}

EnrollmentSystem lowerCase(EnrollmentSystem sys)
{
    for(int i =0;sys->studentsArr[i]>0;i++){
        sys->studentsArr[i]->name=stringToLower(sys->studentsArr[i]->name);
        sys->studentsArr[i]->surname= stringToLower(sys->studentsArr[i]->surname);
    }
    return sys;
}

EnrollmentSystem readEnrollment(EnrollmentSystem sys, FILE* queues)
{
    int course_num;
    while(fscanf(queues,"%d ",&course_num)!=EOF)
    {
        Course sys_course = searchCourse(sys->coursesArr,course_num); //return relevant course from the array
        if(sys_course==NULL) {
            return NULL;
        } //look at this!!
        FriendshipFunction* friendshipFuncArr=(FriendshipFunction*)malloc(sizeof(FriendshipFunction));
        if(friendshipFuncArr==NULL){
            return NULL;
        } //look at this!!!!
        friendshipFuncArr[0]=NULL;
        IsraeliQueue courseQueue = IsraeliQueueCreate(friendshipFuncArr,compareStudents,
                                                      STUDENTS_FRIENDS_THRESHOLD,STUDENTS_RIVALS_THRESHOLD);
        free(friendshipFuncArr);
        int* courseStudentsIDs = updateArrOfInts(queues);
        if (courseStudentsIDs==NULL) {
            continue;
        }
        for(int i =0;courseStudentsIDs[i]>0;i++) {
            Student s = searchStudent(sys->studentsArr, courseStudentsIDs[i]);
            IsraeliQueueError error = IsraeliQueueEnqueue(courseQueue, s);
            if (error != ISRAELIQUEUE_SUCCESS) {
                free(courseStudentsIDs);
                return NULL;
            }
            sys_course->queue=courseQueue; //queue of course
        }
        free(courseStudentsIDs);
    }
    return sys;
}

int friendFuncFile(void* h1,void* s1)
{
    Student hacker = (Student) h1;
    Student stud = (Student) s1;
    int studentId = stud->studentID;
    if (hacker->friendIDs != NULL) {
        for(int i=0; hacker->friendIDs[i]>0 ;i++)
        {
            if(hacker->friendIDs[i]==studentId){
                return STUDENTS_FRIENDS_RES;
            }
        }
    }
    if (hacker->rivalIDs != NULL) {
        for(int i=0;hacker->rivalIDs[i]>0;i++)
        {
            if(hacker->rivalIDs[i]==studentId){
                return STUDENTS_RIVALS_RES;
            } //-20
        }
    }
    return STUDENTS_STRANGERS_RES;
}

int friendFuncIDs (void* a1, void* a2)
{
    Student s1 = (Student) a1;
    Student s2 = (Student) a2;
    int id1 = s1->studentID;
    int id2 = s2->studentID;
    return (id1>id2)? id1-id2:id2-id1;
}

void EnrollmentSysDestroy(EnrollmentSystem sys)
{
    if (sys==NULL) {
        return;
    }
    destroyCoursesArr(sys->coursesArr);
    destroyStudentsArr(sys->studentsArr);
    //destroyHackers:
    free(sys->hackersArr);
    //destroySys:
    free(sys);
}


void destroyCoursesArr(Course* coursesArr) {
    int i=0;
    while(coursesArr[i]!=NULL) {
        IsraeliQueueDestroy(coursesArr[i]->queue);
        free(coursesArr[i]);
        i++;
    }
    //free(coursesArr[i]); //note - no need to free NULL object
    free(coursesArr); //TODO - not sure - do we need it?

}

void destroyStudentsArr(Student* studentArr) {
    for(int i=0;studentArr[i]!=NULL;i++){
        free(studentArr[i]->name);
        free(studentArr[i]->surname);
        free(studentArr[i]->city);
        free(studentArr[i]->department);
        if(studentArr[i]->desiredCourses!=NULL){
            free(studentArr[i]->desiredCourses);
        }
        if(studentArr[i]->rivalIDs!=NULL) {
            free(studentArr[i]->rivalIDs);
        }
        if(studentArr[i]->friendIDs!=NULL){
            free(studentArr[i]->friendIDs);
        }
        free(studentArr[i]);
    }
    free(studentArr);
}