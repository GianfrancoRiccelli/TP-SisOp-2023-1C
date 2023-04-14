#include "../include/conexion_consola.h"


void conectar_consola(int kernel) {
	log_protegido(string_from_format("CONSOLA:Servidor listo para recibir al cliente"));
	pthread_t thread;
	pid_counter = 0;

	// recibir muchas consolas
	while (1) {
		int consola = esperar_cliente(kernel);
		log_protegido(string_from_format("CONSOLA:Se conectó un cliente!"));
		pthread_create(&thread, NULL, (void *) proceso_new, &consola);
		pthread_setname_np(thread, "CONSOLA");
		pthread_detach(thread);
	}
}

void recibir_paquete_consola(void *buffer, int size, t_pcb *pcb) {
	int desplazamiento = 0;

	memcpy(&(pcb->tamanio), buffer + desplazamiento, sizeof(uint32_t)); 
	desplazamiento += sizeof(uint32_t);

	pcb->instrucciones = list_create();

	while(desplazamiento < size)
		list_add(pcb->instrucciones, desempaquetar_instruccion(buffer, &desplazamiento));

	pcb->cant_instrucciones = list_size(pcb->instrucciones);
}

t_pcb *generar_pcb(int socket_cliente) {
	int size;
	void *buffer = recibir_buffer(&size, socket_cliente);

	t_pcb *pcb = (t_pcb *) malloc(sizeof(t_pcb));
	pcb->fd = socket_cliente;

	recibir_paquete_consola(buffer, size, pcb);
	free(buffer);
	return pcb;
}

void proceso_new(int *socket_cliente) {
	while (1) {
		t_pcb *pcb;
		switch (recibir_operacion(*socket_cliente)) {
		case INSTRUCCIONES_CONSOLA:
			pcb = generar_pcb(*socket_cliente);
			agregar_a_new(pcb);

			break;

		case -1:
			log_protegido(string_from_format("CONSOLA:El cliente se desconectó."));
			return;

		default:
			log_protegido(string_from_format("CONSOLA_ERROR:Código de operación inválido."));
			return;
		}
	}
}

void terminar_consola(int socket_consola) {
	int mensaje = TERMINAR_CONSOLA;
	send(socket_consola, &mensaje, sizeof(int), 0);
}
