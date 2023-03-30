enum state {NOT_ARRIVED, WAITING, WASHING, DONE};

typedef struct {
    int T, W, P, time, index;
    int wash_start;
    pthread_mutex_t start, stop;
    enum state state;
    pthread_t thread;
} student;

int stud_time_cmp(const void *a, const void *b);
void read_students(student **student_list, int num_stud);
void *simulate_student(void *arg);