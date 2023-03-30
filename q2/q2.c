#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <unistd.h>

#include "../include/pizza.h"
#include "../include/globals.h"
#include "../include/customer.h"
#include "../include/chef.h"
#include "../include/utils.h"

int num_pizzas, num_chefs, num_customers, num_ingredients;
int num_ovens, pickup_time;
int global_time, close_pizzeria;
int *test;

pizza **pizza_list;
pthread_mutex_t ingredientMutex;
int *ingredients;
chef **chef_list;
customer **customer_list;

pthread_mutex_t orderMutex;
pizza ordered_pizzas[MAXPIZZAS];
int pizza_front, pizza_end;
sem_t *ovens;

pthread_mutex_t customer_cond_mutex, chef_cond_mutex;
pthread_cond_t customer_cond, chef_cond;

int is_pizza_possible()
{
    int possible;
    for (int i = 0; i < num_pizzas; i++)
    {
        possible = 1;
        for (int j = 0; j < pizza_list[i]->ingredients; j++)
        {
            if (!ingredients[pizza_list[i]->ingredient_list[j]])
                possible = 0;
        }
        if (possible)
            break;
    }

    return possible;
}

int get_closing_time()
{
    int closing_time = 0;
    for (int i = 0; i < num_chefs; i++)
    {
        closing_time = max(closing_time, chef_list[i]->Texit);
    }

    return closing_time;
}

int main()
{
    close_pizzeria = 0;
    scanf("%d %d %d %d %d %d\n", &num_chefs, &num_pizzas,
          &num_ingredients, &num_customers, &num_ovens, &pickup_time);
    order_init();
    oven_init();
    cond_init();
    printf("\nSimulation Started.\n");

    read_pizzas(num_pizzas);
    read_ingredients(num_ingredients);
    read_chefs(num_chefs);
    read_customers(num_customers);

    int closing_time = get_closing_time();

    for (int i = 0; i < num_chefs; i++)
    {
        pthread_create(&chef_list[i]->thread, NULL, simulate_chef, (void *)&chef_list[i]);
    }
    for (int i = 0; i < num_customers; i++)
    {
        pthread_create(&customer_list[i]->thread, NULL, simulate_customer, (void *)&customer_list[i]);
    }
    sleep(1);
    // threads waiting on cond.
    int chef_idx = 0, customer_idx = 0;
    for (global_time = 0; global_time <= closing_time; global_time++)
    {
        pthread_cond_broadcast(&customer_cond);
        pthread_cond_broadcast(&chef_cond);
        sleep(1);

        // if (close_pizzeria)
        // {
        //     break;
        // }
    }
    for (int i = 0; i<num_customers; i++) {
        pthread_mutex_lock(&customer_list[i]->custMutex);
        customer_list[i]->rejected = customer_list[i]->npizzas - customer_list[i]->served;
        pthread_mutex_unlock(&customer_list[i]->custMutex);
    }
    pthread_cond_broadcast(&customer_cond);
    pthread_cond_broadcast(&chef_cond);
    sleep(1);
    sleep(1);
    printf("Simulation Ended.\n");

    // join threads and destroy sem, mutex and conds
    for (int i = 0; i < num_chefs; i++)
    {
        pthread_join(chef_list[i]->thread, NULL);
    }
    for (int i = 0; i < num_customers; i++)
    {
        pthread_join(customer_list[i]->thread, NULL);
    }
    destroy_ovens();
    cond_destroy();
    free_globals();
}