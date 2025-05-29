#ifndef MYPTHREADS_H
#define MYPTHREADS_H

#include <ucontext.h>
#define STACK_SIZE 1024*64

typedef enum { READY, RUNNING, FINISHED } thread_state_t;

typedef struct my_thread {
    int id;
    ucontext_t context;
    thread_state_t state;
    void *retval;
    struct my_thread *next;
} my_thread_t;

typedef struct {
    int locked;
    my_thread_t *owner;
} my_mutex_t;

int my_thread_create(my_thread_t **thread, void *attr, void *(*start_routine)(void *), void *arg);
void my_thread_start();
void my_thread_yield();
void my_thread_end();
int my_thread_join(my_thread_t *thread);

void my_mutex_init(my_mutex_t *mutex);
void my_mutex_destroy(my_mutex_t *mutex);
void my_mutex_lock(my_mutex_t *mutex);
void my_mutex_unlock(my_mutex_t *mutex);
void my_mutex_trylock(my_mutex_t *mutex);

void my_sleep(int seconds);

#endif
