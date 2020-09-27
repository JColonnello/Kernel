#pragma once
#include <stddef.h>
#include <stdbool.h>

typedef struct Queue Queue;

Queue *Queue_Create(int capacity, size_t elemSize);
void Queue_Enqueue(Queue *queue, void *elem);
bool Queue_Dequeue(Queue *queue, void *elem);
bool Queue_Peek(Queue *queue, void *elem);
int Queue_Count(Queue *queue);
void Queue_ToArray(Queue *queue, void *array);
void Queue_Dispose(Queue *queue);