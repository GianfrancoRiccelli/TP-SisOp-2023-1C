#include "../../include/estructuras/instrucciones.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t get_identificador(char *identificador_leido) {
	uint8_t identificador = NO_RECONOCIDO;
	if (!strcmp("NO_OP", identificador_leido))
		identificador = NO_OP;
	if (!strcmp("I/O", identificador_leido))
		identificador = IO;
	if (!strcmp("READ", identificador_leido))
		identificador = READ;
	if (!strcmp("COPY", identificador_leido))
		identificador = COPY;
	if (!strcmp("WRITE", identificador_leido))
		identificador = WRITE;
	if (!strcmp("EXIT", identificador_leido))
		identificador = EXIT;
	return identificador;
}

uint8_t get_cant_parametros(uint8_t identificador) {
	uint8_t cant_parametros = 0;
	switch (identificador) {
	case EXIT: case NO_OP:
		break;

	case IO: case READ:
		cant_parametros = 1;
		break;

	case WRITE: case COPY:
		cant_parametros = 2;
		break;
	}
	return cant_parametros;
}

t_instruccion *crear_instruccion(uint8_t identificador, uint8_t cant_parametros, uint32_t *parametros) {
	t_instruccion *instruccion = (t_instruccion *) malloc(sizeof(t_instruccion));
	assert(instruccion != NULL);
	instruccion->identificador = identificador;
	instruccion->cant_parametros = cant_parametros;
	instruccion->parametros = parametros;
	return instruccion;
}

void imprimir_instruccion(t_instruccion *instruccion) {

	assert(instruccion != NULL);

	uint8_t identificador = instruccion->identificador;
	uint8_t cant_parametros = instruccion->cant_parametros;
	uint32_t *parametros = instruccion->parametros;

	printf("\tidentificador: %d\n\tparametros: ", identificador);

	if (!parametros)
		puts("no hay parametros\n");


	for (int i = 0; i < cant_parametros; i++) {
		assert(parametros != NULL);
		if (i == cant_parametros - 1)
			printf("%d\n\n", parametros[i]);
		else
			printf("%d, ", parametros[i]);
	}
}

void liberar_instruccion(t_instruccion *instruccion) {
	if(instruccion->parametros != NULL)
		free(instruccion->parametros);
	free(instruccion);
}

void empaquetar_instruccion(t_paquete *paquete, t_instruccion *i) {

	assert(i->identificador <= 6);
	assert(i->cant_parametros >= 0 && i->cant_parametros <=2);

	agregar_a_paquete(paquete, &(i->identificador), sizeof(uint8_t));
	agregar_a_paquete(paquete, &(i->cant_parametros), sizeof(uint8_t));
	if (i->parametros)
		agregar_a_paquete(paquete, i->parametros, sizeof(uint32_t) * i->cant_parametros);
}

t_instruccion *desempaquetar_instruccion(void *buffer, int *desplazamiento) {
	uint8_t identificador, cant_parametros;
	uint32_t *parametros;

	assert(buffer != NULL);

	memcpy(&identificador, buffer + *desplazamiento, sizeof(uint8_t));
	*desplazamiento += sizeof(uint8_t);

	memcpy(&cant_parametros, buffer + *desplazamiento, sizeof(uint8_t));
	*desplazamiento += sizeof(uint8_t);

	if (cant_parametros) {
		int bytes = cant_parametros * sizeof(uint32_t);
		parametros = (uint32_t *) malloc(bytes);
		memcpy(parametros, buffer + *desplazamiento, bytes);
		*desplazamiento += bytes;
	}
	else parametros = NULL;

	return crear_instruccion(identificador, cant_parametros, parametros);
}
void chequear_instrucciones(t_list* lista_instrucciones, int cant_instrucciones){
	t_list_iterator* iterador_lista = list_iterator_create(lista_instrucciones);
	t_instruccion* instruccion_actual; 
	while(list_iterator_has_next(iterador_lista)){
		instruccion_actual = (t_instruccion*) list_iterator_next(iterador_lista);
		
		assert(instruccion_actual != NULL);
		//imprimir_instruccion(instruccion_actual);	
	}
	list_iterator_destroy(iterador_lista);
}
