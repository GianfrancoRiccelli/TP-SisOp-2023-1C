#ifndef LEER_CONFIG_H_
#define LEER_CONFIG_H_

#include <commons/config.h>

extern t_config *config;

void crear_config(char* config_path);
void destruir_config();

// memoria
char *ip_memoria();
char *puerto_memoria();

// cpu
char *ip_cpu();
char *puerto_cpu_dispatch();
char *puerto_cpu_interrupt();

// kernel
char *puerto_escucha();

// planificación
char *algoritmo_planificacion();
int grado_multiprogramacion();
double tiempo_max_bloqueado();

// sjf
double estimacion_inicial();
double alfa();

#endif /* LEER_CONFIG_H_ */
