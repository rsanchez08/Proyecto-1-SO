#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mypthreads.h"

#define CANVAS_WIDTH 80
#define CANVAS_HEIGHT 20
char canvas[CANVAS_HEIGHT][CANVAS_WIDTH + 1];

char *estrella[] = {
    "  *  ",
    " *** ",
    "*****",
    " *** ",
    "  *  "
};

#define ESTRELLA_HEIGHT 5
#define ESTRELLA_WIDTH 5

typedef struct {
    int x, y;
    int dx, dy;
    int tiempo_inicio;
    int tiempo_final;
    int tiempo_actual;
} Objeto;

Objeto objeto = {0, 5, 1, 0, 2, 8, 0};

my_mutex_t canvas_mutex;

void dibujar_objeto(int x, int y) {
    for (int i = 0; i < ESTRELLA_HEIGHT; i++) {
        for (int j = 0; j < ESTRELLA_WIDTH; j++) {
            char c = estrella[i][j];
            if (c != ' ') {
                int cx = x + j;
                int cy = y + i;
                if (cx >= 0 && cx < CANVAS_WIDTH && cy >= 0 && cy < CANVAS_HEIGHT) {
                    canvas[cy][cx] = c;
                }
            }
        }
    }
}

void borrar_objeto(int x, int y) {
    for (int i = 0; i < ESTRELLA_HEIGHT; i++) {
        for (int j = 0; j < ESTRELLA_WIDTH; j++) {
            int cx = x + j;
            int cy = y + i;
            if (cx >= 0 && cx < CANVAS_WIDTH && cy >= 0 && cy < CANVAS_HEIGHT) {
                canvas[cy][cx] = '.';
            }
        }
    }
}

void mostrar_canvas() {
    system("clear");
    for (int i = 0; i < CANVAS_HEIGHT; i++) {
        printf("%s\n", canvas[i]);
    }
}

void *animar_estrella(void *arg) {
    printf("Hilo iniciado\n");
    fflush(stdout);

    my_sleep(objeto.tiempo_inicio);

    printf("Comenzando animación...\n");
    fflush(stdout);

    for (int t = objeto.tiempo_inicio; t <= objeto.tiempo_final; t++) {
        my_mutex_lock(&canvas_mutex);
        borrar_objeto(objeto.x, objeto.y);

        objeto.x += objeto.dx;
        objeto.y += objeto.dy;

        dibujar_objeto(objeto.x, objeto.y);
        mostrar_canvas();

        my_mutex_unlock(&canvas_mutex);

        my_sleep(1);
        objeto.tiempo_actual++;

        if (objeto.tiempo_actual > objeto.tiempo_final - objeto.tiempo_inicio) {
            printf("\n\nBOOM! El objeto explotó por llegar tarde.\n");
            break;
        }
    }

    my_thread_end();
    return NULL;
}

void inicializar_canvas() {
    for (int i = 0; i < CANVAS_HEIGHT; i++) {
        for (int j = 0; j < CANVAS_WIDTH; j++) {
            canvas[i][j] = '.';
        }
        canvas[i][CANVAS_WIDTH] = '\0';
    }
    printf("Canvas inicializado\n");
}

int main() {
    my_mutex_init(&canvas_mutex);
    inicializar_canvas();

    my_thread_t *hilo;
    my_thread_create(&hilo, NULL, animar_estrella, NULL);
    printf("Hilo creado con ID: %d\n", hilo->id);

    my_thread_start();  // <- Inicia ejecución

    my_mutex_destroy(&canvas_mutex);
    return 0;
}
