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
double getCurrentSimulationTime();


#define QUEUE 2
#define CLERKS 5
#define TRUE 1
#define FALSE 0
#define IDLE 0

struct Queue *economyQueue;
struct Queue *businessQueue;
struct timeval start_time;

pthread_mutex_t businessQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t economyQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clerkAvailable = PTHREAD_COND_INITIALIZER;
pthread_cond_t customerServed = PTHREAD_COND_INITIALIZER;
pthread_mutex_t waitingTimeMutex = PTHREAD_MUTEX_INITIALIZER;

int size;
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

    if (pthread_mutex_init(&businessQueueMutex, NULL) != 0){ //mutex initialization
        printf("\n mutex init failed\n");
        return 1;
    }

    if (pthread_mutex_init(&economyQueueMutex, NULL) != 0){ //mutex initialization
        printf("\n mutex init failed\n");
        return 1;
    }

    usleep(100000); // 0.1 second delay
    gettimeofday(&start_time, NULL);

    economyQueue = createQueue(); //Initialize the economy Queue
    businessQueue = createQueue(); //Initialize the business Queue

    inputFile(argv[1], economyQueue, businessQueue, &size); //Read the input file and store the data in its respective queues

    pthread_t clerkThreads[CLERKS]; // Initialize the clerk threads
    pthread_t customerThreads[size]; // Initialize the customer threads

    //displayQueue(economyQueue); testing purposes
    //displayQueue(businessQueue); testing purposes

    pthread_attr_t clerkattr; //thread attributes
    pthread_attr_init(&clerkattr); //get the thread attributes
    int i, rc;
    for(i = 0; i < CLERKS; i++){
        int* clerk_id = (int*)malloc(sizeof(int)); //allocate memory for the clerk id
        *clerk_id = i+1; //set the clerk id to i
        if((rc = pthread_create(&clerkThreads[i], &clerkattr, clerkThread, clerk_id))){ //Create the clerk threads, Sample Code (pthread_create.c)
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
            return EXIT_FAILURE;
        }
    }

    int j = 0;
    pthread_attr_t customerattr;
    pthread_attr_init(&customerattr);
    struct Node *current = economyQueue->front;
    while (current != NULL) {
        struct Customer *customer = (struct Customer *)malloc(sizeof(struct Customer)); // Allocate memory for customer data
        memcpy(customer, &current->customerData, sizeof(struct Customer)); // Copy customer data
        if ((rc = pthread_create(&customerThreads[customer->user_id - 1], &customerattr, customerThread, customer))) {
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
            return EXIT_FAILURE;
        }
        current = current->next;
    }

    current = businessQueue->front;
    while (current != NULL) {
        struct Customer *customer = (struct Customer *)malloc(sizeof(struct Customer)); // Allocate memory for customer data
        memcpy(customer, &current->customerData, sizeof(struct Customer)); // Copy customer data
        if ((rc = pthread_create(&customerThreads[customer->user_id - 1], &customerattr, customerThread, customer))) {
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
            return EXIT_FAILURE;
        }
        current = current->next;
    }

    for(int k = 0; k < size; k++){
        pthread_join(customerThreads[k], NULL); //Join the customer threads
    }

    for(int l = 0; l < CLERKS-1; l++){
        pthread_join(clerkThreads[l], NULL); //Join the clerk threads
    }

    double total_simulation_time = getCurrentSimulationTime();
    printf("Simulation took %.2f seconds\n", total_simulation_time);
    printf("The average waiting time for all customers in the system is: %.2f seconds. \n", (businessWaitingTime + economyWaitingTime) / 8.0);
    printf("The average waiting time for all business-class customers is: %.2f seconds. \n", businessWaitingTime / 3.0);
    printf("The average waiting time for all economy-class customers is: %.2f seconds. \n", economyWaitingTime / 5.0);

    pthread_attr_destroy(&customerattr);
    pthread_attr_destroy(&clerkattr);
    pthread_mutex_destroy(&waitingTimeMutex); //Destroy the waiting time mutex
    pthread_mutex_destroy(&businessQueueMutex); //Destroy the business queue mutex
    pthread_mutex_destroy(&economyQueueMutex); //Destroy the economy queue mutex
    pthread_cond_destroy(&customerServed);
    pthread_cond_destroy(&clerkAvailable);

    free(economyQueue); //Free the economy queue
    free(businessQueue); //Free the business queue

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
            pthread_mutex_lock(&businessQueueMutex);
            enqueue(businessQueue, customer);
            businessSize++;
            pthread_mutex_unlock(&businessQueueMutex);
        }else{
            pthread_mutex_lock(&economyQueueMutex);
            enqueue(economyQueue, customer); //if the customer is an economy class customer, add it to the economy queue
            economySize++;
            pthread_mutex_unlock(&economyQueueMutex);
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
void* customerThread(void* param) {
    struct Customer* customer = (struct Customer*)param;
    usleep(customer->arrival_time * 100000);
    double current_time = getCurrentSimulationTime();
    if(customer->class_type == 1) {
        pthread_mutex_lock(&businessQueueMutex);
        pthread_mutex_lock(&waitingTimeMutex);
        printf("Customer %d arrived at time %.2f\n", customer->user_id, current_time);
        businessWaitingTime += current_time - (customer->arrival_time / 10.0);
        pthread_cond_signal(&clerkAvailable);
        pthread_mutex_unlock(&waitingTimeMutex);
        pthread_mutex_unlock(&businessQueueMutex);
    }else{
        pthread_mutex_lock(&economyQueueMutex);
        pthread_mutex_lock(&waitingTimeMutex);
        printf("Customer %d arrived at time %.2f\n", customer->user_id, current_time);
        economyWaitingTime += current_time - (customer->arrival_time / 10.0);
        pthread_cond_signal(&clerkAvailable);
        pthread_mutex_unlock(&waitingTimeMutex);
        pthread_mutex_unlock(&economyQueueMutex);
    }

    pthread_exit(NULL);
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
    usleep(100000); 
    while (!isEmpty(economyQueue) || !isEmpty(businessQueue)) {
        struct Customer customer;
        int isBusinessCustomer = 0;

        pthread_mutex_lock(&businessQueueMutex);
        if (!isEmpty(businessQueue)) {
            customer = dequeue(businessQueue);
            isBusinessCustomer = 1;
            pthread_mutex_unlock(&businessQueueMutex);
        } else {
            pthread_mutex_unlock(&businessQueueMutex);
            pthread_mutex_lock(&economyQueueMutex);
            if (!isEmpty(economyQueue)) {
                customer = dequeue(economyQueue);
                pthread_mutex_unlock(&economyQueueMutex);
            } else {
                pthread_mutex_unlock(&economyQueueMutex);
                continue;
            }
        }

        double start_time = getCurrentSimulationTime();
        double wait_time = start_time - (customer.arrival_time / 10.0);
        if (wait_time < 0){
            wait_time = 0;
        }
        printf("Customer %d spent %.2f seconds waiting before being served\n", customer.user_id, wait_time);
        printf("Clerk %d started taking care of customer %d\n", clerk_id, customer.user_id);
        usleep(customer.service_time * 100000); // Service time in tenths of a second
        double end_time = getCurrentSimulationTime();
        printf("Clerk %d finishes taking care of customer %d at time %.2f\n", clerk_id, customer.user_id, end_time);
    }
    pthread_exit(NULL);
}

double getCurrentSimulationTime() {
    struct timeval cur_time;
    gettimeofday(&cur_time, NULL);
    double elapsedTime = (cur_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsedTime += (cur_time.tv_usec - start_time.tv_usec) / 1000.0;
    return elapsedTime / 1000.0;
}