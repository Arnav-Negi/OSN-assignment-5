#include "../include/pizza.h"
#include "../include/globals.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

void create_pizza(pizza **P)
{
    (*P) = (pizza*)malloc(sizeof(pizza));
    (*P)->id = -1;
    (*P)->ingredient_list = NULL;
    (*P)->ingredients = 0;
    (*P)->prep_time = 0;
    (*P)->state = P_WAITING;
}

void destroy_pizza(pizza *P)
{
    free(P->ingredient_list);
    free(P);
}

void read_pizzas(int num_pizzas)
{
    pizza_list = (pizza **) malloc(sizeof(pizza*)*num_pizzas);
    for (int i = 0; i<num_pizzas; i++)
    {
        create_pizza(&pizza_list[i]);
        scanf("%d %d %d", &pizza_list[i]->id, &pizza_list[i]->prep_time, &pizza_list[i]->ingredients);
        pizza_list[i]->id--;
        pizza_list[i]->ingredient_list = malloc(sizeof(int)*pizza_list[i]->ingredients);
        for (int j = 0; j<pizza_list[i]->ingredients; j++) {
            scanf("%d", &pizza_list[i]->ingredient_list[j]);
            pizza_list[i]->ingredient_list[j]--;
        }
    }
}

void read_ingredients(int num_ingredients) {
    ingredients = (int*)malloc(sizeof(int)*num_ingredients);
    for (int i = 0; i<num_ingredients; i++) {
        scanf("%d", &ingredients[i]);
    }
}

void order_init()
{
    pizza_end = -1, pizza_front = 0;
}

void add_pizza(int pizzaid, int orderno, int order_rank)
{
    pthread_mutex_lock(&orderMutex);
    pizza_end++;
    ordered_pizzas[pizza_end].order_rank = order_rank;
    ordered_pizzas[pizza_end].id = pizzaid;
    ordered_pizzas[pizza_end].ingredient_list = pizza_list[pizzaid]->ingredient_list;
    ordered_pizzas[pizza_end].orderno = orderno;
    ordered_pizzas[pizza_end].prep_time = pizza_list[pizzaid]->prep_time;
    ordered_pizzas[pizza_end].ingredients = pizza_list[pizzaid]->ingredients;
    ordered_pizzas[pizza_end].state = P_WAITING;


    pthread_mutex_unlock(&orderMutex);
}