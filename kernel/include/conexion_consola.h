#ifndef COLAS_H_
#define COLAS_H_
#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

#include <commons/log.h>
#include <commons/collections/queue.h>

#include <sys/socket.h>
#include "../../shared/include/estructuras/pcb.h"
#include "colas.h"
#include "conexion_memoria.h"

extern t_log *log_kernel;
extern sem_t mutex_memoria, mutex_new, procesos_en_new;
extern t_queue *queue_new;

int pid_counter;

void conectar_consola(int socket_kernel);
t_pcb *generar_pcb(int socket_cliente);
void proceso_new(int *socket_cliente);
void terminar_consola(int);

#endif /* COLAS_H_ */
