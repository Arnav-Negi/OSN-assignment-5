#include "../include/chef.h"
#include "../include/pizza.h"
#include "../include/globals.h"
#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

int chefcomp(const void *a, const void *b)
{
    return ((chef *)a)->Texit > ((chef *)b)->Texit;
}

void createchef(chef **c) 
{
    (*c) = (chef*)malloc(sizeof(chef));
    (*c)->cust_assigned = -1;
    (*c)->lastchef = 0;
    (*c)->oven_no = -1;
    (*c)->oven_time = -1;
    (*c)->pizza_assigned = -1;
    (*c)->state = CH_NOT_ARRIVED;
    (*c)->time_start_prep = -1;
}

void destory_chef(chef *c)
{
    free(c);
}

void read_chefs(int num_chefs)
{
    chef_list = (chef **)malloc(sizeof(chef *) * num_chefs);
    for (int i = 0; i < num_chefs; i++)
    {
        createchef(&chef_list[i]);
        scanf("%d %d", &chef_list[i]->Tenter, &chef_list[i]->Texit);
        chef_list[i]->id = i;
    }

    qsort(chef_list, num_chefs, sizeof(chef *), chefcomp);
    if (num_chefs)
    chef_list[num_chefs - 1]->lastchef = 1;
}

int checkpizza(int pizzaid)
{
    for (int i = 0; i<pizza_list[pizzaid]->ingredients; i++) {
        if (!ingredients[pizza_list[pizzaid]->ingredient_list[i]]) return 0;
    }
    return 1;
}

void *simulate_chef(void *arg)
{
    chef *self = *(chef **)(arg);
    self->state = CH_NOT_ARRIVED;
    pizza assigned;
    int rc;
    while (1)
    {
        pthread_mutex_lock(&chef_cond_mutex);
        pthread_cond_wait(&chef_cond, &chef_cond_mutex);
        pthread_mutex_unlock(&chef_cond_mutex);
        if (self->state == CH_NOT_ARRIVED)
        {
            if (global_time >= self->Tenter)
            {
                self->state = CH_WAITING_ORDER;
                printf(KBLU "Chef %d arrived at time %d.\n"KNRM, self->id + 1, global_time);
            }
        }

        if (self->state == CH_WAITING_ORDER)
        {
            pthread_mutex_lock(&orderMutex);
            // accept if Texit > preptime + time and enough ingredients.
            // else if no ingredients, reject.
            // else if no time, and lastchef, reject.
            for (int i = pizza_front; i < pizza_end; i++)
            {
                pthread_mutex_lock(&ingredientMutex);
                int valid = checkpizza(ordered_pizzas[i].id);
                if (ordered_pizzas[i].state != P_WAITING) valid = 0;
                if (ordered_pizzas[i].prep_time + global_time <= self->Texit && valid) {
                    assigned = ordered_pizzas[i];
                    ordered_pizzas[i].state = P_ASSIGNED;
                    for (int j = 0; j<assigned.ingredients; j++) {
                        ingredients[assigned.ingredient_list[j]]--;
                    }
                    pthread_mutex_unlock(&ingredientMutex);
                    self->cust_assigned = assigned.orderno;
                    self->pizza_assigned = assigned.id;
                    self->state = CH_ASSIGNED;
                    self->time_start_prep = global_time;
                    printf(KRED"Pizza %d in order %d assigned to chef %d.\n"KNRM, assigned.id + 1, assigned.orderno + 1, self->id + 1);
                    printf(KBLU"Chef %d is preparing the pizza %d from order %d.\n"KNRM, self->id + 1, assigned.id + 1, assigned.orderno + 1);
                    
                    break;
                }
                else if ((!valid && ordered_pizzas[i].state == P_WAITING) || (self->lastchef && ordered_pizzas[i].prep_time + global_time > self->Texit)) {
                    ordered_pizzas[i].state = P_REJECTED;
                    customer_list[ordered_pizzas[i].orderno]->rejected++;
                    customer_list[ordered_pizzas[i].orderno]->pizza_status[ordered_pizzas[i].order_rank] = -1;
                }
                pthread_mutex_unlock(&ingredientMutex);
            }    
            pthread_mutex_unlock(&orderMutex);        
        }

        if (self->state == CH_ASSIGNED) 
        {
            if (global_time >= self->time_start_prep + 3) {
                self->state = CH_WAITING_OVEN;
            }
        }

        if (self->state == CH_WAITING_OVEN) {
            rc = 1;
            for (int i = 0; i<num_ovens; i++) {
                rc = sem_trywait(&ovens[i]);
                if (rc == 0) {
                    printf(KBLU"Chef %d has put the pizza %d for order %d in the oven at time %d.\n"KNRM, self->id + 1, assigned.id + 1, assigned.orderno + 1, global_time);
                    self->state = CH_IN_OVEN;
                    self->oven_no = i;
                    self->oven_time = global_time;
                    break;
                }
            }
        }

        if (self->state == CH_IN_OVEN)
        {
            if (global_time >= customer_list[assigned.orderno]->order_time + pickup_time && global_time >= self->oven_time + pizza_list[self->pizza_assigned]->prep_time - 3) {
                sem_post(&ovens[self->oven_no]);
                pthread_mutex_lock(&customer_list[assigned.orderno]->custMutex);
                customer_list[assigned.orderno]->served++;
                pthread_mutex_unlock(&customer_list[assigned.orderno]->custMutex);
                printf(KBLU"Chef %d has picked up the pizza %d for order %d from the oven at time %d.\n"KNRM, self->id + 1, assigned.orderno + 1, assigned.orderno + 1, global_time);
                self->cust_assigned = -1;
                self->oven_no = -1;
                self->oven_time = -1;
                self->pizza_assigned = -1;
                self->time_start_prep = -1;
                self->state = CH_WAITING_ORDER;
            }
        }

        if (self->Texit <= global_time)
        {
            printf(KBLU"Chef %d exits at time %d.\n"KNRM, self->id + 1, global_time);
            self->state = CH_LEFT;
            pthread_exit(NULL);
        }
    }
}