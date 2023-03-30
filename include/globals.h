#ifndef __GLOBAL_H
#define __GLOBAL_H

#include <pthread.h>
#include <semaphore.h>

#define MAXPIZZAS 256

#include "chef.h"
#include "pizza.h"
#include "customer.h"

extern int global_time;
extern pthread_mutex_t orderMutex;
extern pizza ordered_pizzas[];
extern int pizza_front, pizza_end;

extern int num_pizzas, num_chefs, num_customers, num_ingredients; 
extern int num_ovens, pickup_time;
// extern int close_pizzeria;

extern pizza **pizza_list;
extern pthread_mutex_t ingredientMutex;
extern int *ingredients;
extern chef **chef_list;
extern customer **customer_list;
extern sem_t *ovens;

extern int *test;

extern pthread_mutex_t customer_cond_mutex, chef_cond_mutex;
extern pthread_cond_t customer_cond, chef_cond;

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#endif