#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include "a2_helper.h"
#define SNAME_ST "sem4_start_T3_1"
#define SNAME_EN "sem4_ended_T3_1"


sem_t* sem4_start_T3_1;
sem_t* sem4_ended_T3_1;


sem_t sem2_start_T4;
sem_t sem2_end_T2;
sem_t wait_for_op;

void* thread_func2(void* arg){
    long id = (long) arg;
    if(id == 1) sem_wait(sem4_start_T3_1);

    if(id == 4) sem_wait(&sem2_start_T4);
    info(BEGIN, 3, id);
    if(id == 2) sem_post(&sem2_start_T4);

    if(id == 2) sem_wait(&sem2_end_T2);
    info(END, 3, id);
    if(id == 4) sem_post(&sem2_end_T2);

    if(id == 1) sem_post(sem4_ended_T3_1);
    return NULL;
}

int nr_threads_remaining;
int nr_threads_running;
sem_t wait3_for_op;
sem_t sem3_T14_finished;
sem_t sem3_limiter;
sem_t sem3_6_threads_running;

void* thread_func3(void* arg){
    long id = (long) arg;
    int aux;
    
    sem_wait(&sem3_limiter);
    info(BEGIN, 4, id);

    sem_wait(&wait3_for_op); 
    ++nr_threads_running;
    if(nr_threads_running == 6) sem_post(&sem3_6_threads_running);
    sem_post(&wait3_for_op);

    if(id == 14) {
        sem_wait(&sem3_6_threads_running);
        sem_post(&sem3_6_threads_running);
    }
    else {
        sem_wait(&wait3_for_op);
        aux = nr_threads_remaining;
        sem_post(&wait3_for_op);
        if(aux == 6) {
            sem_wait(&sem3_T14_finished);
            sem_post(&sem3_T14_finished);
        }
        
    }

    sem_wait(&wait3_for_op);
    
    if(nr_threads_running == 6) sem_wait(&sem3_6_threads_running);
    --nr_threads_running;
    --nr_threads_remaining;
    
    sem_post(&wait3_for_op);
    info(END, 4, id);
    if(id == 14) sem_post(&sem3_T14_finished);
    sem_post(&sem3_limiter);
    
    
    
    return NULL;
}

void* thread_func4(void* arg){
    long id = (long) arg;

    if(id == 3) sem_wait(sem4_ended_T3_1);
    info(BEGIN, 2, id);
    
    info(END, 2, id);

    if(id == 4) sem_post(sem4_start_T3_1);

    return NULL;
}


int main(){
    init();

    info(BEGIN, 1, 0);
    pid_t pid1 = fork();
    switch (pid1) {
        case -1: // error case
            perror("Cannot create a new child");
            exit(1);
        case 0: 
            // child (P2)
            info(BEGIN, 2, 0);

            sem4_start_T3_1 = sem_open(SNAME_ST, O_CREAT, 0600, 0);
            sem4_ended_T3_1 = sem_open(SNAME_EN, O_CREAT, 0600, 0);
            sem_unlink(SNAME_ST);
            sem_unlink(SNAME_EN);
            pthread_t th_ids4[6];
            for(long i = 1; i < 6; ++i){ pthread_create( &th_ids4[i], NULL, thread_func4, (void*)i); }
            




            pid_t pid2 = fork();
            switch (pid2) {
                case -1: // error case
                    perror("Cannot create a new child");
                    exit(1);
                case 0: 
                    // child (P3)
                    info(BEGIN, 3, 0);

                    //sem4_start_T3_1 = sem_open(SNAME_ST, O_CREAT);
                    //sem4_ended_T3_1 = sem_open(SNAME_EN, O_CREAT);
                    sem_init(&sem2_start_T4, 0, 0);
                    sem_init(&sem2_end_T2, 0, 0);
                    sem_init(&wait_for_op, 0, 1);
                    
                    pthread_t th_ids2[5];
                    for(long i = 1; i < 5; ++i){ pthread_create( &th_ids2[i], NULL, thread_func2, (void*)i); }
                    for(long i = 1; i < 5; ++i){ pthread_join(th_ids2[i], NULL); }
                    





                    pid_t pid3 = fork();
                    switch (pid3) {
                        case -1: // error case
                            perror("Cannot create a new child");
                            exit(1);
                        case 0: 
                            // child (P4)
                            info(BEGIN, 4, 0);


                            nr_threads_remaining = 36;
                            nr_threads_running = 0;
                            sem_init(&wait3_for_op, 0, 1);
                            sem_init(&sem3_T14_finished, 0, 0);
                            sem_init(&sem3_limiter, 0, 6);
                            sem_init(&sem3_6_threads_running, 0, 0);

                            pthread_t th_ids3[37];
                            for(long i = 1; i < 37; ++i){ pthread_create( &th_ids3[i], NULL, thread_func3, (void*)i); }
                            for(long i = 1; i < 37; ++i){ pthread_join(th_ids3[i], NULL); }
                            





                            pid_t pid4 = fork();
                            switch (pid4) {
                                case -1: // error case
                                    perror("Cannot create a new child");
                                    exit(1);
                                case 0: 
                                    // child (P6)
                                    info(BEGIN, 6, 0);
                                    pid_t pid6 = fork();
                                    switch (pid6) {
                                        case -1: // error case
                                            perror("Cannot create a new child");
                                            exit(1);
                                        case 0: 
                                            // child (P8)
                                            info(BEGIN, 8, 0);
                                            
                                            info(END, 8, 0);
                                            break;
                                        default: // parent (P6) 
                                            waitpid(pid6, NULL, 0);
                                            info(END, 6, 0);
                                            break;
                                    }
                                    
                                    break;
                                default: // parent (P4)
                                    waitpid(pid4, NULL, 0);
                                    info(END, 4, 0);
                                    break;
                            }
                            
                            break;
                        default: // parent (P3) 
                            
                            ;pid_t pid5 = fork();
                            switch (pid5) {
                                case -1: // error case
                                    perror("Cannot create a new child");
                                    exit(1);
                                case 0: 
                                    // child (P5)
                                    info(BEGIN, 5, 0);
                                    info(END, 5, 0);
                                    break;
                                default: // parent (P3) 
                                    waitpid(pid5, NULL, 0);
                                    waitpid(pid3, NULL, 0);
                                    info(END, 3, 0);
                                    break;
                            }
                            break;
                    }
                    for(long i = 1; i < 6; ++i){ pthread_join(th_ids4[i], NULL); }
                    break;
                default: // parent (P2)
                    waitpid(pid2, NULL, 0);
                    info(END, 2, 0);
                    break;
            }
            
            break;
        default: 
            // parent (P1)
            ;pid_t pid7 = fork();
            switch (pid7) {
                case -1: // error case
                    perror("Cannot create a new child");
                    exit(1);
                case 0: 
                    // child (P7)
                    info(BEGIN, 7, 0);
                    info(END, 7, 0);
                    break;
                default: // parent (P1)
                    waitpid(pid7, NULL, 0);
                    waitpid(pid1, NULL, 0);
                    info(END, 1, 0);
                    break;
            }
            
            break;
    }
    
    
    return 0;
}
