#include "defs.h"

#include "student.h"

extern sem_t wm;
extern int simtime, wasted_time, fail;
extern pthread_cond_t turn;
extern pthread_mutex_t turnMutex;

int stud_time_cmp(const void *a, const void *b)
{
    student *A = (student *)a;
    student *B = (student *)b;

    return A->T > B->T;
}

void read_students(student **student_list, int num_stud)
{
    (*student_list) = (student *)malloc(sizeof(student) * num_stud);
    if ((*student_list) == NULL)
    {
        printf("Out of memory.\n");
        exit(1);
    }

    for (int i = 0; i < num_stud; i++)
    {
        scanf("%d %d %d", &((*student_list)[i].T), &((*student_list)[i].W), &((*student_list)[i].P));
        (*student_list)[i].state = NOT_ARRIVED;
        pthread_mutex_init(&(*student_list)[i].start, NULL);
        pthread_mutex_init(&(*student_list)[i].stop, NULL);
        (*student_list)[i].index = i + 1;
    }
    qsort((*student_list), num_stud, sizeof(student), stud_time_cmp);
    return;
}

void *simulate_student(void *arg)
{
    // sem_wait(&wm);
    student *self = (student *)arg;
    int leavetime = self->T + self->P;
    int lasttime = leavetime + self->W + 5;
    while (simtime <= lasttime)
    {
        // pthread_mutex_lock(&turnMutex);
        // pthread_cond_wait(&turn, &turnMutex);

        // check for wm
        pthread_mutex_lock(&self->stop);
        pthread_mutex_lock(&self->start);

        if (self->state == WAITING)
        {

            int res = sem_trywait(&wm);
            if (!res)
            {
                // got a washing machine.
                printf(KGRN"%d: Student %d starts washing.\n"KNRM, simtime, self->index);
                self->state = WASHING;
                self->wash_start = simtime;
            }
            else
            {
                wasted_time++;
                if (leavetime <= simtime)
                {
                    printf(KRED"%d: Student %d leaves without washing.\n"KNRM, simtime, self->index);
                    self->state = DONE;
                    fail++;
                    pthread_mutex_unlock(&self->start);
                    pthread_mutex_unlock(&self->stop);
                    pthread_exit(0);
                }
            }
        }
        else if (self->state == WASHING)
        {
            if (simtime >= self->wash_start + self->W)
            {
                sem_post(&wm);
                printf(KYEL"%d: Student %d leaves after washing.\n"KNRM, simtime, self->index);
                self->state = DONE;
                pthread_mutex_unlock(&self->start);
                pthread_mutex_unlock(&self->stop);
                pthread_exit(0);
            }
        }

        pthread_mutex_unlock(&self->start);
        pthread_mutex_unlock(&self->stop);
        sleep(1);
    }
}
