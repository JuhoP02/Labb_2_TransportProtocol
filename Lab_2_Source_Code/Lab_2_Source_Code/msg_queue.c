#include <string.h>

#include "msg_queue.h"

int front = 0;
int rear = -1;
int size = 0;

struct msg msg_queue[MAX_SIZE];

void enqueue(struct msg message) {

  // Queue is full
  if (isFull())
    return;

  if (rear == (MAX_SIZE - 1))
    rear = -1;

  rear++;
  msg_queue[rear] = message;
  size++;
}

struct msg dequeue() {

  // Queue is empty
  if (isEmpty()) {
    struct msg empty;
    memset(empty.data, ' ', 20);
    return empty;
  }

  if (front == MAX_SIZE)
    front = 0;

  // Reduce amount of elements in queue
  size--;

  // Return first element
  return msg_queue[front++];
}

struct msg get_first() {
  // Queue is empty
  if (isEmpty()) {
    struct msg empty;
    return empty;
  }

  return msg_queue[front];
}

bool isEmpty() { return size == 0; }

bool isFull() { return size == MAX_SIZE; }

int get_length() { return size; }

void print_queue() {
  printf("Elements in queue:\n");
  for (int i = front; i <= rear; i++) {
    printf("[%d]\t%s\n", i, msg_queue[i].data);
  }
}