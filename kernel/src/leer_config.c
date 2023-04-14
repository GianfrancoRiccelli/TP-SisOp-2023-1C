#include "../include/leer_config.h"

void crear_config(char* config_path) {
	config = config_create(config_path);
}

void destruir_config() {
	config_destroy(config);
}

// memoria
char *ip_memoria() {
	return config_get_string_value(config, "IP_MEMORIA");
}
char *puerto_memoria() {
	return config_get_string_value(config, "PUERTO_MEMORIA");
}

// cpu
char *ip_cpu() {
	return config_get_string_value(config, "IP_CPU");
}
char *puerto_cpu_dispatch() {
	return config_get_string_value(config, "PUERTO_CPU_DISPATCH");
}
char *puerto_cpu_interrupt() {
	return config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
}

// kernel
char *puerto_escucha() {
	return config_get_string_value(config, "PUERTO_ESCUCHA");
}

// planificacion
char *algoritmo_planificacion() {
	return config_get_string_value(config, "ALGORITMO_PLANIFICACION");
}
int grado_multiprogramacion() {
	return config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
}
double tiempo_max_bloqueado() {
	return config_get_double_value(config, "TIEMPO_MAXIMO_BLOQUEADO");
}

// sjf
double estimacion_inicial() {
	return config_get_double_value(config, "ESTIMACION_INICIAL");
}
double alfa() {
	return config_get_double_value(config, "ALFA");
}
