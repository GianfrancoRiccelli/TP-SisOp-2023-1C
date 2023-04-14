#ifndef CONEXION_CPU_H_
#define CONEXION_CPU_H_

#include <sys/socket.h>
#include "../../shared/include/sockets.h"
#include "../../shared/include/estructuras/pcb.h"
#include "../../shared/include/estructuras/instrucciones.h"
#include <commons/log.h>

#include "colas.h"

t_log* kernel_cpu_log;
extern t_log* logger;
int socket_dispatch;
int socket_interrupt;

void conectar_cpu(char* ip, char* puerto_dispatch, char* puerto_interrupt);

#endif
