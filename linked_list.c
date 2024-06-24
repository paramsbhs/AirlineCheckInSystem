#include <stdio.h>
#include <stdlib.h>
#include "linked_list.h"

struct Node* createNode(struct Customer customerData) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
    if (newNode == NULL) {
        perror("Memory Allocation Failed (Node)");
        return NULL;
    }
    newNode->customerData = customerData;
    newNode->next = NULL;
    return newNode;
}

struct Queue* createQueue() {
    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
    if (queue == NULL) {
        perror("Memory Allocation Failed (Queue)");
        return NULL;
    }
    queue->front = queue->rear = NULL;
    queue->size = 0;
    return queue;
}

int isQueueEmpty(struct Queue* queue) {
    return queue->front == NULL;
}

void enqueue(struct Queue* queue, struct Customer customerData) {
    struct Node* node = createNode(customerData);
    if (isQueueEmpty(queue)) {
        queue->front = queue->rear = node;
        return;
    } else {
        queue->rear->next = node;
        queue->rear = node;
    }
    queue->size++;
}

struct Customer dequeue(struct Queue* queue) {
    if (isQueueEmpty(queue)) {
        perror("Queue is empty");
        exit(EXIT_FAILURE);
    }
    struct Node* temp = queue->front;
    struct Customer customerData = temp->customerData;
    queue->front = queue->front->next;
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    free(temp);
    queue->size--;
    return customerData;
}

struct Customer peek(struct Queue* queue) {
    if (isQueueEmpty(queue)) {
        perror("Queue is empty");
        exit(EXIT_FAILURE);
    }
    return queue->front->customerData;
}

void displayQueue(struct Queue* queue) {
    struct Node* temp = queue->front;
    while (temp) {
        printf("Customer ID: %d, Class: %d, Arrival Time: %d, Service Time: %d\n",
               temp->customerData.user_id, temp->customerData.class_type, temp->customerData.arrival_time, temp->customerData.service_time);
        temp = temp->next;
    }
    printf("\n");
}
