#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

struct Node {
    int customerData;
    struct Node* next;
};

struct Queue {
    struct Node* front;
    struct Node* rear;
    int size;
};

Queue* createQueue();
int isEmpty(struct Queue* queue);
void enqueue(struct Queue* queue, int customerData);
int dequeue(struct Queue* queue);
int peek(struct Queue* queue);
void QueueContents(struct Queue* queue);
void printList(Node* node);

#endif