#include "../include/instruccionescpu.h"
//./consola.out "../kiss-pruebas/INTEGRAL_1" 2048

bool proceso_terminado;
t_instruccion fetch(t_pcb* pcb);
bool decode(t_instruccion instruccion);
uint32_t fetch_operand(int tabla_paginas, uint32_t direccion_logica);
uint32_t leer_desde_memoria(int tabla_paginas, uint32_t direccion_logica);
uint32_t execute(t_instruccion instruccion, int tabla_paginas, uint32_t operando);

void ciclo_de_instruccion(t_pcb* pcb) {

	log_info(cpu_log, "Comenzando ciclo con nuevo PCB...");
	log_info(cpu_log, "PID: %d \t TP 1: %X", pcb->id, pcb->tabla_paginas);
	t_instruccion instruccion_actual;
	uint32_t hayIO;
	uint32_t operand;
	list_clean_and_destroy_elements(TLB, free);
	t_buffer* buffer = malloc(sizeof(t_buffer));
	tlb_hits = 0;
	acc_memoria = 0;

	while (pcb) {
		instruccion_actual = fetch(pcb);
		log_info(cpu_log, "Instruccion nº%d: %d", pcb->program_counter, instruccion_actual.identificador);
		if (decode(instruccion_actual))
			operand = fetch_operand(pcb->tabla_paginas, instruccion_actual.parametros[1]);
		hayIO = execute(instruccion_actual, pcb->tabla_paginas, operand);
		if (proceso_terminado) {
			proceso_terminado = false;
			log_info(cpu_log, "Proceso %d TERMINADO", pcb->id);
			log_info(cpu_log, "Devolviendo PCB actualizado del PID %d...", pcb->id);
			chequear_instrucciones(pcb->instrucciones, pcb->cant_instrucciones);
			enviar_pcb(pcb, socket_dispatch, 0);
			liberar_pcb(pcb);
			pcb = NULL;
			continue;
		}
		if (hayIO) {
			log_info(cpu_log, "Operacion de IO por %dms solicitada", hayIO);
			log_info(cpu_log, "Devolviendo PCB actualizado del PID %d...", pcb->id);
			chequear_instrucciones(pcb->instrucciones, pcb->cant_instrucciones);
			enviar_pcb(pcb, socket_dispatch, hayIO);
			liberar_pcb(pcb);
			pcb = NULL;
			continue;
		}
		sem_wait(&mutex_interrupt);
		if (check_interrupt) {
			log_warning(interrupt_log, "Se detecto una interrupcion!!!");
			log_info(interrupt_log, "Enviando PCB del PID %d...", pcb->id);
			check_interrupt = false;

			chequear_instrucciones(pcb->instrucciones, pcb->cant_instrucciones);
			enviar_pcb(pcb, socket_dispatch, 0);
			liberar_pcb(pcb);
			pcb = NULL;
		}
		sem_post(&mutex_interrupt);
	}
	log_info(cpu_log, "TLB HITS: %d  ACCESOS MEMORIA: %d", tlb_hits, acc_memoria);
	free(buffer);

}

t_instruccion fetch(t_pcb* pcb) {
	t_instruccion instruccion_nueva;
	instruccion_nueva = *((t_instruccion*) list_get(pcb->instrucciones, pcb->program_counter));

	log_info(cpu_log, "Program Counter: %d + 1", pcb->program_counter);
	pcb->program_counter++;
	return instruccion_nueva;
}

bool decode(t_instruccion instruccion) {
	if (instruccion.identificador == COPY)
		return true;
	else
		return false;
}

uint32_t fetch_operand(int tabla_paginas, uint32_t direccion_logica) {
	uint32_t valor_leido;
	valor_leido = leer_desde_memoria(tabla_paginas, direccion_logica);
	log_info(cpu_log, "El valor leido desde %d es: %d", direccion_logica, valor_leido);
	return valor_leido;
}

bool escribir_a_memoria(int tabla_paginas, uint32_t direccion_logica, void* dato_a_escribir) {
	int nro_de_pagina = floor(((double) direccion_logica) / ((double) *tam_de_pagina));
	int desplazamiento = direccion_logica - nro_de_pagina * (*tam_de_pagina);
	int direccion_fisica = obtener_direccion_fisica(tabla_paginas, direccion_logica, *entradas_por_tabla, *tam_de_pagina);

	int diferencia = desplazamiento + 4 - *tam_de_pagina;
	bool confirmacion;

	if (diferencia <= 0)
		confirmacion = pedir_escritura(4, dato_a_escribir, direccion_fisica);

	else {
		confirmacion = pedir_escritura(4 - diferencia, dato_a_escribir, direccion_fisica);
		if (confirmacion == false)
			return confirmacion;
		nro_de_pagina += 1;
		int nueva_direccion_logica = nro_de_pagina * (*tam_de_pagina);
		direccion_fisica = obtener_direccion_fisica(tabla_paginas, nueva_direccion_logica, *entradas_por_tabla, *tam_de_pagina);
		confirmacion = pedir_escritura(diferencia, dato_a_escribir + 4 - diferencia, direccion_fisica);
	}
	return confirmacion;
}

uint32_t leer_desde_memoria(int tabla_paginas, uint32_t direccion_logica) {
	int nro_de_pagina = floor(((double) direccion_logica) / ((double) *tam_de_pagina));
	int desplazamiento = direccion_logica - nro_de_pagina * (*tam_de_pagina);
	int direccion_fisica = obtener_direccion_fisica(tabla_paginas, direccion_logica, *entradas_por_tabla, *tam_de_pagina);
	void* valor_leido;
	int diferencia = desplazamiento + 4 - *tam_de_pagina;
	uint32_t retorno;
	if (diferencia <= 0)
		valor_leido = pedir_lectura(4, direccion_fisica);

	else {
		valor_leido = pedir_lectura(4 - diferencia, direccion_fisica);
		valor_leido = realloc(valor_leido, 4);
		nro_de_pagina += 1;
		void *nuevo_valor_leido;
		int nueva_direccion_logica = nro_de_pagina * (*tam_de_pagina);
		direccion_fisica = obtener_direccion_fisica(tabla_paginas, nueva_direccion_logica, *entradas_por_tabla, *tam_de_pagina);
		nuevo_valor_leido = pedir_lectura(diferencia, direccion_fisica);
		memcpy(valor_leido + 4 - diferencia, nuevo_valor_leido, diferencia);
		free(nuevo_valor_leido);
	}

	retorno = *(uint32_t *) valor_leido;
	free(valor_leido);
	return retorno;

}

uint32_t execute(const t_instruccion instruccion, const int tabla_paginas,  uint32_t operando) {
	switch (instruccion.identificador) {
	uint32_t valor_leido;
	case READ:
		valor_leido = leer_desde_memoria(tabla_paginas, instruccion.parametros[0]);
		log_info(cpu_log, "El valor leido desde %d es: %d", instruccion.parametros[0], valor_leido);
		break;
	case WRITE:
		escribir_a_memoria(tabla_paginas, instruccion.parametros[0], &(instruccion.parametros[1]));
		break;
	case COPY:
		escribir_a_memoria(tabla_paginas, instruccion.parametros[0], (void*) &operando);
		break;
	case NO_OP:
		log_info(cpu_log, "NO_OP");
		usleep(1000 * cpuconfig.retardo_noop);
		break;
	case IO:
		return instruccion.parametros[0];
		break;
	case EXIT:
		proceso_terminado = true;
		break;
	default:
		break;
		}
	return 0;
}
