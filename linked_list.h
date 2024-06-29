#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_
#include <pthread.h>

struct Customer {
    int user_id;
    int class_type;
    int service_time;
    int arrival_time;
    pthread_t thread;
};

struct Node {
    struct Customer customerData;
    struct Node* next;
};

struct Queue {
    struct Node* front;
    struct Node* rear;
    int size;
};

struct Queue* createQueue();
int isEmpty(struct Queue* queue);
void enqueue(struct Queue* queue, struct Customer customerData);
struct Customer dequeue(struct Queue* queue);
struct Customer peek(struct Queue* queue);
void displayQueue(struct Queue* queue);

#endif