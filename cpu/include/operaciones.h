#ifndef OPERACIONES_H_
#define OPERACIONES_H_

#include "../../shared/include/sockets.h"
#include "../../shared/include/conexion_memoria.h"
#include <math.h>
#include "estructurascpu.h"
#include "tlb.h"

int obtener_direccion_fisica(int tabla_paginas, uint32_t direccion_logica, int entradas_por_tabla, int tam_pagina);
bool pedir_escritura(int tam_escribir, void* dato_a_escribir, int direccion_fisica);
void* pedir_lectura(int tam_leer, int direccion_fisica);
#endif
