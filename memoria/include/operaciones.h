#ifndef OPERACIONES_H_
#define OPERACIONES_H_
#include "estructuras.h"
#include <stdint.h>
#include <string.h>
#include <semaphore.h>
#include "reemplazo.h"

sem_t mutex_tabla_planificacion;

void suspender_proceso(int id_proceso);
void suspender_proceso2(t_list* direccion);
void finalizar_proceso(t_list *direccion);
t_list *obtener_tabla2(t_list *direccion, int indice);
int obtener_marco(t_list *direccion, int indice);
void escribir_a_memoria(int direccion, int tamanio_a_escribir, void* a_escribir);
void* leer_de_memoria(int direccion, int tamanio_a_leer);
void leer_pagina_SWAP(int id_proceso, int pagina, int marco);
void escribir_pagina_SWAP(int id_proceso, int pagina, int marco);

#endif
