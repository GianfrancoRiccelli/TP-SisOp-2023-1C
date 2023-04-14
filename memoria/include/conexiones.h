#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include "../../shared/include/sockets.h"
#include "../../shared/include/conexion_memoria.h"
#include "reemplazo.h"
#include "estructuras.h"
#include <semaphore.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

int iniciar_conexiones();
void *conectar_con_cpu(int* socket_cpu);
void *conectar_con_kernel(int* socket_cpu);

#endif
