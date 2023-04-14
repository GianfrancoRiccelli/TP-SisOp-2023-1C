#ifndef PCB_H_
#define PCB_H_

#include <stdint.h>
#include <assert.h>
#include <commons/collections/list.h>

typedef struct {
	int fd; //4
	uint16_t id;//2
	uint32_t tamanio;//4
	uint8_t cant_instrucciones;// 1
	t_list *instrucciones;// 4 + ?
	uint32_t program_counter;// 4
	int tabla_paginas;//4
	double est_rafaga;// 8
	double rafaga_inicial;// 8
} t_pcb;

void imprimir_pcb(t_pcb *pcb);
int enviar_pcb(t_pcb* pcb, int socket_a_enviar, int IO);
t_pcb *desempaquetar_pcb(void *buffer);
void liberar_pcb(t_pcb *pcb);

#endif /* PCB_H_ */
