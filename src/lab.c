#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include "lab.h"

// Opaque type definition for the queue
struct queue {
    int capacity;
    bool shutdown;
    bool empty;
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    void **data; // Pointer to the data array
    int head;   // Index of the head of the queue
    int tail;   // Index of the tail of the queue
    int count;  // Number of elements in the queue
};

queue_t queue_init(int capacity) {
    // Allocate memory for the queue structure
    queue_t q = malloc(sizeof(struct queue));
    if (q == NULL) {
        return NULL; // Memory allocation failed
    }

    // Initialize the queue fields
    q->capacity = capacity;
    q->shutdown = false;
    q->empty = true;
    q->head = 0;
    q->tail = 0;
    q->count = 0;

    // Allocate memory for the data array
    q->data = malloc(capacity * sizeof(void *));
    if (q->data == NULL) {
        free(q);
        return NULL; // Memory allocation failed
    }

    // Initialize mutex and condition variables
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->not_empty, NULL);
    pthread_cond_init(&q->not_full, NULL);

    return q;
}

void queue_destroy(queue_t q) {
    if (q == NULL) {
        return; // Nothing to destroy
    }

    // Free the data array
    free(q->data);

    // Destroy mutex and condition variables
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->not_empty);
    pthread_cond_destroy(&q->not_full);

    // Free the queue structure itself
    free(q);
    q = NULL; // Avoid dangling pointer
}

void enqueue(queue_t q, void *data) { 

    // Lock the mutex to ensure thread safety
    pthread_mutex_lock(&q->mutex);

    // Check if the queue is in shutdown state
    if (q->shutdown) {
        pthread_mutex_unlock(&q->mutex);
        return; // Do not enqueue if the queue is shutting down
    }

    // Wait until the queue is not full
    while (q->count == q->capacity) {
        pthread_cond_wait(&q->not_full, &q->mutex);
        if (q->shutdown) {
            pthread_mutex_unlock(&q->mutex);
            return;
        }
    }
    
    // Add the data to the queue
    q->data[q->tail] = data;
    q->tail = (q->tail + 1) % q->capacity; // Circular increment
    q->count++;
    q->empty = false; // Queue is not empty anymore

    pthread_cond_signal(&q->not_empty); // Signal that the queue is not empty

    if(q->count < q->capacity) {
        pthread_cond_signal(&q->not_full); // Signal that the queue is not full
    }

    pthread_mutex_unlock(&q->mutex); // Unlock the mutex

}

void *dequeue(queue_t q) {
    
    // Lock the mutex to ensure thread safety
    pthread_mutex_lock(&q->mutex);
    // Wait until the queue is not empty
    while (q->count == 0) {
        pthread_cond_wait(&q->not_empty, &q->mutex);
        if (q->shutdown) {
            pthread_mutex_unlock(&q->mutex);
            return NULL;
        }
    }

    // Remove the data from the queue
    void *data = q->data[q->head];
    q->head = (q->head + 1) % q->capacity; // Circular increment
    q->count--;
    
    if (q->count == 0) {
        q->empty = true; // Queue is empty now
    }

    pthread_cond_signal(&q->not_full); // Signal that the queue is not full

    pthread_mutex_unlock(&q->mutex); // Unlock the mutex

    return data;
}

void queue_shutdown(queue_t q) {
    q->shutdown = true; // Set the shutdown flag
    pthread_cond_broadcast(&q->not_empty); // Wake up all waiting threads
    pthread_cond_broadcast(&q->not_full); // Wake up all waiting threads
}

bool is_empty(queue_t q) {
    return q->empty;
}

bool is_shutdown(queue_t q) {
    return q->shutdown;
}