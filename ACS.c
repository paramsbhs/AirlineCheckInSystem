#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "linked_list.h"

void inputFile(const char *filename, struct Queue *economyQueue, struct Queue *businessQueue, int *size);
void* customerThread(void* param);
void* clerkThread(void* param); 

struct Queue *economyQueue;
struct Queue *businessQueue;

#define QUEUE 2
#define CLERKS 5
#define TRUE 1
#define FALSE 0
#define IDLE 0

pthread_mutex_t businessQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t economyQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clerkAvailable = PTHREAD_COND_INITIALIZER;
pthread_cond_t customerServed = PTHREAD_COND_INITIALIZER;
pthread_mutex_t waitingTimeMutex = PTHREAD_MUTEX_INITIALIZER;

int economySize = 0;
int businessSize = 0;
float totalWaitingTime = 0;
float businessWaitingTime = 0;
float economyWaitingTime = 0;
int totalCustomers = 0;
int businessCustomers = 0;
int economyCustomers = 0;

struct timeval startTime;

float getCurrentTime() {
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);
    return (currentTime.tv_sec - startTime.tv_sec) + (currentTime.tv_usec - startTime.tv_usec) / 1000000.0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    gettimeofday(&startTime, NULL);

    economyQueue = createQueue();
    businessQueue = createQueue();
    int size;

    inputFile(argv[1], economyQueue, businessQueue, &size);

    pthread_t clerkThreads[CLERKS];
    pthread_t customerThreads[size];

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    int i, rc;
    for(i = 0; i < CLERKS; i++) {
        int* clerk_id = malloc(sizeof(int));
        *clerk_id = i+1;
        if((rc = pthread_create(&clerkThreads[i], &attr, clerkThread, clerk_id))) {
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
            return EXIT_FAILURE;
        }
    }

    struct Node *current = economyQueue->front;
    int j = 0;
    while (current != NULL) {
        struct Customer *customer = malloc(sizeof(struct Customer));
        *customer = current->customerData;
        if ((rc = pthread_create(&customerThreads[j], &attr, customerThread, customer))) {
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
            return EXIT_FAILURE;
        }
        current = current->next;
        j++;
    }

    current = businessQueue->front;
    while (current != NULL) {
        struct Customer *customer = malloc(sizeof(struct Customer));
        *customer = current->customerData;
        if ((rc = pthread_create(&customerThreads[j], &attr, customerThread, customer))) {
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
            return EXIT_FAILURE;
        }
        current = current->next;
        j++;
    }

    for(int k = 0; k < size; k++) {
        pthread_join(customerThreads[k], NULL);
    }

    for(int l = 0; l < CLERKS; l++) {
        pthread_cancel(clerkThreads[l]);
        pthread_join(clerkThreads[l], NULL);
    }

    printf("The average waiting time for all customers in the system is: %.2f seconds. \n", totalWaitingTime / totalCustomers);
    printf("The average waiting time for all business-class customers is: %.2f seconds. \n", businessWaitingTime / businessCustomers);
    printf("The average waiting time for all economy-class customers is: %.2f seconds. \n", economyWaitingTime / economyCustomers);

    pthread_mutex_destroy(&businessQueueMutex);
    pthread_mutex_destroy(&economyQueueMutex);
    pthread_cond_destroy(&customerServed);
    pthread_cond_destroy(&clerkAvailable);
    pthread_mutex_destroy(&waitingTimeMutex);

    free(economyQueue);
    free(businessQueue);

    return 0;
}

void* customerThread(void* param) {
    struct Customer* customer = (struct Customer*)param;

    usleep(customer->arrival_time * 100000);

    float arrivalTime = getCurrentTime();
    printf("A customer arrives: customer ID %2d. \n", customer->user_id);

    pthread_mutex_t* queueMutex;
    struct Queue* queue;
    int* queueSize;
    int queueId;

    if (customer->class_type == 1) {
        queueMutex = &businessQueueMutex;
        queue = businessQueue;
        queueSize = &businessSize;
        queueId = 1;
    } else {
        queueMutex = &economyQueueMutex;
        queue = economyQueue;
        queueSize = &economySize;
        queueId = 0;
    }

    pthread_mutex_lock(queueMutex);
    enqueue(queue, *customer);
    (*queueSize)++;
    printf("A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n", queueId, *queueSize);
    pthread_cond_signal(&clerkAvailable);
    pthread_mutex_unlock(queueMutex);

    pthread_mutex_lock(queueMutex);
    while (queue->front->customerData.user_id != customer->user_id) {
        pthread_cond_wait(&clerkAvailable, queueMutex);
    }
    pthread_mutex_unlock(queueMutex);

    float serviceStartTime = getCurrentTime();
    float waitingTime = serviceStartTime - arrivalTime;

    pthread_mutex_lock(&waitingTimeMutex);
    totalWaitingTime += waitingTime;
    totalCustomers++;
    if (customer->class_type == 1) {
        businessWaitingTime += waitingTime;
        businessCustomers++;
    } else {
        economyWaitingTime += waitingTime;
        economyCustomers++;
    }
    pthread_mutex_unlock(&waitingTimeMutex);

    usleep(customer->service_time * 100000);

    float serviceEndTime = getCurrentTime();
    printf("A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d. \n",
           serviceEndTime, customer->user_id, 101);

    pthread_cond_signal(&customerServed);

    free(customer);
    return NULL;
}

void* clerkThread(void* param) {
    int* clerk_id = (int*)param;

    while (1) {
        struct Customer* customer = NULL;
        pthread_mutex_t* queueMutex;
        struct Queue* queue;
        int* queueSize;

        pthread_mutex_lock(&businessQueueMutex);
        if (businessQueue->front != NULL) {
            customer = &(businessQueue->front->customerData);
            queue = businessQueue;
            queueMutex = &businessQueueMutex;
            queueSize = &businessSize;
        } else {
            pthread_mutex_unlock(&businessQueueMutex);
            pthread_mutex_lock(&economyQueueMutex);
            if (economyQueue->front != NULL) {
                customer = &(economyQueue->front->customerData);
                queue = economyQueue;
                queueMutex = &economyQueueMutex;
                queueSize = &economySize;
            } else {
                pthread_mutex_unlock(&economyQueueMutex);
                pthread_mutex_lock(&businessQueueMutex);
                pthread_cond_wait(&clerkAvailable, &businessQueueMutex);
                pthread_mutex_unlock(&businessQueueMutex);
                continue;
            }
        }

        customer->clerk_id = 1;
        dequeue(queue);
        (*queueSize)--;

        float startTime = getCurrentTime();
        printf("A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d. \n",
               startTime, customer->user_id, *clerk_id);

        pthread_cond_signal(&clerkAvailable);
        pthread_mutex_unlock(queueMutex);

        pthread_mutex_lock(queueMutex);
        pthread_cond_wait(&customerServed, queueMutex);
        pthread_mutex_unlock(queueMutex);
    }

    free(clerk_id);
    return NULL;
}
/*  
    inputFile function reads the input 
    file and stores the data in the queue
*/
void inputFile(const char *filename, struct Queue *economyQueue, struct Queue *businessQueue, int *size) {
    FILE *file = fopen(filename, "r"); //Open the file in read mode
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    fscanf(file, "%d\n", size); //scan how many customers are in the file
    for (int i = 0; i < *size; i++) { //store the data from the file in the queue
        struct Customer customer;
        fscanf(file, "%d:%d,%d,%d\n", &customer.user_id, &customer.class_type, &customer.arrival_time, &customer.service_time); //scan the data from the file
        if(customer.class_type == 1){ //if the customer is a business class customer, add it to the business queue
            enqueue(businessQueue, customer);
        }else{
            enqueue(economyQueue, customer); //if the customer is an economy class customer, add it to the economy queue
        }
    }
    fclose(file);
}
