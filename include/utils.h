#ifndef __UTILS_H
#define __UTILS_H

#include <pthread.h>
#include <semaphore.h>

int min(int a, int b);
int max(int a, int b);

void cond_init();
void oven_init();
void cond_destroy();
void destroy_ovens();
void free_globals();

#endif