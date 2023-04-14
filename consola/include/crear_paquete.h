#ifndef LEER_ARCHIVO_H_
#define LEER_ARCHIVO_H_

#include <stdlib.h>
#include <commons/log.h>
#include <commons/string.h>
#include "../../shared/include/estructuras/instrucciones.h"
#include "../../shared/include/sockets.h"

extern t_log *logger;

t_paquete *crear_paquete_instrucciones(FILE *f, uint32_t tamanio);

#endif
