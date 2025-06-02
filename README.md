# 🧠 Proyecto-1 - Principios de Sistemas Operativos

Repositorio para el control de versiones del **Proyecto 1** del curso **Principios de Sistemas Operativos**.

**Autor:** René Sánchez Torres y Jose Andrés Vargas Serrano

**Carné:** 2020051805 y 2019211290

**Institución:** Tecnológico de Costa Rica

---

## 🛠️ Descripción
El proyecto MultiDisplay Animator consiste en un sistema distribuido para crear animaciones ASCII que se despliegan en múltiples monitores. Una biblioteca personalizada de hilos mypthreads con tres schedulers:
        Round Robin: Turnos iguales
        Sorteo (Lottery): Sorteo con "tickets", entre más tickets, más probablidad. 
        Tiempo Real: Prioridad al que tiene menos tiempo para terminar.
 
Además usamos mecanismo mutex para permitir a un solo hilo a la vez acceder a una sección crítica del código. Esto en este proyecto se utiliza principalmente para evitar que varios hilos dibujen al mismo tiempo en el canvas.

El objetivo principal es comprender los principios de los sistemas operativos mediante la implementación de estos componentes en espacio de usuario.
La tarea propuesta busca simular este escenario mediante la construcción de un WebServer que utilice el protocolo HTTP 1.1 y sea capaz de responder a solicitudes utilizando: pre-threaded. Estos modelos permiten manejar múltiples conexiones mediante la creación anticipada de hilos. Además, se desarrollará un cliente HTTP capaz de interactuar con este servidor, y una herramienta de stress testing con el fin de saturarlo y evaluar su comportamiento bajo carga extrema, simulando un ataque.

---

## 🚀 Compilación

```bash
gcc -o animar Proyecto1.c mypthreads.c -Wall```
```

## 🦾 EJECUCIÓN 
```bash
./animar
```
