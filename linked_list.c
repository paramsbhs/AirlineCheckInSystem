#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "linked_list.h"

struct Node* createNode(struct Customer customerData) {
    struct Node* newNode = (struct Node*)malloc(sizeof(struct Node)); //Allocate memory for a new Node
    if (newNode == NULL) {
        perror("Memory Allocation Failed (Node)"); //Check to see if memory allocation is successful
        return NULL;
    }
    newNode->customerData = customerData; //give the new node data
    newNode->next = NULL; //set the nodes next to NULL
    return newNode;
}

struct Queue* createQueue() { //implementation from https://www.geeksforgeeks.org/queue-linked-list-implementation/
    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue)); //Allocate memory for a new Queue
    if (queue == NULL) {
        perror("Memory Allocation Failed (Queue)"); //Check to see if memory allocation is successful
        return NULL;
    }
    queue->front = queue->rear = NULL; //set the front and rear of the queue to NULL
    queue->size = -1; //set the size of the queue to 0
    return queue;
}

int isEmpty(struct Queue* queue) {
    return queue->front == NULL; //check if the front of the queue is empty
}

void enqueue(struct Queue* queue, struct Customer customerData) {
    struct Node* node = createNode(customerData); //create a new node
    if (isEmpty(queue)) {
        queue->front = queue->rear = node; //set the front and rear of the queue to the new node
        return;
    } else {
        queue->rear->next = node; //set the rear of the queue to the new node
        queue->rear = node; //set the rear of the queue to the new node
    }
    queue->size++; //increment the size of the queue
}

struct Customer dequeue(struct Queue* queue) {
    if (isEmpty(queue)) { //if the queue is empty, exit
        perror("Queue is empty");
        exit(EXIT_FAILURE);
    }
    struct Node* temp = queue->front; //create a temporary node and set it to the front of the queue
    struct Customer customerData = temp->customerData; //get the data from the temporary node
    queue->front = queue->front->next; //set the front of the queue to the next node
    if (queue->front == NULL) { //if the front of the queue is NULL
        queue->rear = NULL; //set the rear of the queue to NULL
    }
    free(temp); //free the temporary node
    queue->size--; //decrement the size of the queue
    return customerData; //return the data
} 

struct Customer peek(struct Queue* queue) {
    if (isEmpty(queue)) { //if the queue is empty, exit
        perror("Queue is empty");
        exit(EXIT_FAILURE);
    }
    return queue->front->customerData; //return the data at the front of the queue
}

void displayQueue(struct Queue* queue) {
    struct Node* temp = queue->front; //create a temporary node and set it to the front of the queue
    while (temp != NULL) { //loop through the queue
        printf("Customer ID: %d, Class: %d, Arrival Time: %d, Service Time: %d\n", temp->customerData.user_id, temp->customerData.class_type, temp->customerData.arrival_time, temp->customerData.service_time); //print the data of the current node
        temp = temp->next; //traverse through the queue
    }
}
