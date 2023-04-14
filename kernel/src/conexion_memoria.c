#include "../include/conexion_memoria.h"

void conectar_memoria(char *ip, char *puerto) {
	log_protegido(string_from_format("MEMORIA:Conectando a memoria..."));

	while((socket_memoria = crear_conexion(ip, puerto)) <= 0){
		log_protegido(string_from_format("MEMORIA:No se ha podido establecer la conexion con Memoria"));
		sleep(5);
	}

	int distinto_de_cero = 1;
	bool confirmacion;
	send(socket_memoria, &distinto_de_cero, sizeof(int),0);
	recv(socket_memoria, &confirmacion, sizeof(bool), MSG_WAITALL);

	if(confirmacion)
		log_protegido(string_from_format("MEMORIA:Conexion con memoria exitosa"));
	else
		log_protegido(string_from_format("MEMORIA:ERROR - Handshake fallido"));
}

void *recibir_mensaje_memoria() {
	return recibir_mensaje(socket_memoria);
}

void desconectar_memoria() {
	liberar_conexion(socket_memoria);
}

int agregar_proceso_memoria(int pid, int tam_proceso){
	log_protegido(string_from_format("MEMORIA:Creando Proceso..."));
	log_protegido(string_from_format("MEMORIA:Enviando paquete a memoria, espero por nueva tabla"));
	t_paquete *instrucciones_a_memoria = crear_paquete(CREAR_NUEVA_TABLA);
	agregar_a_paquete(instrucciones_a_memoria, &pid, sizeof(int));
	agregar_a_paquete(instrucciones_a_memoria, &tam_proceso, sizeof(int));
	enviar_paquete(instrucciones_a_memoria, socket_memoria);

	log_protegido(string_from_format("MEMORIA: Esperando respuesta"));
	int tabla_proceso;
	recv(socket_memoria, &tabla_proceso, sizeof(int), MSG_WAITALL);
	log_protegido(string_from_format("MEMORIA:Respuesta Recibida!!!"));
	log_protegido(string_from_format("MEMORIA:Mi nueva tabla para el proceso %d es: %X", pid, tabla_proceso));

	return tabla_proceso;
}

bool suspender_proceso_memoria(int direccion_tabla, int pid){
	log_protegido(string_from_format("MEMORIA:Suspendiendo PID: %d", pid));
	//log_protegido(string_from_format("MEMORIA:Enviando paquete a memoria, espero confirmacion de suspendimiento"));
	t_paquete *instrucciones_a_memoria = crear_paquete(PROCESO_SUSPENDIDO);
	agregar_a_paquete(instrucciones_a_memoria, &direccion_tabla, sizeof(int));
	enviar_paquete(instrucciones_a_memoria, socket_memoria);

	//log_protegido(string_from_format("MEMORIA:Esperando respuesta..."));
	//bool confirmacion;
	//recv(socket_memoria, &confirmacion, sizeof(bool), MSG_WAITALL);
	//if(confirmacion)
		//log_protegido(string_from_format("MEMORIA:Habemos confirmacion de suspendimiento"));
	//else log_protegido(string_from_format("ERROR:La memoria rechazo la suspension"));
	return true;
}

bool finalizar_proceso_memoria(int direccion_tabla){
	log_protegido(string_from_format("MEMORIA:Finalizando Proceso..."));
	log_protegido(string_from_format("MEMORIA:Enviando paquete a memoria, espero confirmacion de boleteamiento"));
	t_paquete *instrucciones_a_memoria = crear_paquete(LIBERAR);
	agregar_a_paquete(instrucciones_a_memoria, &direccion_tabla, sizeof(int));
	enviar_paquete(instrucciones_a_memoria, socket_memoria);

	log_protegido(string_from_format("MEMORIA:Esperando respuesta..."));
	bool confirmacion;
	recv(socket_memoria, &confirmacion, sizeof(bool), MSG_WAITALL);
	log_protegido(string_from_format("MEMORIA:Respuesta Recibida!!!"));
	if(confirmacion)
		log_protegido(string_from_format("MEMORIA:Habemos confirmacion de boleteamiento"));
	else log_protegido(string_from_format("ERROR:La memoria rechazo el boleteamiento"));

	return confirmacion;
}
