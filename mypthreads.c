#include "mypthreads.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

#define MAX_THREADS 128
#define DEBUG_PRINT(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)

char mensaje_scheduler[100] = "Esperando hilos...";

static my_thread_t *rr_queue = NULL;
static my_thread_t *lottery_queue = NULL;
static my_thread_t *rt_queue = NULL;

static my_thread_t *current_thread = NULL;
static ucontext_t main_context;
static int thread_id_counter = 0;

static void thread_wrapper(void (*start_routine)(void *), void *arg) {
    start_routine(arg);
    my_thread_end();
}

void enqueue_thread(my_thread_t **queue, my_thread_t *thread) {
    thread->next = NULL;
    if (!*queue) {
        *queue = thread;
    } else {
        my_thread_t *temp = *queue;
        while (temp->next) temp = temp->next;
        temp->next = thread;
    }
}

my_thread_t *dequeue_rr() {
    if (!rr_queue) return NULL;
    my_thread_t *t = rr_queue;
    rr_queue = rr_queue->next;
    return t;
}

my_thread_t *dequeue_lottery() {
    if (!lottery_queue) return NULL;

    int total_tickets = 0;
    for (my_thread_t *t = lottery_queue; t != NULL; t = t->next) {
        total_tickets += t->tickets;
    }

    if (total_tickets == 0) return NULL;

    int winning_ticket = rand() % total_tickets;
    my_thread_t *prev = NULL;
    my_thread_t *curr = lottery_queue;
    while (curr) {
        if (winning_ticket < curr->tickets) {
            if (prev)
                prev->next = curr->next;
            else
                lottery_queue = curr->next;
            return curr;
        }
        winning_ticket -= curr->tickets;
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

my_thread_t *dequeue_rt() {
    if (!rt_queue) return NULL;
    my_thread_t *prev = NULL;
    my_thread_t *min_prev = NULL;
    my_thread_t *min_node = rt_queue;
    int min_deadline = min_node->deadline;

    my_thread_t *curr = rt_queue;
    while (curr->next) {
        if (curr->next->deadline < min_deadline) {
            min_prev = curr;
            min_node = curr->next;
            min_deadline = curr->next->deadline;
        }
        curr = curr->next;
    }

    if (min_prev) {
        min_prev->next = min_node->next;
    } else {
        rt_queue = min_node->next;
    }

    return min_node;
}

my_thread_t *scheduler_dispatch() {
    if (rt_queue) {
        my_thread_t *t = dequeue_rt();
        snprintf(mensaje_scheduler, sizeof(mensaje_scheduler), "[Scheduler: Tiempo Reeal] Ejecutando hilo ID: %d", t->id);
        return t;
    }
    if (lottery_queue) {
        my_thread_t *t = dequeue_lottery();
        snprintf(mensaje_scheduler, sizeof(mensaje_scheduler), "[Scheduler: Sorteo] Ejecutando hilo ID: %d", t->id);
        return t;
    }
    if (rr_queue) {
        my_thread_t *t = dequeue_rr();
        snprintf(mensaje_scheduler, sizeof(mensaje_scheduler), "[Scheduler: Round Robin] Ejecutando hilo ID: %d", t->id);
        return t;
    }
    return NULL;
}

int my_thread_create(my_thread_t **thread, void *attr, void *(*start_routine)(void *), void *arg, scheduler_type_t scheduler, int extra) {
    *thread = (my_thread_t *)malloc(sizeof(my_thread_t));
    (*thread)->id = thread_id_counter++;
    (*thread)->state = READY;
    (*thread)->retval = NULL;
    (*thread)->scheduler = scheduler;
    (*thread)->tickets = (scheduler == SCHED_LOTTERY) ? extra : 0;
    (*thread)->deadline = (scheduler == SCHED_RT) ? extra : 0;

    getcontext(&(*thread)->context);
    (*thread)->context.uc_stack.ss_sp = malloc(STACK_SIZE);
    (*thread)->context.uc_stack.ss_size = STACK_SIZE;
    (*thread)->context.uc_stack.ss_flags = 0;
    (*thread)->context.uc_link = &main_context;
    makecontext(&(*thread)->context, (void (*)())thread_wrapper, 2, start_routine, arg);

    if (scheduler == SCHED_RR)
        enqueue_thread(&rr_queue, *thread);
    else if (scheduler == SCHED_LOTTERY)
        enqueue_thread(&lottery_queue, *thread);
    else
        enqueue_thread(&rt_queue, *thread);

    DEBUG_PRINT("Se creó el hilo con ID: %d", (*thread)->id);
    return 0;
}

void my_thread_start() {
    srand(time(NULL));
    current_thread = scheduler_dispatch();
    if (current_thread) {
        current_thread->state = RUNNING;
        setcontext(&current_thread->context);
    }
}

void my_thread_yield() {
    if (!current_thread || current_thread->state != RUNNING) return;
    current_thread->state = READY;

    if (current_thread->scheduler == SCHED_RR)
        enqueue_thread(&rr_queue, current_thread);
    else if (current_thread->scheduler == SCHED_LOTTERY)
        enqueue_thread(&lottery_queue, current_thread);
    else
        enqueue_thread(&rt_queue, current_thread);

    my_thread_t *next = scheduler_dispatch();
    if (!next) return;
    my_thread_t *prev = current_thread;
    current_thread = next;
    current_thread->state = RUNNING;
    swapcontext(&prev->context, &current_thread->context);
}

void my_thread_end() {
    current_thread->state = FINISHED;
    free(current_thread->context.uc_stack.ss_sp);
    my_thread_t *next = scheduler_dispatch();
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
