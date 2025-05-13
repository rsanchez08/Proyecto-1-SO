#ifndef MY_PTHREADS_H
#define MY_PTHREADS_H

#include <ucontext.h>

// Tipos de schedulers
#define SCHED_RR 0
#define SCHED_SORTEO 1
#define SCHED_REAL 2

// Tipos de mutex
#define MY_MUTEX_NORMAL 0
#define MY_MUTEX_RECURSIVE 1
#define MY_MUTEX_ERRORCHECK 2


// Enumeración de los estados posibles de un hilo
typedef enum { 
    READY,     // Hilo listo para ejecutar
    RUNNING,   // Hilo en ejecución actual
    BLOCKED    // Hilo esperando 
} thread_state;

// Struct del hilo 
typedef struct my_pthread {
    ucontext_t context;            // Contexto de ejecución
    void* stack;                   // Stack del hilo
    thread_state state;            // Estado actual
    int scheduler_type;            // Tipo de scheduler (RR, Sorteo, etc.)
    void* (*start_routine)(void*); // Función a ejecutar
    void* arg;                     // Argumentos para la función
    int deadline_ms;               // Límite de tiempo (opcional para Tiempo Real)
} *my_pthread_t;

// Atributos para los hilos
typedef struct {
    size_t stack_size;
    int is_detached;
    int default_scheduler;
} my_pthread_attr_t;

// Struct del mutex
typedef struct {
    volatile int lock;
    my_pthread_t owner;
    int type;
    int count;
} my_pthread_mutex_t;

// Struct de los atributos del mutex
typedef struct {
    int type;    // MY_MUTEX_NORMAL, RECURSIVE, ERRORCHECK.
    int pshared; // MY_PROCESS_PRIVATE o MY_PROCESS_SHARED
} my_pthread_mutexattr_t;

// Definición de las funciones de la bilioteca myphtreads
// Las de hilos
int my_thread_create(my_pthread_t *thread,                    // Función para crear un hilo 
                     const my_pthread_attr_t *attr,
                     void *(*start_routine)(void *),
                     void *arg, 
                     int scheduler_type, 
                     int tickets_or_deadline); 
void my_thread_exit(void *retval);                            // Función para terminar un hilo
int my_thread_yield(void);                                    // Función para darle permiso a otros hilos de correr
int my_thread_join(my_pthread_t thread, void **value_ptr);    // Función que espera que un hilo termine 
int my_thread_detach(my_pthread_t thread);                    // Función para desligar un hilo 
int my_thread_chsched(my_pthread_t thread,                    // Función que se encarga de cambiar el tipo de scheduling del hilo
                      int new_scheduler, 
                      int param);                   

// Las de mutex
int my_mutex_init(my_pthread_mutex_t *mutex,                  // Función para inicializar el objeto mutex
                  const my_pthread_mutexattr_t *mutexattr);
int my_mutex_destroy(my_pthread_mutex_t *mutex);              // Función para destruir un objeto mutex
int my_mutex_lock(my_pthread_mutex_t *mutex);                 // Función que bloquea un objeto mutex
int my_mutex_unlock(my_pthread_mutex_t *mutex);               // Función que desbloquea un objeto mutex
int my_mutex_trylock(my_pthread_mutex_t *mutex);              // Mismo comportamiento de lock, solo que no bloquea el hilo si el mutex ya está siendo bloqueado por otro hilo

#endif