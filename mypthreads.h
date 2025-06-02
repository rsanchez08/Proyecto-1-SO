#ifndef MYPTHREADS_H
#define MYPTHREADS_H

#include <ucontext.h>

// Constantes
#define STACK_SIZE 8192

extern char mensaje_scheduler[100];

// Enumeración para los estados de los hilos
typedef enum { 
    READY, 
    RUNNING, 
    FINISHED 
} thread_state_t;

// Enumeración para los schedulers que hay
typedef enum {
    SCHED_RR,
    SCHED_LOTTERY,
    SCHED_RT
} scheduler_type_t;

// Struct de un hilo
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

// Struct de un mutex
typedef struct {
    int locked;
    my_thread_t *owner;
} my_mutex_t;

// Funciones de hilos
int my_thread_create(my_thread_t **thread, void *attr, void *(*start_routine)(void *), void *arg, scheduler_type_t scheduler, int extra);
void my_thread_start();
int my_thread_yield();
int my_thread_end();
int my_thread_join(my_thread_t *thread);
int my_thread_chsched(my_thread_t *thread, scheduler_type_t new_scheduler, int extra_param);

// Mutex
int my_mutex_init(my_mutex_t *mutex);
int my_mutex_destroy(my_mutex_t *mutex);
int my_mutex_lock(my_mutex_t *mutex);
int my_mutex_unlock(my_mutex_t *mutex);
int my_mutex_trylock(my_mutex_t *mutex);

// Utilidad
void my_sleep(int seconds);

#endif
