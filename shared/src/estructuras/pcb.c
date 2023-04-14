#include "../../include/estructuras/pcb.h"
#include "../../include/estructuras/instrucciones.h"
#include "../../include/sockets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void imprimir_pcb(t_pcb *pcb) {
	printf(
			"\n\nId: %d\nTamaño: %d\nCantidad de instrucciones: %d\nInstrucciones:\n\n",
			pcb->id, pcb->tamanio, pcb->cant_instrucciones);
	list_iterate(pcb->instrucciones, (void *) imprimir_instruccion);
}

t_pcb *desempaquetar_pcb(void *buffer) {
	int desplazamiento = 0;
	t_pcb *pcb = (t_pcb *) malloc(sizeof(t_pcb));
	assert(pcb != NULL);
	pcb->instrucciones = list_create();

	memcpy(&(pcb->fd), buffer + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);
          
	memcpy(&(pcb->id), buffer + desplazamiento, sizeof(uint16_t));
	desplazamiento += sizeof(uint16_t);

	memcpy(&(pcb->tamanio), buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(pcb->cant_instrucciones), buffer + desplazamiento, sizeof(uint8_t));
	desplazamiento += sizeof(uint8_t);

	for (int i = 0; i < pcb->cant_instrucciones; i++)
		list_add(pcb->instrucciones, desempaquetar_instruccion(buffer, &desplazamiento));
	

	memcpy(&(pcb->program_counter), buffer + desplazamiento, sizeof(uint32_t));
	desplazamiento += sizeof(uint32_t);

	memcpy(&(pcb->tabla_paginas), buffer + desplazamiento, sizeof(int));
	desplazamiento += sizeof(int);

	memcpy(&(pcb->est_rafaga), buffer + desplazamiento, sizeof(double));
	desplazamiento += sizeof(double);

	memcpy(&(pcb->rafaga_inicial), buffer + desplazamiento, sizeof(double));
	desplazamiento += sizeof(double);

	return pcb;
}

void liberar_pcb(t_pcb *pcb) {
	list_destroy_and_destroy_elements(pcb->instrucciones, (void*) liberar_instruccion);
	free(pcb);
}

void empaquetar_pcb(t_paquete *paquete_pcb, t_pcb *pcb) {
	assert(pcb != NULL);
	assert(paquete_pcb != NULL);
	
	agregar_a_paquete(paquete_pcb, &(pcb->fd), sizeof(int));
	agregar_a_paquete(paquete_pcb, &(pcb->id), sizeof(uint16_t));
	agregar_a_paquete(paquete_pcb, &(pcb->tamanio), sizeof(uint32_t));
	agregar_a_paquete(paquete_pcb, &(pcb->cant_instrucciones), sizeof(uint8_t));
	t_instruccion* siguiente;

	t_list_iterator *iterador_instrucciones = list_iterator_create(pcb->instrucciones);

	while (list_iterator_has_next(iterador_instrucciones)){
		siguiente = (t_instruccion*) list_iterator_next(iterador_instrucciones);
		assert(siguiente != NULL);
		empaquetar_instruccion(paquete_pcb, siguiente);
	}

	list_iterator_destroy(iterador_instrucciones);

	agregar_a_paquete(paquete_pcb, &(pcb->program_counter), sizeof(uint32_t));
	agregar_a_paquete(paquete_pcb, &(pcb->tabla_paginas), sizeof(int));
	agregar_a_paquete(paquete_pcb, &(pcb->est_rafaga), sizeof(double));
	agregar_a_paquete(paquete_pcb, &(pcb->rafaga_inicial), sizeof(double));

	//liberar_pcb(pcb);
}

int enviar_pcb(t_pcb *pcb, int socket_a_enviar, int IO) {
	t_paquete *paquete_pcb = crear_paquete(IO);
	empaquetar_pcb(paquete_pcb, pcb);
	enviar_paquete(paquete_pcb, socket_a_enviar);
	return EXIT_SUCCESS;
}
