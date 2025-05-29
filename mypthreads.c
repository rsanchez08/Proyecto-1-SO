// mypthreads.c
#include "mypthreads.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#define MAX_THREADS 128
#define DEBUG_PRINT(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)

static my_thread_t *thread_queue = NULL;
static my_thread_t *current_thread = NULL;
static ucontext_t main_context;
static int thread_id_counter = 0;

// Función envoltorio que asegura terminar el hilo
static void thread_wrapper(void (*start_routine)(void *), void *arg) {
    start_routine(arg);
    my_thread_end();
}

void enqueue_thread(my_thread_t *thread) {
    thread->next = NULL;
    if (!thread_queue) {
        thread_queue = thread;
    } else {
        my_thread_t *temp = thread_queue;
        while (temp->next) temp = temp->next;
        temp->next = thread;
    }
}

my_thread_t *dequeue_thread() {
    if (!thread_queue) return NULL;
    my_thread_t *t = thread_queue;
    thread_queue = thread_queue->next;
    return t;
}

int my_thread_create(my_thread_t **thread, void *attr, void *(*start_routine)(void *), void *arg) {
    *thread = (my_thread_t *)malloc(sizeof(my_thread_t));
    (*thread)->id = thread_id_counter++;
    (*thread)->state = READY;
    (*thread)->retval = NULL;
    getcontext(&(*thread)->context);
    (*thread)->context.uc_stack.ss_sp = malloc(STACK_SIZE);
    (*thread)->context.uc_stack.ss_size = STACK_SIZE;
    (*thread)->context.uc_stack.ss_flags = 0;
    (*thread)->context.uc_link = &main_context;
    makecontext(&(*thread)->context, (void (*)())thread_wrapper, 2, start_routine, arg);
    enqueue_thread(*thread);
    DEBUG_PRINT("Se creó el hilo con ID: %d", (*thread)->id);
    return 0;
}

void my_thread_start() {
    current_thread = dequeue_thread();
    if (current_thread) {
        current_thread->state = RUNNING;
        setcontext(&current_thread->context);
    }
}

void my_thread_yield() {
    if (!current_thread || current_thread->state != RUNNING) return;
    current_thread->state = READY;
    enqueue_thread(current_thread);
    my_thread_t *next = dequeue_thread();
    my_thread_t *prev = current_thread;
    current_thread = next;
    current_thread->state = RUNNING;
    swapcontext(&prev->context, &current_thread->context);
}

void my_thread_end() {
    current_thread->state = FINISHED;
    free(current_thread->context.uc_stack.ss_sp);
    my_thread_t *next = dequeue_thread();
    if (next) {
        current_thread = next;
        current_thread->state = RUNNING;
        setcontext(&current_thread->context);
    } else {
        setcontext(&main_context);
    }
}

int my_thread_join(my_thread_t *thread) {
    while (thread->state != FINISHED) {
        my_thread_yield();
    }
    return 0;
}

void my_mutex_init(my_mutex_t *mutex) {
    mutex->locked = 0;
    mutex->owner = NULL;
}

void my_mutex_destroy(my_mutex_t *mutex) {
    mutex->locked = 0;
    mutex->owner = NULL;
}

void my_mutex_lock(my_mutex_t *mutex) {
    while (__sync_lock_test_and_set(&mutex->locked, 1)) {
        my_thread_yield();
    }
    mutex->owner = current_thread;
}

void my_mutex_unlock(my_mutex_t *mutex) {
    if (mutex->owner == current_thread) {
        mutex->locked = 0;
        mutex->owner = NULL;
    }
}

void my_mutex_trylock(my_mutex_t *mutex) {
    if (__sync_lock_test_and_set(&mutex->locked, 1) == 0) {
        mutex->owner = current_thread;
    } else {
        my_thread_yield();
    }
}

void my_sleep(int seconds) {
    usleep(seconds * 1000000);
}
