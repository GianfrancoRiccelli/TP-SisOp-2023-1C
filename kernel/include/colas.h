#ifndef INCLUDE_COLAS_H_
#define INCLUDE_COLAS_H_
#define _GNU_SOURCE

#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/string.h>
#include <sys/time.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>


#include "../../shared/include/estructuras/pcb.h"
#include "conexion_memoria.h"
#include "leer_config.h"
#include "kernel.h"


int new_counter, suspendido_counter, id_counter;
extern t_log* log_kernel;
bool interrupcion;

//Colas
t_list *ready;
t_queue *new_queue, *exit_queue, *bloqueado_queue, *bloqueado_tiempo,
*suspendido_bloqueado, *suspendido_tiempo, *suspendido_ready;

t_pcb* pcb_en_execute;

//Semaforos Mutex
sem_t mnew, mnew_counter, mid_counter, mready, mexit, msuspendido_counter, mbloqueado,
mbloqueado_tiempo, msuspendido_bloqueado, msuspendido_tiempo,
msuspendido_ready;
sem_t mlog;

//Semaforos
sem_t nivel_multiprogramacion; //Inicializa en el que sea el nivel de multiprogramacion
sem_t procesos_en_ready; //Inicializa en 0
sem_t ready_disponible; //Inicializa en 0
sem_t procesos_en_exit; //Inicializa en 0
sem_t bloqueado; //Inicializa en 0
sem_t suspendido;



void inicializar_estructuras();
int inicializar_threads();
void log_protegido(char* mensaje);
void* decrementador_cola_bloqueado(void* arg);
void agregar_a_new(t_pcb *pcb);
void *thread_ready();
void *thread_execute();
void *thread_blocked();
void *thread_suspendido_blocked();
void *thread_exit();

#endif /* INCLUDE_COLAS_H_ */
