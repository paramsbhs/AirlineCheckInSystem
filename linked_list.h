#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

struct Customer {
    int user_id;
    int class_type;
    int service_time;
    int arrival_time;
};

struct Node {
    int customerData;
    struct Node* next;
};

struct Queue {
    struct Node* front;
    struct Node* rear;
    int size;
};

struct Queue* createQueue();
int isEmpty(struct Queue* queue);
void enqueue(struct Queue* queue, int customerData);
int dequeue(struct Queue* queue);
int peek(struct Queue* queue);
void QueueContents(struct Queue* queue);
void printList(struct Node* node);

#endif