// mypthreads.h
#ifndef MYPTHREADS_H
#define MYPTHREADS_H

#include <ucontext.h>

#define STACK_SIZE 8192

extern char mensaje_scheduler[100];

typedef enum { 
    READY, 
    RUNNING, 
    FINISHED 
} thread_state_t;

typedef enum {
    SCHED_RR,
    SCHED_LOTTERY,
    SCHED_RT
} scheduler_type_t;

typedef struct my_thread {
    int id;
    ucontext_t context;
    thread_state_t state;
    void *retval;
    struct my_thread *next;
    scheduler_type_t scheduler;
    int tickets;   // solo para LOTTERY
    int deadline;  // solo para RT
} my_thread_t;

typedef struct {
    int locked;
    my_thread_t *owner;
} my_mutex_t;

// Funciones principales
int my_thread_create(my_thread_t **thread, void *attr, void *(*start_routine)(void *), void *arg, scheduler_type_t scheduler, int extra);
void my_thread_start();
void my_thread_yield();
void my_thread_end();
int my_thread_join(my_thread_t *thread);

// Mutex
void my_mutex_init(my_mutex_t *mutex);
void my_mutex_destroy(my_mutex_t *mutex);
void my_mutex_lock(my_mutex_t *mutex);
void my_mutex_unlock(my_mutex_t *mutex);
void my_mutex_trylock(my_mutex_t *mutex);

// Utilidad
void my_sleep(int seconds);

#endif
