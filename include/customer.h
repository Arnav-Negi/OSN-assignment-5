#ifndef __CUSTOMER_H
#define __CUSTOMER_H

#include <pthread.h>

typedef struct {
    int Tentry;
    int id;
    int npizzas;
    int *plist, *pizza_status;  //0 : ordered, -1 : rejected, 1 : ready, 2 : picked up
    int order_time;
    int rejected, served;
    pthread_t thread;
    pthread_mutex_t custMutex;
    enum state_customer {C_NOT_ARRIVED, C_ORDERING, C_DRIVING, C_WAITING, C_LEFT} state;
} customer;

void create_customer(customer** c);
void destroy_customer(customer *c);
void read_customers(int num_customers);
void *simulate_customer(void *arg);

#endif