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


void inputFile(const char *filename, struct Customer **customers, int *size) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    fscanf(file, "%d\n", size);
    *customers = (struct Customer *)malloc((*size) * sizeof(struct Customer));
    if (*customers == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < *size; i++) {
        fscanf(file, "%d:%d,%d,%d\n",&(*customers)[i].user_id,&(*customers)[i].class_type,&(*customers)[i].arrival_time,&(*customers)[i].service_time);
    }
    fclose(file);
}


int main(int argc, char *argv[]){
     if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }
    struct Queue* CustomerQueue = createQueue();
    int size;

    inputFile(argv[1], CustomerQueue*, &size);
    displayQueue(CustomerQueue);
        while (!isQueueEmpty(CustomerQueue)) {
        struct Customer customer = dequeue(CustomerQueue);
        // Process customer (for now, just print the details)
        printf("Processing Customer ID: %d, Class: %s, Arrival Time: %d (tenths of a second), Service Time: %d (tenths of a second)\n",
               customer.user_id, customer.class_type == 1 ? "Business" : "Economy", customer.arrival_time, customer.service_time);
    }
    free(queue);
    return 0;
}