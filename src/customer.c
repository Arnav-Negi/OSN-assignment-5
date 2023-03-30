#include "../include/customer.h"
#include "../include/globals.h"
#include "../include/pizza.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

void create_customer(customer **c)
{
   (*c) = (customer*)malloc(sizeof(customer));
   (*c)->npizzas = 0;
   (*c)->plist = NULL,(*c)->pizza_status = NULL;
   (*c)->Tentry = 0;
   (*c)->rejected = 0,(*c)->served = 0;
    pthread_mutex_init(&((*c)->custMutex), NULL);
}

void destroy_customer(customer * c)
{
    free(c->plist);
    free(c->pizza_status);
    free(c);
}

void read_customers(int num_customers)
{
    customer_list = (customer **)malloc(sizeof(customer *) * num_customers);

    for (int i = 0; i < num_customers; i++)
    {
        create_customer(&customer_list[i]);
        scanf("%d %d", &customer_list[i]->Tentry, &customer_list[i]->npizzas);
        customer_list[i]->id = i;
        customer_list[i]->plist = (int *)malloc(sizeof(int) * customer_list[i]->npizzas);
        customer_list[i]->pizza_status = (int *)malloc(sizeof(int) * customer_list[i]->npizzas);
        for (int j = 0; j < customer_list[i]->npizzas; j++)
        {
            scanf("%d", &customer_list[i]->plist[j]);
            customer_list[i]->plist[j]--;
            customer_list[i]->pizza_status[j] = 0;
        }
    }
}

void *simulate_customer(void *arg)
{
    customer *self = *(customer **)(arg);
    self->state = C_NOT_ARRIVED;
    while (1)
    {
        pthread_mutex_lock(&customer_cond_mutex);
        pthread_cond_wait(&customer_cond, &customer_cond_mutex);
        pthread_mutex_unlock(&customer_cond_mutex);
        pthread_mutex_lock(&(self->custMutex));
        // sanity check if arrived.
        if (self->state == C_NOT_ARRIVED)
        {
            if (global_time >= self->Tentry)
            {
                printf(KYEL "Customer %d arrives at time %d.\n" KNRM, self->id + 1, global_time);
                printf(KYEL "Customer %d enters the drive-thru zone and gives out their order %d.\n" KNRM, self->id + 1, self->id + 1);
                self->state = C_ORDERING;
            }
        }

        if (self->state == C_ORDERING)
        {
            printf(KRED "Order %d placed by customer %d has pizzas {", self->id + 1, self->id + 1);

            for (int i = 0; i < self->npizzas; i++)
            {
                add_pizza(self->plist[i], self->id, i);
                if (i != self->npizzas - 1)
                    printf("%d,", self->plist[i] + 1);
                else
                    printf("%d}.\n" KNRM, self->plist[i] + 1);
            }

            printf(KRED "Order %d placed by customer %d awaits processing.\n" KNRM, self->id + 1, self->id + 1);
            printf(KRED "Order %d placed by customer %d is being processed.\n" KNRM, self->id + 1, self->id + 1);
            self->state = C_DRIVING;
            self->order_time = global_time;
        }
        // sanity check if rejected
        if (self->rejected == self->npizzas)
        {
            printf(KRED "Order %d placed by customer %d is completely rejected.\n" KNRM, self->id + 1, self->id + 1);
            printf(KYEL "Customer %d rejected.\n" KNRM, self->id + 1);
            printf(KYEL "Customer %d exits the drive-thru zone.\n" KNRM, self->id + 1);
            self->state = C_LEFT;
        }

        if (self->state == C_DRIVING)
        {
            if (global_time >= self->order_time + pickup_time)
            {
                self->state = C_WAITING;
            }
        }

        if (self->state == C_WAITING)
        {
            for (int i = 0; i < self->npizzas; i++)     
            {
                if (self->pizza_status[i] == 1)
                {
                    printf(KYEL "Customer %d picks up their pizza %d.\n" KNRM, self->id + 1, i);
                    self->served++;
                    self->pizza_status[i] = 2;
                }
            }

            if (self->served == self->npizzas)
            {   
                printf(KRED "Order %d placed by customer %d has been processed.\n" KNRM, self->id + 1, self->id + 1);
                printf(KYEL "Customer %d exits the drive thru zone.\n" KNRM, self->id + 1);
                self->state = C_LEFT;
            }
            else if (self->rejected + self->served == self->npizzas) {
                printf(KRED "Order %d placed by customer %d has been partially processed and remaining couldn’t be.\n" KNRM, self->id + 1, self->id + 1);
                printf(KYEL "Customer %d exits the drive thru zone.\n" KNRM, self->id + 1);
                self->state = C_LEFT;
            }
        }
        pthread_mutex_unlock(&(self->custMutex));

        if (self->state == C_LEFT)
        {
            pthread_exit(NULL);
        }
    }
}