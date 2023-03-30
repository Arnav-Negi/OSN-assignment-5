#include "../include/utils.h"
#include "../include/globals.h"
#include "../include/chef.h"
#include "../include/customer.h"
#include "../include/pizza.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

int min(int a, int b) { return a > b ? b : a; }
int max(int a, int b) { return a > b ? a : b; }

void oven_init()
{
    ovens = (sem_t *)malloc(sizeof(sem_t) * num_ovens);
    for (int i = 0; i < num_ovens; i++)
    {
        sem_init(&ovens[i], 0, 1);
    }
}

void cond_init()
{
    pthread_mutex_init(&orderMutex, NULL);
    pthread_mutex_init(&chef_cond_mutex, NULL);
    pthread_mutex_init(&customer_cond_mutex, NULL);
    pthread_mutex_init(&ingredientMutex, NULL);
    pthread_cond_init(&chef_cond, NULL);
    pthread_cond_init(&customer_cond, NULL);
}

void cond_destroy()
{
    pthread_mutex_destroy(&orderMutex);
    pthread_mutex_destroy(&ingredientMutex);
    pthread_mutex_destroy(&chef_cond_mutex);
    pthread_mutex_destroy(&customer_cond_mutex);
    pthread_cond_destroy(&chef_cond);
    pthread_cond_destroy(&customer_cond);
}

void destroy_ovens()
{
    for (int i = 0; i < num_ovens; i++)
    {
        sem_destroy(&ovens[i]);
    }
    free(ovens);
}

void free_globals()
{
    for (int i = 0; i<num_chefs; i++) {
        // destroy_chef(chef_list[i]);
        free(chef_list[i]);
    }
    free(chef_list);
    for (int i = 0; i<num_customers; i++) {
        destroy_customer(customer_list[i]);
    }
    free(customer_list);
    for (int i = 0; i<num_pizzas; i++) {
        destroy_pizza(pizza_list[i]);
    }
    free(pizza_list);
    free(ingredients);
}