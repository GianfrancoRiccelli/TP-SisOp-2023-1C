#ifndef CONEXION_CON_MEMORIA_H_
#define CONEXION_CON_MEMORIA_H_

#include <commons/log.h>
#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include "../../shared/include/sockets.h"
#include "../../shared/include/conexion_memoria.h"
#include "colas.h"

int socket_memoria;
extern t_log *logger;
t_log* memoria_log;


void conectar_memoria(char *ip, char *puerto);
void enviar_mensaje_memoria(void *mensaje, int tam);
void *recibir_mensaje_memoria();
void desconectar_memoria();

int agregar_proceso_memoria(int pid, int tam_proceso);
bool suspender_proceso_memoria(int direccion_tabla, int pid);
bool finalizar_proceso_memoria(int direccion_tabla);

#endif /* CONEXION_MEMORIA_H_ */
