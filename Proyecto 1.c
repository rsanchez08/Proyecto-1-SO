#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#define STACK_SIZE (1024 * 1024) // Tamaño de pila para cada hilo
#define MAX_THREADS 64           // Máximo número de hilos

// Estados de un hilo
typedef enum {
    READY,
    RUNNING,
    BLOCKED,
    FINISHED
} thread_state;

// Estructura para representar un hilo
typedef struct {
    ucontext_t context;      // Contexto del hilo
    thread_state state;      // Estado actual
    int id;                  // Identificador único
    void *(*function)(void*);// Función a ejecutar
    void *arg;               // Argumentos de la función
    void *retval;            // Valor de retorno
    char stack[STACK_SIZE];  // Pila del hilo
} my_thread;

// Estructura para la cola de hilos listos
typedef struct {
    my_thread *threads[MAX_THREADS];
    int front, rear;
} thread_queue;

// Variables globales
my_thread *current_thread = NULL;
thread_queue ready_queue;
ucontext_t scheduler_context;
int thread_id_counter = 0;

// Inicializar la cola de hilos listos
void init_queue(thread_queue *q) {
    q->front = q->rear = -1;
}

// Añadir hilo a la cola
void enqueue(thread_queue *q, my_thread *thread) {
    if (q->rear == MAX_THREADS - 1) {
        fprintf(stderr, "Error: Cola de hilos llena\n");
        exit(EXIT_FAILURE);
    }
    
    if (q->front == -1) {
        q->front = 0;
    }
    
    q->rear++;
    q->threads[q->rear] = thread;
}

// Sacar hilo de la cola
my_thread *dequeue(thread_queue *q) {
    if (q->front == -1) {
        return NULL;
    }
    
    my_thread *thread = q->threads[q->front];
    
    if (q->front == q->rear) {
        q->front = q->rear = -1;
    } else {
        q->front++;
    }
    
    return thread;
}

// Función wrapper para ejecutar el hilo
void thread_wrapper(my_thread *thread) {
    thread->retval = thread->function(thread->arg);
    thread->state = FINISHED;
    
    // Cambiar al scheduler
    swapcontext(&thread->context, &scheduler_context);
}

// Crear un nuevo hilo
int my_thread_create(my_thread *thread, void *(*start_routine)(void*), void *arg) {
    if (thread_id_counter >= MAX_THREADS) {
        fprintf(stderr, "Error: Máximo número de hilos alcanzado\n");
        return -1;
    }
    
    // Inicializar el contexto
    if (getcontext(&thread->context) == -1) {
        perror("getcontext");
        return -1;
    }
    
    // Configurar el contexto
    thread->context.uc_stack.ss_sp = thread->stack;
    thread->context.uc_stack.ss_size = sizeof(thread->stack);
    thread->context.uc_link = &scheduler_context;
    
    thread->id = thread_id_counter++;
    thread->function = start_routine;
    thread->arg = arg;
    thread->state = READY;
    thread->retval = NULL;
    
    // Crear el contexto con la función wrapper
    makecontext(&thread->context, (void (*)(void))thread_wrapper, 1, thread);
    
    // Añadir a la cola de listos
    enqueue(&ready_queue, thread);
    
    return 0;
}

// Ceder el control a otro hilo
void my_thread_yield() {
    if (current_thread != NULL) {
        current_thread->state = READY;
        enqueue(&ready_queue, current_thread);
    }
    
    // Cambiar al scheduler
    swapcontext(&current_thread->context, &scheduler_context);
}

// Planificador Round Robin
void scheduler() {
    while (1) {
        my_thread *next_thread = dequeue(&ready_queue);
        
        if (next_thread == NULL) {
            // No hay hilos listos, salir
            break;
        }
        
        next_thread->state = RUNNING;
        current_thread = next_thread;
        
        // Cambiar al hilo seleccionado
        swapcontext(&scheduler_context, &next_thread->context);
        
        // El hilo ha vuelto al scheduler
        if (current_thread->state == FINISHED) {
            // Liberar recursos del hilo terminado
            // (En una implementación real habría que hacer más limpieza)
            current_thread = NULL;
        }
    }
    
    // Todos los hilos han terminado
    exit(EXIT_SUCCESS);
}

// Función de ejemplo para un hilo
void *thread_function(void *arg) {
    int id = *(int *)arg;
    
    for (int i = 0; i < 5; i++) {
        printf("Hilo %d: iteración %d\n", id, i);
        my_thread_yield();
    }
    
    return NULL;
}

int main() {
    // Inicializar la cola de hilos listos
    init_queue(&ready_queue);
    
    // Configurar el contexto del scheduler
    if (getcontext(&scheduler_context) == -1) {
        perror("getcontext");
        return EXIT_FAILURE;
    }
    
    // Crear algunos hilos de prueba
    my_thread threads[3];
    int ids[3] = {1, 2, 3};
    
    for (int i = 0; i < 3; i++) {
        if (my_thread_create(&threads[i], thread_function, &ids[i]) != 0) {
            fprintf(stderr, "Error al crear el hilo %d\n", i);
            return EXIT_FAILURE;
        }
    }
    
    // Iniciar el scheduler
    scheduler();
    
    return 0;
}