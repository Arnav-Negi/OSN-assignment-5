#ifndef __CHEF_H
#define __CHEF_H

#include <pthread.h>

typedef struct {
    int Tenter, Texit;
    enum state_chef {CH_NOT_ARRIVED, CH_WAITING_ORDER, CH_ASSIGNED, CH_WAITING_OVEN, CH_IN_OVEN, CH_LEFT} state;
    int cust_assigned, pizza_assigned;
    int id;
    int lastchef;
    int time_start_prep;
    int oven_no;
    int oven_time;
    pthread_t thread;
} chef;

void createchef(chef **c);
void destroy_chef(chef* c);
void read_chefs(int num_chefs);
int chefcomp(const void *a, const void *b);
int checkpizza(int pizzaid);
void *simulate_chef(void *arg);

#endif