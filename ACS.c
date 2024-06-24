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


struct Customer {
    int user_id;
    int class_type;
    int service_time;
    int arrival_time;
};

void inputFile(const char *filename, struct Customer **customers, int *totalCustomers) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }
    fscanf(file, "%d\n", totalCustomers);
    *customers = (struct Customer *)malloc((*totalCustomers) * sizeof(struct Customer));
    if (*customers == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < *totalCustomers; i++) {
        fscanf(file, "%d:%d,%d,%d\n",
               &(*customers)[i].user_id,
               &(*customers)[i].class_type,
               &(*customers)[i].arrival_time,
               &(*customers)[i].service_time);
    }

    fclose(file);
}


int main(int argc, char *argv[]){
     if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }
    struct Customer *customers;
    int totalCustomers;

    inputFile(argv[1], &customers, &totalCustomers);
    for (int i = 0; i < totalCustomers; i++) {
        printf("Customer %d\n", customers[i].user_id);
        printf("  Class: %s\n", customers[i].class_type == 1 ? "Business" : "Economy");
        printf("  Arrival Time: %d (tenths of a second)\n", customers[i].arrival_time);
        printf("  Service Time: %d (tenths of a second)\n", customers[i].service_time);
    }
    return 0;
}