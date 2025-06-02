#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mypthreads.h"

#define CANVAS_WIDTH 50
#define CANVAS_HEIGHT 50

char canvas[CANVAS_HEIGHT][CANVAS_WIDTH];
my_mutex_t canvas_mutex; //Mutex para asegurar que solo un hilo acceda al canvas a la vez. 

typedef struct {
    int x, y;               // posición actual
    int dx, dy;             // dirección
    int tiempo_inicio;      // segundos antes de comenzar
    int tiempo_final;       // segundos para finalizar
    int tiempo_actual;      // contador interno
    int id;                 // número único del objeto
} Objeto;

void inicializar_canvas() {
    for (int i = 0; i < CANVAS_HEIGHT; i++) {
        for (int j = 0; j < CANVAS_WIDTH; j++) {
            canvas[i][j] = '.'; //Rellena el canvas con puntos que representa el espacio vacío 
        }
    }
    printf("Canvas inicializado\n");
}

void imprimir_canvas() {
    system("clear"); //Limpia para que se vea la animación fluida. 
    for (int i = 0; i < CANVAS_HEIGHT; i++) {
        for (int j = 0; j < CANVAS_WIDTH; j++) {
            putchar(canvas[i][j]);
        }
        putchar('\n');
    }

    // Mostrar el estado del scheduler debajo del canvas
    printf("\n\n%s\n", mensaje_scheduler);
}


void dibujar_estrella(Objeto *obj) {
    char c = '0' + obj->id; //Caracter que repreetna el objeto. 
    my_mutex_lock(&canvas_mutex); //Bloquea acceso exclusivo al canvas. 
    int x = obj->x, y = obj->y;

    if (x >= 2 && x < CANVAS_WIDTH - 2 && y >= 2 && y < CANVAS_HEIGHT - 2) {
        canvas[y - 2][x] = c;
        canvas[y - 1][x - 1] = c;
        canvas[y - 1][x] = c;
        canvas[y - 1][x + 1] = c;
        canvas[y][x - 2] = c;
        canvas[y][x - 1] = c;
        canvas[y][x] = c;
        canvas[y][x + 1] = c;
        canvas[y][x + 2] = c;
        canvas[y + 1][x - 1] = c;
        canvas[y + 1][x] = c;
        canvas[y + 1][x + 1] = c;
        canvas[y + 2][x] = c;
    }
    imprimir_canvas();
    my_mutex_unlock(&canvas_mutex);
}


void borrar_estrella(Objeto *obj) {
    my_mutex_lock(&canvas_mutex);
    int x = obj->x, y = obj->y;

    if (x >= 2 && x < CANVAS_WIDTH - 2 && y >= 2 && y < CANVAS_HEIGHT - 2) {
        canvas[y - 2][x] = '.';
        canvas[y - 1][x - 1] = '.';
        canvas[y - 1][x] = '.';
        canvas[y - 1][x + 1] = '.';
        canvas[y][x - 2] = '.';
        canvas[y][x - 1] = '.';
        canvas[y][x] = '.';
        canvas[y][x + 1] = '.';
        canvas[y][x + 2] = '.';
        canvas[y + 1][x - 1] = '.';
        canvas[y + 1][x] = '.';
        canvas[y + 1][x + 1] = '.';
        canvas[y + 2][x] = '.';
    }
    my_mutex_unlock(&canvas_mutex);
}


void animar_objeto(void *arg) {
    Objeto *obj = (Objeto *)arg;

    my_sleep(obj->tiempo_inicio); // Espera el tiempo inicial antes de comenzar la animación

    for (int i = 0; i < obj->tiempo_final; i++) {
        borrar_estrella(obj); // Borra la estrella en la posición anterior
        obj->x += obj->dx; // Actualiza la posición
        obj->y += obj->dy;
        dibujar_estrella(obj); // Dibuja la estrella en la nueva posición
        obj->tiempo_actual++;
        my_sleep(1); // Espera 1 segundo entre movimientos
        my_thread_yield(); // Cede el control al scheduler (permite que otros hilos avancen)
    }

    borrar_estrella(obj); // Borra la estrella al finalizar su animación

    my_mutex_lock(&canvas_mutex);
    if (obj->x >= 0 && obj->x < CANVAS_WIDTH && obj->y >= 0 && obj->y < CANVAS_HEIGHT) {
        strcpy(&canvas[obj->y][obj->x], "BOOM");
    }
    imprimir_canvas();
    my_mutex_unlock(&canvas_mutex);
}

int main() {
    my_mutex_init(&canvas_mutex);
    inicializar_canvas();

    Objeto objetos[9] = {
        {0, 6, 1, 0, 1, 8, 0, 0},   // Round Robin
        {0, 11, 1, 0, 1, 9, 0, 1},
        {0, 16, 1, 0, 1, 10, 0, 2},

        {0, 21, 1, 0, 1, 8, 0, 3},   // Lottery
        {0, 26, 1, 0, 1, 9, 0, 4},
        {0, 31, 1, 0, 1, 10, 0, 5},

        {0, 36, 1, 0, 1, 5, 0, 6},  // Real-Time
        {0, 41, 1, 0, 1, 6, 0, 7},
        {0, 46, 1, 0, 1, 7, 0, 8}
    };

    my_thread_t *threads[9];

    for (int i = 0; i < 3; i++)
        my_thread_create(&threads[i], NULL, animar_objeto, &objetos[i], SCHED_RR, 0);

    for (int i = 3; i < 6; i++)
        my_thread_create(&threads[i], NULL, animar_objeto, &objetos[i], SCHED_LOTTERY, 5 + i);

    for (int i = 6; i < 9; i++)
        my_thread_create(&threads[i], NULL, animar_objeto, &objetos[i], SCHED_RT, objetos[i].tiempo_final);

    my_thread_start(); // Inicia la ejecución de todos los hilos

    return 0;
}
