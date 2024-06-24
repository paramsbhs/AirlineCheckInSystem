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

void inputFile(const char *filename, struct Queue *customerQueue, int *size) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    fscanf(file, "%d\n", size);
    for (int i = 0; i < *size; i++) {
        struct Customer customer;
        fscanf(file, "%d:%d,%d,%d\n", &customer.user_id, &customer.class_type, &customer.arrival_time, &customer.service_time);
        enqueue(customerQueue, customer);
    }
    fclose(file);
}

int main(int argc, char *argv[]){
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }
    struct Queue *customerQueue = createQueue();
    int size;

    inputFile(argv[1], customerQueue, &size);
    displayQueue(customerQueue);

    while (!isQueueEmpty(customerQueue)) {
        struct Customer customer = dequeue(customerQueue);
        // Process customer (for now, just print the details)
        printf("Processing Customer ID: %d, Class: %s, Arrival Time: %d (tenths of a second), Service Time: %d (tenths of a second)\n",
               customer.user_id, customer.class_type == 1 ? "Business" : "Economy", customer.arrival_time, customer.service_time);
    }
    free(customerQueue);
    return 0;
}
