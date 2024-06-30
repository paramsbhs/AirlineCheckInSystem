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



int main(int argc, char *argv[]){
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    economyQueue = createQueue(); //Initialize the economy Queue
    businessQueue = createQueue(); //Initialize the business Queue
    int size; //Initialize the size of the queue

    inputFile(argv[1], economyQueue, businessQueue, &size); //Read the input file and store the data in its respective queues

    pthread_t clerkThreads[CLERKS]; // Initialize the clerk threads
    pthread_t customerThreads[size]; // Initialize the customer threads

    //displayQueue(economyQueue); testing purposes
    //displayQueue(businessQueue); testing purposes

    pthread_attr_t clerkattr; //thread attributes
    pthread_attr_init(&clerkattr); //get the thread attributes
    int i, rc;
    for(i = 1; i <= CLERKS; i++){
        int* clerk_id = (int*)malloc(sizeof(int)); //allocate memory for the clerk id
        *clerk_id = i; //set the clerk id to i
        if((rc = pthread_create(&clerkThreads[i], &clerkattr, clerkThread, clerk_id))){ //Create the clerk threads, Sample Code (pthread_create.c)
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
            return EXIT_FAILURE;
        }
    }

    pthread_attr_t customerattr;
    pthread_attr_init(&customerattr);
    int j = 0;
    struct Node *current = economyQueue->front;
    while (current != NULL) {
        struct Customer *customer = &current->customerData;
        if ((rc = pthread_create(&customerThreads[j], &customerattr, customerThread, customer))) { // Create the customer threads
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
            return EXIT_FAILURE;
        }
        current = current->next;
        j++;
    }

    current = businessQueue->front;
    while (current != NULL) {
        struct Customer *customer = &current->customerData;
        if ((rc = pthread_create(&customerThreads[j], &customerattr, customerThread, customer))) { // Create the customer threads
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
            return EXIT_FAILURE;
        }
        current = current->next;
        j++;
    }

    for(int k = 0; k < size; k++){
        pthread_join(customerThreads[k], NULL); //Join the customer threads
    }
    for(int l = 0; l < CLERKS; l++){
        pthread_join(clerkThreads[l], NULL); //Join the clerk threads
    }


    pthread_mutex_destroy(&businessQueueMutex); //Destroy the business queue mutex
    pthread_mutex_destroy(&economyQueueMutex); //Destroy the economy queue mutex
    pthread_cond_destroy(&customerServed);
    pthread_cond_destroy(&clerkAvailable);

    free(economyQueue); //Free the economy queue
    free(businessQueue); //Free the business queue

    sleep(3);
    return 0;
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

/*
    The customerThread function simulates the customer
    arriving at the check-in counter, it extracts
    the information, sleeps for the arrival time,
    acquires the appropriate queue mutex, enqueues
    the customer into the respective queue, signals
    the clerk thread that a customer is available
    for processing, releases the queue mutex, and 
    returns NULL.
*/
void* customerThread(void* param){
    struct Customer* customer = (struct Customer*)param;

    // Simulate arrival time
    usleep(customer->arrival_time * 100000); // Convert to microseconds

    printf("A customer arrives: customer ID %2d. \n", customer->user_id);

    if (customer->class_type == 1) {
        pthread_mutex_lock(&businessQueueMutex);
        enqueue(businessQueue, *customer);
        businessSize++;
        printf("A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n", 1, businessSize);
        pthread_cond_signal(&clerkAvailable);
        pthread_mutex_unlock(&businessQueueMutex);
    } else {
        pthread_mutex_lock(&economyQueueMutex);
        enqueue(economyQueue, *customer);
        economySize++;
        printf("A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n", 0, economySize);
        pthread_cond_signal(&clerkAvailable);
        pthread_mutex_unlock(&economyQueueMutex);
    }

    while (TRUE) {
        pthread_mutex_lock(&economyQueueMutex);
        pthread_mutex_lock(&businessQueueMutex);
        
        if (businessQueue->front && businessQueue->front->customerData.user_id == customer->user_id) {
            pthread_mutex_unlock(&businessQueueMutex);
            pthread_mutex_unlock(&economyQueueMutex);
            break;
        } else if (economyQueue->front && economyQueue->front->customerData.user_id == customer->user_id) {
            pthread_mutex_unlock(&businessQueueMutex);
            pthread_mutex_unlock(&economyQueueMutex);
            break;
        }

        pthread_cond_wait(&clerkAvailable, &economyQueueMutex);
        pthread_cond_wait(&clerkAvailable, &businessQueueMutex);
        
        pthread_mutex_unlock(&businessQueueMutex);
        pthread_mutex_unlock(&economyQueueMutex);
    }

    free(customer);
    return NULL;
}

/*
    The clerkThread function simulates the clerk
    processing the customer, it acquires the queue
    mutex, dequeues the customer from the queue,
    sleeps for the service time, releases the queue
    mutex, and returns NULL.
*/
void* clerkThread(void* param) {
    int clerk_id = *(int*)param;
    free(param);

    while (TRUE) {
        struct Customer customer;
        int queue_id = -1;

        // Check if there are customers in the queues
        pthread_mutex_lock(&businessQueueMutex);
        pthread_mutex_lock(&economyQueueMutex);

        while (businessQueue->size == 0 && economyQueue->size == 0) {
            // Wait for a customer to arrive
            pthread_cond_wait(&clerkAvailable, &businessQueueMutex);
            pthread_cond_wait(&clerkAvailable, &economyQueueMutex);
        }

        // Serve business class customer first if available
        if (businessQueue->size > 0) {
            customer = dequeue(businessQueue);
            businessSize--;
            queue_id = 1;
            printf("Clerk ID %1d starts serving a business customer: customer ID %2d. \n", clerk_id, customer.user_id);
            printf("A customer leaves a queue: the queue ID 1, and length of the queue %2d. \n", businessSize);
        } else if (economyQueue->size > 0) {
            customer = dequeue(economyQueue);
            economySize--;
            queue_id = 0;
            printf("Clerk ID %1d starts serving an economy customer: customer ID %2d. \n", clerk_id, customer.user_id);
            printf("A customer leaves a queue: the queue ID 0, and length of the queue %2d. \n", economySize);
        }

        pthread_mutex_unlock(&businessQueueMutex);
        pthread_mutex_unlock(&economyQueueMutex);

        // Simulate service time
        usleep(customer.service_time * 100000); // Convert to microseconds

        printf("Clerk ID %1d finishes serving customer ID %2d. \n", clerk_id, customer.user_id);

        pthread_mutex_lock(&waitingTimeMutex);
        totalWaitingTime += (customer.service_time - customer.arrival_time);
        pthread_mutex_unlock(&waitingTimeMutex);

        // Check if all customers are served and exit if true
        pthread_mutex_lock(&businessQueueMutex);
        pthread_mutex_lock(&economyQueueMutex);
        if (businessQueue->size == 0 && economyQueue->size == 0) {
            pthread_mutex_unlock(&businessQueueMutex);
            pthread_mutex_unlock(&economyQueueMutex);
            break;
        }
        pthread_mutex_unlock(&businessQueueMutex);
        pthread_mutex_unlock(&economyQueueMutex);
    }

    return NULL;
}