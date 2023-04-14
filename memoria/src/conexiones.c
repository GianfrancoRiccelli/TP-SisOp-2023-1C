#include "../include/conexiones.h"

sem_t mutex_log;

void loguear_protegido (char* cadena, t_log_level level){
    switch(level) {
	case LOG_LEVEL_ERROR:
		sem_wait(&mutex_log);
		log_error (memoria_log, "%s",cadena);
		sem_post(&mutex_log);
		break;
	case LOG_LEVEL_DEBUG:
		sem_wait(&mutex_log);
		log_debug (memoria_log,"%s", cadena);
		sem_post(&mutex_log);
		break;
	case LOG_LEVEL_INFO:
		sem_wait(&mutex_log);
		log_info (memoria_log, "%s",cadena);
		sem_post(&mutex_log);
		break;
	case LOG_LEVEL_TRACE:
		sem_wait(&mutex_log);
		log_trace (memoria_log,"%s", cadena);
		sem_post(&mutex_log);
		break;
	case LOG_LEVEL_WARNING:
		sem_wait(&mutex_log);
		log_warning (memoria_log,"%s", cadena);
		sem_post(&mutex_log);
		break;
    }
}

int iniciar_conexiones(){

    log_info(memoria_log, "Iniciando Conexiones...");
    int socket_servidor,
        socket_cliente,
        socket_cpu,
        socket_kernel;

    sem_init(&mutex_tabla_planificacion, 0, 1);

    log_info(memoria_log, "Iniciando servidor...");
    socket_servidor = iniciar_servidor(configuracion->puerto);
    if(socket_servidor == -1){
        log_error(memoria_log, "ERROR - No se pudo crear el servidor");
        return EXIT_FAILURE;
    }
    log_info(memoria_log, "Servidor Iniciado!!!");

    pthread_t comunicacion_cpu;
    pthread_t comunicacion_kernel;

    sem_init(&mutex_log, 0, 1);

    for(int i=0; i<2; i++){
        loguear_protegido("Esperando Cliente...", LOG_LEVEL_INFO);
        socket_cliente = esperar_cliente(socket_servidor);

        if (recibir_operacion(socket_cliente) == 0){
            loguear_protegido("Se conecto el CPU!!!", LOG_LEVEL_INFO);
            socket_cpu = socket_cliente;
            pthread_create(&comunicacion_cpu,NULL,(void *)conectar_con_cpu,&socket_cpu);
        }
        
        else{
            loguear_protegido("Se conecto el KERNEL!!!", LOG_LEVEL_INFO);
            socket_kernel = socket_cliente;
            pthread_create(&comunicacion_kernel,NULL,(void *)conectar_con_kernel,&socket_kernel);
        }
        sleep(1);
    }

    pthread_join(comunicacion_cpu, NULL);
    pthread_join(comunicacion_kernel, NULL);

    return EXIT_SUCCESS;
}

void enviar_paquete_configuraciones(int socket_cpu);
void devolver_indice(t_buffer *buffer, int socket_cpu);
void devolver_marco(t_buffer *buffer, int socket_cpu);
void leer(t_buffer *buffer, int socket_cpu);
void escribir(t_buffer *buffer, int socket_cpu);

void *conectar_con_cpu(int* socket_cpu){
    cpu_log = log_create("memoria_cpu.log", "MEMORIA-CPU", 1, LOG_LEVEL_INFO);
    printf("Aca se hace la conexion con CPU en el socket: %d\n", *socket_cpu);

    log_info(cpu_log, "Enviando paquete de configuraciones al CPU...");
    enviar_paquete_configuraciones(*socket_cpu);

    t_buffer *buffer = crear_buffer();

    while(true) {
        log_info(cpu_log, "Esperando peticion de CPU...");

        int instruccion_recibida = recibir_operacion(*socket_cpu);
        if (instruccion_recibida == -1) {
        	log_error(cpu_log, "ERROR - El cliente ha cerrado la conexion");
        	return NULL;
        }

        log_info(cpu_log, "Instruccion de CPU leida!!!");

        buffer->stream = recibir_buffer(&(buffer->size), *socket_cpu);
        log_info(cpu_log, "Buffer Recibido!!!");
		
		usleep(1000 * configuracion->retardo_memoria);

        switch(instruccion_recibida) {
		case DEVOLVER_INDICE_TABLA_NVL2:
			devolver_indice(buffer, *socket_cpu);
			break;

		case DEVOLVER_MARCO:
			devolver_marco(buffer, *socket_cpu);
			break;

		case LEER:
			leer(buffer, *socket_cpu);
			break;

		case ESCRIBIR:
			escribir(buffer, *socket_cpu);
			break;

		default:
			log_error(cpu_log, "ERROR - La instruccion recibida del CPU es invalida");
			break;
        }
        free(buffer->stream);
    }
    free(buffer);
    return NULL;
}

void enviar_paquete_configuraciones(int socket_cpu) {
	t_paquete* paquete_configuraciones = crear_paquete(932);
	agregar_a_paquete(paquete_configuraciones, &(configuracion->entradas_por_tabla), sizeof(int));
	agregar_a_paquete(paquete_configuraciones, &(configuracion->tam_pagina), sizeof(int));
	enviar_paquete(paquete_configuraciones, socket_cpu);
	log_info(cpu_log, "Paquete enviado");
}

void loguear_page_faults(int pid){

	sem_wait(&mutex_pf);

	if(pid_actual != pid){
		if(page_fault_counter > 0){
			log_warning(cpu_log,"PID: %d PAGE FAULTS: %d", pid_actual, page_fault_counter);
			page_fault_counter = 0;
		}
		if(pid >= 0)
			log_info(cpu_log,"Nuevo PID: %d", pid);
		pid_actual = pid;
	}

	sem_post(&mutex_pf);

}

void devolver_indice(t_buffer *buffer, int socket_cpu) {
	int *dato = (int*) sacar_de_buffer(buffer, sizeof(int));
	t_list* direccion_tabla1 = (t_list*)*dato;

	int pid = ((t_tabla2*)(list_get( (t_list*) (list_get( direccion_tabla1 ,0)) ,0)))->id;

	sem_wait(&mutex_pid_actual);
	loguear_page_faults(pid);
	sem_post(&mutex_pid_actual);

	log_info(cpu_log, "El CPU quiere Tabla NVL 2");


	int *indice = sacar_de_buffer(buffer, sizeof(int));
	t_list *direccion_tabla2 = obtener_tabla2(direccion_tabla1, *indice);
	send(socket_cpu, &direccion_tabla2, sizeof(int), 0);
	free(dato);
	free(indice);
}

void devolver_marco(t_buffer *buffer, int socket_cpu) {
	log_info(cpu_log, "El CPU quiere Nro de marco");
	int* dato = (int *) sacar_de_buffer(buffer, sizeof(int));
	t_list *direccion = (t_list*)*dato;
	int *indice = sacar_de_buffer(buffer, sizeof(int));
	int marco = obtener_marco(direccion, *indice);
	send(socket_cpu, &marco, sizeof(int), 0);
	free(dato);
	free(indice);
}

void leer(t_buffer *buffer, int socket_cpu) {
	log_info(cpu_log, "El CPU quiere LEER de memoria");
	int *dato = sacar_de_buffer(buffer, sizeof(int));
	int direccion_fisica = *dato;
	int *tam_leer = sacar_de_buffer(buffer, sizeof(int));
	void *datos_leidos = leer_de_memoria(direccion_fisica, *tam_leer);
	send(socket_cpu, datos_leidos, *tam_leer, 0);
	free(datos_leidos);
	free(tam_leer);
	free(dato);
}

void escribir(t_buffer *buffer, int socket_cpu) {
	log_info(cpu_log, "El CPU quiere ESCRIBIR en memoria");
	int *dato_direccion_fisica = sacar_de_buffer(buffer, sizeof(int));
	int direccion_fisica = *dato_direccion_fisica;
	int *tam_escribir = sacar_de_buffer(buffer, sizeof(int));
	void *datos = sacar_de_buffer(buffer, *tam_escribir);
	escribir_a_memoria(direccion_fisica, *tam_escribir, datos);
	bool confirmacion = true;
	send(socket_cpu, &confirmacion, sizeof(bool), 0);
	free(dato_direccion_fisica);
	free(tam_escribir);
	free(datos);
}

void crear_tabla(t_buffer *buffer, int socket_kernel);
void proceso_suspendido(t_buffer *buffer, int socket_kernel);
void liberar(t_buffer *buffer, int socket_kernel);

void *conectar_con_kernel(int *socket_kernel){
    kernel_log = log_create("memoria_kernel.log", "MEMORIA-KERNEL", 1, LOG_LEVEL_INFO);
    printf("Aca se hace la conexion con KERNEL en el socket: %d\n", *socket_kernel);

    log_info(kernel_log, "Enviando mensaje de confirmacion...");
    bool confirmacion = true;
    send(*socket_kernel, &confirmacion, sizeof(bool), 0);
    log_info(kernel_log, "Mensaje enviado!!!");

    t_buffer *buffer = crear_buffer();

    while(true){
        log_info(kernel_log, "Esperando Peticion de KERNEL...");

        int instruccion_recibida = recibir_operacion(*socket_kernel);
        if (instruccion_recibida == -1) {
        	log_error(kernel_log, "ERROR - El cliente ha cerrado la conexion");
        	return NULL;
        }

        log_info(kernel_log, "Recibiendo buffer...");
        buffer->stream = recibir_buffer(&(buffer -> size), *socket_kernel);
        log_info(kernel_log, "Buffer Recibido!!!");

        switch(instruccion_recibida) {
		case CREAR_NUEVA_TABLA:
			crear_tabla(buffer, *socket_kernel);
			break;

		case PROCESO_SUSPENDIDO:
			proceso_suspendido(buffer, *socket_kernel);
			break;

		case LIBERAR:
			liberar(buffer, *socket_kernel);
			break;

		default:
			log_error(kernel_log, "ERROR - La instruccion recibida del KERNEL es invalida");
			break;
        }
        free(buffer->stream);
    }
    free(buffer);
    return NULL;
}

void crear_tabla(t_buffer *buffer, int socket_kernel) {
	log_info(kernel_log, "El kernel quiere agregar un proceso");
	log_info(kernel_log, "Recibiendo Proceso...");
	int *id_proceso = sacar_de_buffer(buffer, sizeof(int));
	int *tam_proceso = sacar_de_buffer(buffer, sizeof(int));
	void *puntero_proceso = calloc(*tam_proceso,1);
	log_info(kernel_log, "Proceso recibido!!!");

	log_info(kernel_log, "Agregando proceso...");
	t_list *direccion_de_tabla;
	direccion_de_tabla = agregar_proceso(*id_proceso, *tam_proceso, puntero_proceso);
	free(puntero_proceso);

	if(!direccion_de_tabla){
		log_error(kernel_log, "ERROR - No se ha podido agregar el proceso");
		return;
	}
	log_info(kernel_log, "Proceso agregado exitosamente!!");

	log_info(kernel_log, "Respondiendo al kernel...");
	send(socket_kernel,&direccion_de_tabla, sizeof(int), 0);
	log_info(kernel_log, "Listo!");

	free(id_proceso);
	free(tam_proceso);
}

void proceso_suspendido(t_buffer *buffer, int socket_kernel) {
	log_info(kernel_log, "El KERNEL quiere suspender un proceso");
	//bool confirmacion = true;

	int* aux = sacar_de_buffer(buffer, sizeof(int));
	t_list *direccion_de_tabla = (t_list*)*aux;
	free(aux);

	int pid = ((t_tabla2*)(list_get( (t_list*) (list_get( direccion_de_tabla ,0)) ,0)))->id;

	sem_wait(&mutex_pid_actual);
	if(pid == pid_actual)
		loguear_page_faults(-1);
	sem_post(&mutex_pid_actual);

	log_info(kernel_log, "Proceso recibido! PID: %d Tabla: %X  SUSPENDIENDO...",pid, (int)direccion_de_tabla);

	suspender_proceso2(direccion_de_tabla);
	log_info(kernel_log, "PID: %d Suspendido!!!",pid);

	//send(socket_kernel, &confirmacion, sizeof(bool), 0); ARREGLANDO CAGADAS DEL KERNEL 
}

void liberar(t_buffer *buffer, int socket_kernel) {
	log_info(kernel_log, "El kernel quiere eliminar un Proceso");
	bool confirmacion = true;

	int* aux = sacar_de_buffer(buffer, sizeof(int));
	t_list *direccion_de_tabla = (t_list*)*aux;
	free(aux);

	int pid = ((t_tabla2*)(list_get( (t_list*) (list_get( direccion_de_tabla ,0)) ,0)))->id;

	sem_wait(&mutex_pid_actual);
	if(pid == pid_actual)
		loguear_page_faults(-1);
	sem_post(&mutex_pid_actual);

	log_info(kernel_log, "Proceso recibido! PID: %d Tabla: %X",pid, (int)direccion_de_tabla);
	finalizar_proceso(direccion_de_tabla);

	send(socket_kernel, &confirmacion, sizeof(bool), 0);
}