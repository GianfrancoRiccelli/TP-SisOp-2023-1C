#ifndef INSTRUCCIONESCPU_H_
#define INSTRUCCIONESCPU_H_

#include "conexionescpu.h"
#include "operaciones.h"
#include <math.h>
#include "../../shared/include/estructuras/instrucciones.h"
#include "tlb.h"

void ciclo_de_instruccion(t_pcb* pcb);

#endif