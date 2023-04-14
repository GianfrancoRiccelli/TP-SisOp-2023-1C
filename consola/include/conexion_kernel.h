#ifndef EMPAQUETAR_H_
#define EMPAQUETAR_H_

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include <commons/log.h>
#include <commons/config.h>

#include "../../shared/include/sockets.h"

extern t_log *logger;

int conectar_a_kernel();
bool conexion_exitosa(int);
void enviar_paquete_instrucciones(t_paquete *paquete, int socket_kernel);
bool terminar(int);

#endif
