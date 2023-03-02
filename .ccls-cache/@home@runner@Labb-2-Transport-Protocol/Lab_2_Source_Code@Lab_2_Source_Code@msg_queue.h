#ifndef MSG_QUEUE
#define MSG_QUEUE

#include <stdbool.h>
#include <stdio.h>

#include "Sim_Engine.h"

#define MAX_SIZE 100

void enqueue(struct msg message);
struct msg dequeue();
struct msg get_first();
bool isEmpty();
bool isFull();
int get_length();
void print_queue();

#endif