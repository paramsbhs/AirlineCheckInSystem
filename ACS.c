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
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    fscanf(file, "%d\n", size);
    for (int i = 0; i < *size; i++) {
        struct Customer customer;
        fscanf(file, "%d:%d,%d,%d\n", &customer.user_id, &customer.class_type, &customer.arrival_time, &customer.service_time);
        if(customer.class_type == 1){
            enqueue(businessQueue, customer);
        }else{
            enqueue(economyQueue, customer);
        }
    }
    fclose(file);
}

void* createCusomterThread(void* customer){
    struct Customer* customerData = (struct Customer*)customer;
    sleep(customerData->service_time);
    printf("A customer arrives: customer ID %2d. \n", customerData->user_id);
    if(customerData->class_type == 1){
        pthread_mutex_lock(&businessQueueMutex);
        //enqueue(businessQueue, customerData);
        pthread_mutex_unlock(&businessQueueMutex);
    }else{
        pthread_mutex_lock(&economyQueueMutex);
        //enqueue(economyQueue, customerData);
        pthread_mutex_unlock(&economyQueueMutex);
    }
    pthread_cond_signal(&clerkAvailable);
    pthread_exit(NULL);

}

int main(int argc, char *argv[]){
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }
    struct Queue *economyQueue = createQueue();
    struct Queue *businessQueue = createQueue();
    int size;

    inputFile(argv[1], economyQueue, businessQueue, &size);
    displayQueue(economyQueue);
    displayQueue(businessQueue);

    while (!isEmpty(economyQueue) || !isEmpty(businessQueue)) {
        struct Customer economyCustomer = dequeue(economyQueue);
        struct Customer businessCustomer = dequeue(businessQueue);
        //create customer threads
        // Initialize mutexes and condition variables
        // Create clerk threads
    }
    // Join customer and clerk threads
    free(businessQueue);
    free(economyQueue);
    return 0;
}
