#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <stdio.h>
#include <semaphore.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>


typedef struct{
    char* puerto;
    int tam_memoria;
    int tam_pagina;
    int entradas_por_tabla;
    int retardo_memoria;
    int marcos_por_proceso;
    char* algoritmo_reemplazo;
    int retardo_swap;
    char* path_swap;
} t_config_memoria;

t_log *memoria_log;
t_log *cpu_log;
t_log *kernel_log;

sem_t mutex_pf;
sem_t mutex_pid_actual;

void* memoria_principal;
//t_list* tablas;
t_list* tabla_planificacion;
t_config_memoria* configuracion;

typedef struct{
    int id;
    int pagina;
	int marco;
    bool P;
    bool U;
    bool M;
} t_tabla2;

int *planificacion_ptrs;
int page_fault_counter;
int pid_actual;

void iniciar_estructuras(char* config_path);
void cargar_configuraciones(char* path);
FILE* encontrar_SWAP(int id_proceso);
bool es_configuracion_valida(t_config *config_memoria);
t_list *agregar_proceso(int id_proceso, int tamanio_proceso, void* proceso);
#endif
