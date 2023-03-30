#include "../defs.h"
#include "../student.h"

int num_stud, num_wm, simtime, wasted_time, fail;
student *student_list;

sem_t wm;
pthread_cond_t turn;
pthread_mutex_t turnMutex;

int min(int a, int b) { return a > b ? b : a; }
int max(int a, int b) { return a > b ? a : b; }

int get_last_time()
{
    int maxtime = 0;
    for (int i = 0; i < num_stud; i++)
    {
        maxtime = max(maxtime, student_list[i].T + student_list[i].P + student_list[i].W);
    }
    return maxtime + 5;
}

int main()
{
    num_stud = 0, num_wm = 0, simtime = 0, wasted_time = 0, fail = 0;
    pthread_mutex_init(&turnMutex, NULL);
    pthread_cond_init(&turn, NULL);

    scanf("%d %d", &num_stud, &num_wm);
    read_students(&student_list, num_stud);

    sem_init(&wm, 0, num_wm);
    int last_time = get_last_time(), i = 0;

    while (simtime <= last_time)
    {
        while (i < num_stud && simtime == student_list[i].T)
        {
            pthread_mutex_lock(&student_list[i].start);
            student_list[i].state = WAITING;
            pthread_create(&student_list[i].thread, NULL, &simulate_student, (void *)&student_list[i]);
            printf(KWHT"%d: Student %d arrives.\n"KNRM, simtime, student_list[i].index);
            i++;
        }

    sleep(1);
        for (int j = 0; j < num_stud; j++)
        {
            if (student_list[j].state == DONE || student_list[j].state == NOT_ARRIVED)
                continue;

            pthread_mutex_unlock(&student_list[j].start);
            pthread_mutex_lock(&student_list[j].stop);
            pthread_mutex_unlock(&student_list[j].stop);
            pthread_mutex_lock(&student_list[j].start);
        }
        
        simtime++;
    }
    void *retval;
    retval = malloc(sizeof(int));
    for (int i = 0; i < num_stud; i++)
    {
        pthread_join(student_list[i].thread, &retval);
    }
    printf(KWHT"%d\n", fail);
    printf("%d\n", wasted_time);
    
    if ((float)fail/num_stud >= 0.25) {
        printf("YES\n");
    }
    else printf("NO\n"KNRM);

    free(student_list);
}