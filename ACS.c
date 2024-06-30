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
    usleep(customer->arrival_time * 100000); //sleep for the arrival time

    printf("A customer arrives: customer ID %2d. \n", customer->user_id); //print the customer ID

    if (customer->class_type == 1) { //if the customer is in business
        pthread_mutex_lock(&businessQueueMutex); //lock the business queue mutex
        enqueue(businessQueue, *customer); //add the customer to the business queue
        businessSize++; //increment the size of the business queue
        printf("A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n", 1, businessSize); //print the queue ID and the length of the queue
        pthread_cond_signal(&clerkAvailable); //signal the clerk that a customer is available
        pthread_mutex_unlock(&businessQueueMutex); //unlock the business queue mutex
    } else { //if the customer is in economy
        pthread_mutex_lock(&economyQueueMutex); //lock the economy queue mutex
        enqueue(economyQueue, *customer); //add the customer to the economy queue
        economySize++; //increment the size of the economy queue
        printf("A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n", 0, economySize); //print the queue ID and the length of the queue
        pthread_cond_signal(&clerkAvailable); //signal the clerk that a customer is available
        pthread_mutex_unlock(&economyQueueMutex); //unlock the economy queue mutex
    }

    while (TRUE) {
        pthread_mutex_lock(&economyQueueMutex); //lock the economy queue mutex
        pthread_mutex_lock(&businessQueueMutex); //lock the business queue mutex
        
        if (businessQueue->front && businessQueue->front->customerData.user_id == customer->user_id) { //if the customer is in the business queue
            pthread_mutex_unlock(&businessQueueMutex); //unlock the business queue mutex
            pthread_mutex_unlock(&economyQueueMutex); //unlock the economy queue mutex
            break;
        } else if (economyQueue->front && economyQueue->front->customerData.user_id == customer->user_id) { //if the customer is in the economy queue
            pthread_mutex_unlock(&businessQueueMutex); //unlock the business queue mutex
            pthread_mutex_unlock(&economyQueueMutex); //unlock the economy queue mutex
            break;
        }

        pthread_cond_wait(&clerkAvailable, &economyQueueMutex);
        pthread_cond_wait(&clerkAvailable, &businessQueueMutex);
        
        pthread_mutex_unlock(&businessQueueMutex);
        pthread_mutex_unlock(&economyQueueMutex);
    }

    // Self-serving
    usleep(customer->service_time * 100000); // sleep for the service time

    // Print simulation time and update waiting time
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    float waiting_time = (current_time.tv_sec - customer->arrival_time) + (current_time.tv_usec - customer->arrival_time) / 1000000.0;
    totalWaitingTime += waiting_time;
    if (customer->class_type == 1) {
        businessWaitingTime += waiting_time;
    } else {
        economyWaitingTime += waiting_time;
    }

    // Print customer served by clerk
    printf("Customer ID %2d is served by clerk %d. \n", customer->user_id, clerk_id);

    // Signal clerk that service is finished
    pthread_cond_signal(&customerServed);

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
    int clerk_id = *((int*)param);
    free(param);

    while (TRUE) {
        pthread_mutex_lock(&businessQueueMutex);
        pthread_mutex_lock(&economyQueueMutex);

        // Check if there are customers waiting in the queues
        int businessQueueEmpty = isEmpty(businessQueue);
        int economyQueueEmpty = isEmpty(economyQueue);

        if (!businessQueueEmpty) {
            // Fetch the first customer from the business queue
            struct Customer customer = dequeue(businessQueue);
            businessSize--;

            // Signal the business queue that a customer is being served
            pthread_cond_signal(&customerServed);

            pthread_mutex_unlock(&businessQueueMutex);
            pthread_mutex_unlock(&economyQueueMutex);

            // Wait for the customer to finish service
            pthread_mutex_lock(&waitingTimeMutex);
            pthread_cond_wait(&customerServed, &waitingTimeMutex);
            pthread_mutex_unlock(&waitingTimeMutex);

            printf("Clerk %d finished serving customer ID %2d.\n", clerk_id, customer.user_id);
        } else if (!economyQueueEmpty) {
            // Fetch the first customer from the economy queue
            struct Customer customer = dequeue(economyQueue);
            economySize--;

            // Signal the economy queue that a customer is being served
            pthread_cond_signal(&customerServed);

            pthread_mutex_unlock(&businessQueueMutex);
            pthread_mutex_unlock(&economyQueueMutex);

            // Wait for the customer to finish service
            pthread_mutex_lock(&waitingTimeMutex);
            pthread_cond_wait(&customerServed, &waitingTimeMutex);
            pthread_mutex_unlock(&waitingTimeMutex);

            printf("Clerk %d finished serving customer ID %2d.\n", clerk_id, customer.user_id);
        } else {
            pthread_mutex_unlock(&businessQueueMutex);
            pthread_mutex_unlock(&economyQueueMutex);

            // No customers waiting, wait for a while and check again
            usleep(100000); // Sleep for 100 milliseconds
        }
    }

    return NULL;
}