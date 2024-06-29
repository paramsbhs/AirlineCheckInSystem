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

#define QUEUE 2
#define CLERKS 5
#define TRUE 1
#define FALSE 0
#define IDLE 0

pthread_mutex_t businessQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t economyQueueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t clerkAvailable = PTHREAD_COND_INITIALIZER;


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

    //displayQueue(economyQueue); testing purposes
    //displayQueue(businessQueue); testing purposes

    pthread_t clerk; //thread identifier
    pthread_attr_t clerkattr; //thread attributes
    pthread_attr_init(&clerkattr); //get the thread attributes
    int i, rc;
    for(i = 0; i < CLERKS; i++){
        if((rc = pthread_create(&clerkThreads[i], &clerkattr, clerkThread, NULL))){ //Create the clerk threads, Sample Code (pthread_create.c)
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
            return EXIT_FAILURE;
        }
    }

    pthread_t customer;
    pthread_attr_t customerattr;
    pthread_attr_init(&customerattr);
    int j, rc2;
    for(j = 0; j < size; j++){
        if((rc2 = pthread_create(&customerThreads[j], &customerattr, customerThread, NULL))){ //Create the customer threads, Sample Code (pthread_create.c)
            fprintf(stderr, "error: pthread_create, rc: %d\n", rc2);
            return EXIT_FAILURE;
        }
    }
    //create all the threads
    //then process


    pthread_join(clerk, NULL);
    free(businessQueue); //Free the business queue
    free(economyQueue); //Free the economy queue
    sleep(3);//
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

}

/*
    The clerkThread function simulates the clerk
    processing the customer, it acquires the queue
    mutex, dequeues the customer from the queue,
    sleeps for the service time, releases the queue
    mutex, and returns NULL.
*/
void* clerkThread(void* param){

}