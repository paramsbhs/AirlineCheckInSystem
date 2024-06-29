#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "linked_list.h"

#define QUEUE 2
#define CLERKS 5
#define TRUE 1
#define FALSE 0
#define IDLE 0

pthread_mutex_t businessQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t economyQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clerkAvailable = PTHREAD_COND_INITIALIZER;

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
void* customerThread(void* customerArg){
    struct ThreadArgs* customerArguments = (struct ThreadArgs*)customerArg;
    struct Queue* businessQueue = customerArguments->businessQueue;
    struct Queue* economyQueue = customerArguments->economyQueue;
    struct Customer customer = customerArguments->customer;
    usleep(customer->arrival_time*100000); //sleep until the customer arrives, multiply by 100000 to convert to microseconds
    printf("A customer arrives: customer ID %2d. \n", customer->user_id); //print that the customer has arrived
    if(customer->class_type == 1){ //if the customer is in business class
        pthread_mutex_lock(&businessQueueMutex); //lock the business queue
        enqueue(businessQueue, *customer); //add the customer to the business queue
        printf("A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n", customer->class_type, businessQueue->size); //print contents
        pthread_mutex_unlock(&businessQueueMutex); //unlock the business queue
    }else{ //if the customer is in economy class
        pthread_mutex_lock(&economyQueueMutex); //lock the economy queue
        enqueue(economyQueue, *customer); //add the customer to the economy queue
        printf("A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n", customer->class_type, economyQueue->size); //print contents
        pthread_mutex_unlock(&economyQueueMutex); //unlock the economy queue
    }
    pthread_cond_signal(&clerkAvailable); // Signal the clerk thread that a customer is available for processing.
    return NULL;
}

void* clerkThread(void* customer){

}

int main(int argc, char *argv[]){
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    struct Queue *economyQueue = createQueue(); //Initialize the economy Queue
    struct Queue *businessQueue = createQueue(); //Initialize the business Queue
    int size; //Initialize the size of the queue

    inputFile(argv[1], economyQueue, businessQueue, &size); //Read the input file and store the data in its respective queues

    pthread_t clerkThreads[CLERKS]; // Initialize the clerk threads
    pthread_t customerThreads[size]; // Initialize the customer threads

    displayQueue(economyQueue);
    displayQueue(businessQueue);

    pthread_t clerk; //Initialize the clerk thread
    pthread_create(&clerk, NULL, clerkThread, NULL); //Create the clerk thread
    //create all the threads
    //then process

    while (!isEmpty(economyQueue) || !isEmpty(businessQueue)) { //While the queues are not empty, run the simulation
        //customer thread function
        // clerk thread function
        // join customer and clerk threads
    }
    usleep(2); 
    free(businessQueue); //Free the business queue
    free(economyQueue); //Free the economy queue
    return 0;
}
