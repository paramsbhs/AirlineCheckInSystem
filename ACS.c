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
        struct Customer customer = dequeue(economyQueue);
        struct Customer customer = dequeue(businessQueue);
        //write code here
    }
    free(economyQueue);
    return 0;
}
