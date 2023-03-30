#ifndef __PIZZA_H
#define __PIZZA_H

typedef struct {
    int id;
    int orderno;
    int ingredients;
    int prep_time;
    int *ingredient_list;
    int order_rank;
    enum state_pizza {P_WAITING, P_ASSIGNED, P_OVEN, P_READY, P_REJECTED} state;
} pizza;

void create_pizza(pizza **P);
void destroy_pizza(pizza *P);
void read_pizzas(int num_pizzas);
void read_ingredients(int num_ingredients);
void order_init();
void add_pizza(int pizzaid, int orderno, int order_rank);

#endif