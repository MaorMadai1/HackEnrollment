#include "IsraeliQueue.h"
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#define IQC IsraeliQueueCreate
#define CHANGE_SIZE true
#define DONT_CHANGE_SIZE false
#define NEGATIVE (-1)
#define INVALID_SIZE 0

#define FRIEND 1
#define RIVAL (-1)
#define STRANGER 0
#define ERROR 2

//Maor
typedef struct node {
    void* item;
    int friend_counter;
    int rival_counter;
    struct node* next;
    struct node* prev;
    bool checked_flag;
} *Node;

//Maor
struct IsraeliQueue_t {
    Node head; //point to the beginning of the queue (FIFO - head comes first)
    Node tail;
    FriendshipFunction* friendshipFunctionArr; //remember that FriendshipFunction is a ptr to func!
    ComparisonFunction compare; //ptr to a given compare func.
    int friendshipThreshold;
    int rivalryThreshold;
};

//ROY
static int calculateSizeOfFunctionArray(FriendshipFunction* friendshipFunctions) {
    assert(friendshipFunctions!=NULL);
    int counter = 0;
    while (friendshipFunctions[counter]) {
        counter++;
    }
    return counter;
}

//Maor
static int areFriends(Node node_a,Node node_b,FriendshipFunction* friendshipFunction_arr,
                      int friendship_th, int rivalry_th)
{
    if(node_a == NULL || node_b == NULL){
        return ERROR;
    }
    //int len = sizeof(friendshipFunction_arr)/sizeof(FriendshipFunction);
    int len = calculateSizeOfFunctionArray(friendshipFunction_arr);
    int sum=0;
    for(int i=0;i<len;i++)
    {
        if(friendshipFunction_arr[i]==NULL) {
            return ERROR;
        }
        int result = friendshipFunction_arr[i](node_a->item ,node_b->item); //check if we need to declare res before the loop
        if (result>friendship_th){ //does it > or >= ???? - need to check!
            return FRIEND;
        }
        sum+=result;
    }
    return (sum < rivalry_th*len) ? RIVAL : STRANGER; //rival-if the avg is less than the rival_quota
}

//ROY - counts number of queues to merge
static int findNumOfQueues(IsraeliQueue* qarr) {
    int i = 0;
    while (qarr[i]){
        i++;
    }
    return i;
}

//ROY - merges friend function arrays of merging queues - malloc but no free! -free it in merge function
static FriendshipFunction* mergeFriendshipFunctions (IsraeliQueue* qarr) {
    int totalSize = 0;
    int i = 0;
    while (qarr[i]) {
        totalSize += calculateSizeOfFunctionArray(qarr[i]->friendshipFunctionArr); //subtract NULL at end of each array
        i++;
    }
    totalSize += 1; //add NULL at the end of merged array
    FriendshipFunction* merged_arr = (FriendshipFunction*) malloc(totalSize * sizeof(FriendshipFunction));
    i = 0;
    int j, k = 0;
    while (qarr[i]) {
        j = 0;
        while ((qarr[i]->friendshipFunctionArr)[j]) {
            merged_arr[k] = ((qarr[i])->friendshipFunctionArr)[j];
            j++;
            k++;
        }
        i++;
    }
    merged_arr[totalSize - 1] = NULL; //note to self - this is last item in array
    return merged_arr;
}

//ROY - enqueues nodes in a cycle from qarr into merged q
static IsraeliQueueError enqueueCycle (IsraeliQueue* qarr, IsraeliQueue mergedQ, int num_of_queues) {
    Node* nodeTrackerArray = (Node*) malloc(num_of_queues * sizeof(Node));
    if (!nodeTrackerArray) {
        return ISRAELIQUEUE_ALLOC_FAILED;
    }
    for (int i=0; i < num_of_queues; i++) {
        nodeTrackerArray[i] = ((qarr[i])->head);
    }
    bool nodesLeft = true;
    while (nodesLeft) {
        nodesLeft = false;
        for (int i = 0; i < num_of_queues; i++) {
            if (nodeTrackerArray[i]) {
                nodesLeft = true; //still nodes left
                IsraeliQueueError status = IsraeliQueueEnqueue(mergedQ, nodeTrackerArray[i]->item);   // equivalent to (mergedQ, qarr[i]->head->item);
                if (status != ISRAELIQUEUE_SUCCESS) {
                    free(nodeTrackerArray);
                    return ISRAELI_QUEUE_ERROR;
                }
                nodeTrackerArray[i] = (nodeTrackerArray[i]->next);
            }
        }
    }
    free(nodeTrackerArray);
    return ISRAELIQUEUE_SUCCESS;
}

//ROY
static int averageArithmeticFriendThreshold (IsraeliQueue* qarr, int num_of_queues) {
    int sum = 0;
    for (int i=0; i<num_of_queues; i++){
        sum += (qarr[i]->friendshipThreshold);
    }
    int average = (sum/num_of_queues);
    if (sum % num_of_queues) { //round up if positive (negative is rounded already)
        if (average >= 0) {
            average++;
        }
    }
    return average;
}

int power (int base, int exponent)
{
    int result = 1;
    for(int i=0;i<exponent;i++){
        result*=base;
    }
    return result;

}

//ROY
static int averageGeometricRivalThreshold (IsraeliQueue* qarr, int num_of_queues) {
    int product = 1;
    for (int i = 0; i < num_of_queues; i++) {
        product *= (qarr[i]->rivalryThreshold);
    }
    if (product < 0) {
        product *= (NEGATIVE);
    }
    int average_geometric = 0, average_to_the_power = 0;
    while (average_to_the_power <= product) {
        average_to_the_power = power(++average_geometric,num_of_queues);
    }
    return average_geometric;
}

//Maor
static void detachNode(Node b, IsraeliQueue queue) { //changed in 25/4
    Node a = b->prev;
    Node c = b->next;
    if(b==queue->tail){
        a->next = c;
        queue->tail=a;
    }
    else if(b==queue->head){
        c->prev=a;
        queue->head=c;
    }
    else{
        a->next=c;
        c->prev=a;
    }
    b->next = NULL;
    b->prev = NULL;
}

//ROY- clones params from one node to another
static void copyNodeParams (Node pasteNode, Node copyNode) {
    pasteNode->friend_counter = copyNode->friend_counter;
    pasteNode->rival_counter = copyNode->rival_counter;
    pasteNode->checked_flag = copyNode->checked_flag;
}

//Maor - we did not free the node here!!
static Node createNode(void* newItem) {
    Node d = (Node) malloc(sizeof(*d));
    if(d==NULL) {
        return NULL;
    }//malloc did not succeed
    d->item=newItem;
    d->next=NULL;
    d->prev=NULL;
    d->friend_counter = 0;
    d->rival_counter = 0;
    d->checked_flag = false;
    return d;
}

//ROY - copy functions array with malloc, without free
FriendshipFunction* copyFriendFunctionArr(FriendshipFunction* friendshipFunctions,
                                          bool changeSize, FriendshipFunction newFunction) {
    int oldArraySize = calculateSizeOfFunctionArray(friendshipFunctions) + 1; //+1 to include NULL at the end
    if (changeSize) {
        FriendshipFunction* copied_arr = (FriendshipFunction*) malloc((oldArraySize+1)*sizeof(FriendshipFunction)); //+1 to include additional measure
        if (!copied_arr) {
            return NULL;
        }
        copied_arr[oldArraySize-1] = newFunction;
        copied_arr[oldArraySize] = NULL;
        for (int i=0; i < (oldArraySize-1); i++) {
            copied_arr[i] = friendshipFunctions[i];
        }
        return copied_arr;
    }
    else {
        FriendshipFunction* copied_arr = (FriendshipFunction*) malloc(oldArraySize*sizeof(FriendshipFunction));
        if (!copied_arr) {
            return NULL;
        }
        copied_arr[oldArraySize-1] = NULL;
        for (int i=0; i < (oldArraySize-1); i++) {
            copied_arr[i] = friendshipFunctions[i];
        }
        return copied_arr;
    }
}

//ROY -create Israeli queue with malloc, without free
IsraeliQueue IsraeliQueueCreate(FriendshipFunction* friendshipFuncArr, ComparisonFunction comparisonFunction,
                                int friendThreshold,int rivalThreshold) {
    if(friendshipFuncArr==NULL||comparisonFunction==NULL){
        return NULL;
    }
    IsraeliQueue ptr = (IsraeliQueue) malloc(sizeof(*ptr));
    if(!ptr) {
        return NULL;
    }
    FriendshipFunction* copiedFriendshipFunctions = copyFriendFunctionArr(friendshipFuncArr, DONT_CHANGE_SIZE, NULL);
    ptr->friendshipFunctionArr = copiedFriendshipFunctions;
    ptr->compare = comparisonFunction;
    ptr->friendshipThreshold = friendThreshold;
    ptr->rivalryThreshold = rivalThreshold;
    ptr->head = NULL; //-don't default head to null b/c then delete won't work
    ptr->tail = NULL;
    return ptr;
}

//ROY - frees all memory attached to given queue, including all nodes and friend func array
void IsraeliQueueDestroy(IsraeliQueue q) {
    if (q==NULL) {
        return;
    }
    while (q->head) {
        void* trash_item = IsraeliQueueDequeue(q);
        if(trash_item != NULL) {
            trash_item = NULL;
        }
    }
    free(q->friendshipFunctionArr); //free func array which is a copy of array given by user
    free(q);
}

//ROY - adds friendship function to array
IsraeliQueueError IsraeliQueueAddFriendshipMeasure(IsraeliQueue q, FriendshipFunction friendshipFunction) {
    if(friendshipFunction==NULL){
        return ISRAELIQUEUE_BAD_PARAM;
    }
    FriendshipFunction* temp_array = copyFriendFunctionArr(q->friendshipFunctionArr, CHANGE_SIZE, friendshipFunction);
    if (!(temp_array)) {
        return ISRAELIQUEUE_ALLOC_FAILED;
    }
    free(q->friendshipFunctionArr);   //free old array (which is also a copy of array given by user)
    q->friendshipFunctionArr = temp_array;
    return ISRAELIQUEUE_SUCCESS;
}

//ROY - changes friendship threshold
IsraeliQueueError IsraeliQueueUpdateFriendshipThreshold(IsraeliQueue q, int friendThr) {
    if (!q) {
        return ISRAELIQUEUE_BAD_PARAM;
    }
    q->friendshipThreshold = friendThr;
    return ISRAELIQUEUE_SUCCESS;
    //maybe include a function to check that the requested change indeed took place and return error if not
    //the check function would be called upon both update threshold functions
}

//ROY - changes rivalry threshold
IsraeliQueueError IsraeliQueueUpdateRivalryThreshold(IsraeliQueue q, int rivalThr) {
    if (!q) {
        return ISRAELIQUEUE_BAD_PARAM;
    }
    q->rivalryThreshold = rivalThr;
    return ISRAELIQUEUE_SUCCESS;
}

//ROY - counts nodes in queue
int IsraeliQueueSize(IsraeliQueue q) {
    if (!q) {
        return INVALID_SIZE; //set to 0, should be -1?
    }
    int counter = 0;
    Node current = (q->head);
    while (current) {
        counter++;
        current = (current->next);
    }
    return counter;
}

//ROY - checks if item is in queue
bool IsraeliQueueContains(IsraeliQueue q, void* item) {
    if ( (!q) || (!item) ) {
        return false;
    }
    Node current = (q->head);
    while (current) {
        if (q->compare(item, current->item)) { //using built-in comparison function
            return true;
        }
        current = (current->next);
    }
    return false;
}

//ROY - merges array according to project demands
IsraeliQueue IsraeliQueueMerge(IsraeliQueue* qarr,ComparisonFunction compareFunc) {
    if (!qarr) {
        return NULL;
    }
    int num_of_queues = findNumOfQueues(qarr);
    FriendshipFunction* mergedFunctionArray = mergeFriendshipFunctions(qarr);
    IsraeliQueue mergedQ = IsraeliQueueCreate(mergedFunctionArray, compareFunc,
                                              averageArithmeticFriendThreshold (qarr, num_of_queues),
                                              averageGeometricRivalThreshold (qarr, num_of_queues));

    IsraeliQueueError status = enqueueCycle (qarr, mergedQ, num_of_queues);
    if (status != ISRAELIQUEUE_SUCCESS) {
        return NULL;
    }
    free (mergedFunctionArray); //this array is copied in IQC, and it's not the original array given by user -> free it
    return mergedQ;
}

//Maor
static void linkNodes(Node a,Node b,IsraeliQueue queue)
{
    if(a==NULL){ //the queue is empty, a==null if queue.head is null
        queue->head=b;
        queue->tail=b;
    }
    else if(a==queue->tail) //only one person in the queue OR a is the last node
    {
        queue->tail=b;
        a->next=b;
        b->prev=a;
        b->next=NULL;
    }
    else
    {
        Node c = a->next;
        a->next=b;
        b->next=c;
        b->prev=a;
        c->prev=b;
    }
}

//ROY - clones entire chain of nodes
static void cloneNodes(IsraeliQueue q, IsraeliQueue newQ) {
    Node current = q->head;
    if (!(current)) {
        return;
    }

    Node newNodeA = createNode(current->item);
    copyNodeParams(newNodeA, current);
    newQ->head = newNodeA;
    newQ->tail = newNodeA;
    while (current->next) {
        Node newNodeB = createNode(current->next->item);
        copyNodeParams(newNodeB, current->next);
        linkNodes (newNodeA, newNodeB, newQ);
        current = current->next;
        newNodeA = newNodeB;
    }
    //return;
}

//ROY
IsraeliQueue IsraeliQueueClone(IsraeliQueue q) {
    IsraeliQueue newQ = IQC(q->friendshipFunctionArr, q->compare,
                            q->friendshipThreshold, q->rivalryThreshold);
    if (!newQ) {
        return NULL;
    }
    cloneNodes(q, newQ); //changes newQ with linkNodes function
    return newQ;
}

//Maor
static void findPlaceInQueue(Node newNode,IsraeliQueue queue) {
    if (queue->head == NULL) {
        linkNodes(queue->head, newNode, queue);
        return;
    }
    Node s = queue->head;
    Node friendNode = NULL;
    while (s != NULL) {
        int result = areFriends(newNode,s,queue->friendshipFunctionArr,
                                queue->friendshipThreshold,queue->rivalryThreshold);
        if(result == FRIEND && s->friend_counter < FRIEND_QUOTA && friendNode==NULL) {
            friendNode = s;
        }
        else if(result == RIVAL && s->rival_counter < RIVAL_QUOTA && friendNode!=NULL){
            friendNode = NULL;
            s->rival_counter = s->rival_counter+1;
        }
        s=s->next;
    }
    if(friendNode!=NULL) {
        linkNodes(friendNode, newNode, queue);
        friendNode->friend_counter=friendNode->friend_counter+1;
    }
    else { //friendNode == NULL meaning no friends so end of line
        linkNodes(queue->tail,newNode,queue);
    }
}

//Maor
IsraeliQueueError IsraeliQueueEnqueue(IsraeliQueue queue, void* item)
{
    if(item == NULL || queue == NULL) { //check params
        return ISRAELIQUEUE_BAD_PARAM;
    }
    Node newNode = createNode(item); //create
    if(newNode==NULL) {
        return ISRAELIQUEUE_ALLOC_FAILED;
    } //new node is not null
    findPlaceInQueue(newNode,queue); //newNode comes *after* toLink
    return ISRAELIQUEUE_SUCCESS;
}

//Maor
void* IsraeliQueueDequeue(IsraeliQueue queue)
{
    if (queue == NULL || queue->head == NULL) {
        return NULL;
    }
    else {
        Node toDelete = queue->head; //we checked it is not null before
        queue->head = queue->head->next;
        if (queue->head != NULL) {
            queue->head->prev = NULL;
        }
        else {
            queue->tail = NULL;
        }
        void* oldItem = toDelete->item;
        free(toDelete);//important warning! - how to treat that?
        return oldItem;
    }
}

//Maor - check it!!
IsraeliQueueError IsraeliQueueImprovePositions(IsraeliQueue queue)
{ //DO NOT FORGET TO GET IT OUT OF THE QUEUE BEFORE!
    if(queue==NULL) {
        return ISRAELIQUEUE_BAD_PARAM;
    }
    Node q, p = queue->tail;
    while(p!=NULL) //p == NULL means 0/1 node - either way returns the same
    {
        if(p->checked_flag){
            p=p->prev; //it is not null;
            continue;
        }
        q=p->prev; //
        detachNode(p,queue);
        findPlaceInQueue(p,queue);
        p->checked_flag=true; //this is WRONG! I NEED TO CHANGE IT!!
        p=q;
    }
    return ISRAELIQUEUE_SUCCESS;
}