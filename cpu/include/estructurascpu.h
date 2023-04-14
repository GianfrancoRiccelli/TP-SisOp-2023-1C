#ifndef ESTRUCTURASCPU_H_
#define ESTRUCTURASCPU_H_

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <semaphore.h>

#include <commons/config.h>
#include <commons/log.h>

struct t_config_cpu {
    int entradas_tlb;
    char* reemplazo_tlb;
    int retardo_noop;
    char* ip_memoria;
    char* puerto_memoria;
    char* puerto_escucha_dispatch;
    char* puerto_escucha_interrupt;
} cpuconfig;

t_log *cpu_log;
t_log *memoria_log;
t_log *interrupt_log;

int* entradas_por_tabla;
int* tam_de_pagina;

int socket_dispatch;
int socket_interrupt;
int socket_memoria;

sem_t mutex_interrupt;
bool check_interrupt;

t_config *cargar_config(char* path);

#endif
