#include "../include/colas.h"

void inicializar_estructuras() {
	sem_init(&mlog, 0, 1);
	log_protegido(string_from_format("Inicializando estructuras"));

	//Colas
	new_queue = queue_create();
	ready = list_create();
	bloqueado_queue = queue_create();
	bloqueado_tiempo = queue_create();
	suspendido_bloqueado = queue_create();
	suspendido_ready = queue_create();
	suspendido_tiempo = queue_create();
	exit_queue = queue_create();
	log_protegido(string_from_format("Colas creadas."));
	//Counters
	new_counter = 0;
	suspendido_counter = 0;
	id_counter = 1;
	log_protegido(string_from_format("Contadores inicializados."));

	//Semaforos
	sem_init(&nivel_multiprogramacion, 0, grado_multiprogramacion());
	sem_init(&procesos_en_ready, 0, 0);
	sem_init(&procesos_en_exit, 0, 0);
	sem_init(&ready_disponible, 0, 0);
	sem_init(&suspendido, 0, 0);
	sem_init(&bloqueado, 0, 0);
	log_protegido(string_from_format("Semaforos inicializados."));

	//Semaforos Mutex
	sem_init(&mnew, 0, 1);
	sem_init(&mnew_counter, 0, 1);
	sem_init(&mid_counter, 0, 1);
	sem_init(&mready, 0, 1);
	sem_init(&mexit, 0, 1);
	sem_init(&msuspendido_counter, 0, 1);
	sem_init(&mbloqueado, 0, 1);
	sem_init(&mbloqueado_tiempo, 0, 1);
	sem_init(&msuspendido_bloqueado, 0, 1);
	sem_init(&msuspendido_tiempo, 0, 1);
	sem_init(&msuspendido_ready, 0, 1);
	log_protegido(string_from_format("Semaforos Mutex inicializados."));
	interrupcion = false;
}

int inicializar_threads(){
	pthread_t hilo_ready, hilo_execute, hilo_blocked, /*hilo_suspendido_blocked,*/ 
			  hilo_exit, hilo_decrementador_cola_bloqueado;

	pthread_create(&hilo_ready, NULL, &thread_ready, NULL);
	pthread_create(&hilo_execute, NULL, &thread_execute, NULL);
	pthread_create(&hilo_blocked, NULL, &thread_blocked, NULL);
	//pthread_create(&hilo_suspendido_blocked, NULL, &thread_suspendido_blocked, NULL);
	pthread_create(&hilo_decrementador_cola_bloqueado, NULL, &decrementador_cola_bloqueado, NULL);
	pthread_create(&hilo_exit, NULL, &thread_exit, NULL);

	pthread_setname_np(hilo_ready, "READY");
	pthread_setname_np(hilo_execute, "EXEC");
	pthread_setname_np(hilo_blocked, "BLOCK");
	//pthread_setname_np(hilo_suspendido_blocked, "SUS-BLOCK");
	pthread_setname_np(hilo_decrementador_cola_bloqueado, "DECRE-BLOCK");
	pthread_setname_np(hilo_exit, "EXIT");

	// pthread_join(hilo_ready, NULL);
	// pthread_join(hilo_execute, NULL);
	// pthread_join(hilo_blocked, NULL);
	// pthread_join(hilo_suspendido_blocked, NULL);
	// pthread_join(hilo_exit, NULL);

	return EXIT_SUCCESS;
}

void agregar_a_cola(t_queue *cola, t_pcb *pcb, sem_t semaforo_mutex){
	sem_wait(&semaforo_mutex);
	queue_push(cola, pcb);
	sem_post(&semaforo_mutex);
}

t_pcb* sacar_de_cola(t_queue *cola, sem_t semaforo_mutex){
	sem_wait(&semaforo_mutex);
	t_pcb* pcb = queue_pop(cola);
	sem_post(&semaforo_mutex);
	return pcb;
}

void log_protegido(char* mensaje){
	sem_wait(&mlog);
	log_info(log_kernel, "%s", mensaje);
	sem_post(&mlog);
	free(mensaje);
}
// new
void agregar_a_new(t_pcb *pcb) {
	sem_wait(&mid_counter);
	pcb->id = id_counter;
	id_counter++;
	sem_post(&mid_counter);
	pcb->program_counter = 0;
	pcb->est_rafaga = estimacion_inicial();
	pcb->rafaga_inicial = 0;

	sem_wait(&mnew);
	queue_push(new_queue, pcb);
	sem_post(&mnew);
	sem_wait(&mnew_counter);
	new_counter++;
	sem_post(&mnew_counter);
	log_protegido(string_from_format("CONSOLA:Se agrego el pcb %d a new.", pcb->id));

	sem_post(&ready_disponible);
}

void imprimir_elemento_ready(t_pcb* pcb){
	log_protegido(string_from_format("PCB id:%d   Rafaga:%lf", pcb->id, pcb->est_rafaga));
}

void imprimir_lista_ready(){
	log_protegido(string_from_format("Imprimo elementos de la lista ready en orden:"));
	list_iterate(ready, (void*)imprimir_elemento_ready);
}


// ready-fifo
void agregar_a_ready_fifo(t_pcb *pcb) {
	//sem_wait(&procesos);
	//sem_wait(&grado_multip);

	//pthread_mutex_lock(&mready);
	sem_wait(&mready);
	list_add(ready, pcb);
	sem_post(&mready);
	//pthread_mutex_unlock(&mready);
}

// ready-sjf
void calcular_estimacion(double real, t_pcb *pcb) {
	pcb->est_rafaga = pcb->est_rafaga * alfa() + real * (1 - alfa());
	log_protegido(string_from_format("EXECUTE: Estimacion PID: %d = %lf",pcb->id, pcb->est_rafaga));
}

int menor_estimacion(t_pcb *pcb1, t_pcb * pcb2) {
	return pcb1->est_rafaga <= pcb2->est_rafaga;
}

void agregar_a_ready_sjf(t_pcb *pcb) {
	//sem_wait(&procesos);
	//sem_wait(&grado_multip);

	//pthread_mutex_lock(&mready);
	sem_wait(&mready);
	list_add_sorted(ready, pcb, (void *) menor_estimacion);
	sem_post(&mready);
	//pthread_mutex_unlock(&mready);

	//TODO Avisar por interrupt
	if(interrupcion){
		log_protegido(string_from_format("Se da aviso de interrupt a CPU."));
		send(socket_interrupt, &interrupcion, sizeof(bool), 0);
		interrupcion = false;
	}
	

}

// ready
void agregar_a_ready(t_pcb *pcb) {
	//enviar_mensaje_memoria("init", 5);
	//pcb->tabla_paginas = (int) recibir_mensaje_memoria();
	if(!strcmp("FIFO", algoritmo_planificacion()))
		agregar_a_ready_fifo(pcb);
	if(!strcmp("SRT", algoritmo_planificacion()))
		agregar_a_ready_sjf(pcb);

	imprimir_lista_ready();
	sem_post(&procesos_en_ready);
}

t_pcb* quitar_de_ready(){
	sem_wait(&mready);
	t_pcb* pcb = list_remove(ready, 0);
	sem_post(&mready);
	return pcb;
}

//false = new, true = suspendido-ready
bool disponible_para_ready(){
	bool respuesta;
	sem_wait(&ready_disponible);

	//Chequea si hay un proceso en new
	sem_wait(&mnew_counter);
	if(new_counter > 0){
		respuesta = false;
	}
	sem_post(&mnew_counter);

	//Chequea si hay un proceso en suspendido_ready
	sem_wait(&msuspendido_counter);
	if(suspendido_counter > 0){
		respuesta = true;
	}
	sem_post(&msuspendido_counter);

	return respuesta;
}

void *thread_ready(){
	while(1){
		//Revisar semaforo de nivel multiprogramacion
		sem_wait(&nivel_multiprogramacion);
		log_protegido(string_from_format("READY:El nivel de multiprogramacion permite agregar un proceso a ready."));

		if(disponible_para_ready()){
			//Suspendido-ready
			log_protegido(string_from_format("READY:Hay un proceso disponible en suspendido-ready."));
			sem_wait(&msuspendido_counter);
			suspendido_counter--;
			sem_post(&msuspendido_counter);
			t_pcb *pcb = sacar_de_cola(suspendido_ready, msuspendido_ready);
			//pcb->tabla_paginas = agregar_proceso_memoria(pcb->id, pcb->tamanio);
			agregar_a_ready(pcb);
			log_protegido(string_from_format("READY:Se agrego el proceso %d a ready.", pcb->id));
		}else{
			//New
			log_protegido(string_from_format("READY:Hay un proceso disponible en new."));
			sem_wait(&mnew_counter);
			new_counter--;
			sem_post(&mnew_counter);
			t_pcb *pcb = sacar_de_cola(new_queue, mnew);
			pcb->tabla_paginas = agregar_proceso_memoria(pcb->id, pcb->tamanio);
			agregar_a_ready(pcb);
			log_protegido(string_from_format("READY:Se agrego el proceso %d a ready.", pcb->id));
		}
		//Avisa que hay procesos en ready
	}
}

//execute
t_pcb_recibido* recibir_proceso(){
	t_pcb_recibido* proceso_para_devolver = malloc(sizeof(t_pcb_recibido));
	t_pcb* pcb;
	
	//Saco el tiempo de IO que se lo pase como "codigo de operacion"
	int IO = recibir_operacion(socket_dispatch);
	assert(IO != -1);

	t_buffer* buffer = crear_buffer(); 
	buffer->stream = recibir_buffer(&(buffer->size), socket_dispatch);

	pcb = desempaquetar_pcb(buffer->stream);
	
	proceso_para_devolver->pcb = pcb;
	proceso_para_devolver->milisegundos = IO;
	proceso_para_devolver->contador = (int) tiempo_max_bloqueado();
	
	free(buffer->stream);
	free(buffer);

	return proceso_para_devolver;
}

double calcular_milisegundos(struct timeval tiempo_envio, struct timeval tiempo_retorno){
	double milisegundos_envio;
	double milisegundos_retorno;

	milisegundos_envio = (double)(tiempo_envio.tv_sec*1000.0) + (double)(tiempo_envio.tv_usec / 1000.0);
	milisegundos_retorno = (double)(tiempo_retorno.tv_sec*1000.0) + (double)(tiempo_retorno.tv_usec / 1000.0);
	return milisegundos_retorno - milisegundos_envio;

}

void *thread_execute(){
	struct timeval tiempo_envio, tiempo_retorno;
	int buffer_vacio;
	t_pcb_recibido* proceso_recibido;
	double real;
	while(1){
		sem_wait(&procesos_en_ready);
		log_protegido(string_from_format("EXECUTE:Hay procesos en ready para ejecutar."));

		t_pcb* pcb = quitar_de_ready();
		interrupcion = true;
		log_protegido(string_from_format("EXECUTE:Se mueve a ejecutar el PID: %d.", pcb->id));
		chequear_instrucciones(pcb->instrucciones, pcb->cant_instrucciones);
		//Enviar a cpu;
		enviar_pcb(pcb, socket_dispatch, 0);
		gettimeofday(&tiempo_envio, NULL);
		log_protegido(string_from_format("EXECUTE:Proceso enviado a CPU para su ejecucion."));
		
		//Eliminar pcb
		liberar_pcb(pcb);
		//Recibir nuevo pcb con milisegundos
		ioctl(socket_dispatch, FIONREAD, &buffer_vacio);

		proceso_recibido = recibir_proceso();
		interrupcion = false;
		chequear_instrucciones(proceso_recibido->pcb->instrucciones, proceso_recibido->pcb->cant_instrucciones);
		//Chequea que el buffer de socket este vacio (que no haya quedado basura)
		//De lo contrario termina el programa
		ioctl(socket_dispatch, FIONREAD, &buffer_vacio);
		assert(buffer_vacio == 0);

		gettimeofday(&tiempo_retorno, NULL);
		log_protegido(string_from_format("EXECUTE:Proceso recibido de CPU."));

		if(proceso_recibido->pcb->program_counter < proceso_recibido->pcb->cant_instrucciones){
			//Interrupcion
			if(proceso_recibido->milisegundos == 0){
				log_protegido(string_from_format("EXECUTE: PID: %d recibido por interrupcion.", proceso_recibido->pcb->id));
				if(proceso_recibido->pcb->rafaga_inicial == 0){
					proceso_recibido->pcb->rafaga_inicial = proceso_recibido->pcb->est_rafaga;
					proceso_recibido->pcb->est_rafaga = proceso_recibido->pcb->est_rafaga - calcular_milisegundos(tiempo_envio, tiempo_retorno);
				}else{
					proceso_recibido->pcb->est_rafaga = proceso_recibido->pcb->est_rafaga - calcular_milisegundos(tiempo_envio, tiempo_retorno);
				}
				//log_protegido(string_from_format("EXECUTE:Rafaga actual del proceso %d: %lf",proceso_recibido->pcb->id, proceso_recibido->pcb->est_rafaga));
				agregar_a_ready(proceso_recibido->pcb);
				log_protegido(string_from_format("EXECUTE:Se agrego el PID: %d a ready.", proceso_recibido->pcb->id));
				free(proceso_recibido);
			}else{
				//Espera
				log_protegido(string_from_format("EXECUTE: PID: %d recibido por I/O: %dms",proceso_recibido->pcb->id ,proceso_recibido->milisegundos));

				//Hago la estimacion de la proxima rafaga
				if(proceso_recibido->pcb->rafaga_inicial == 0){
					calcular_estimacion(calcular_milisegundos(tiempo_envio, tiempo_retorno), proceso_recibido->pcb);
				}else{
					real = proceso_recibido->pcb->est_rafaga;
					proceso_recibido->pcb->est_rafaga = proceso_recibido->pcb->rafaga_inicial;
					real = proceso_recibido->pcb->est_rafaga - real + calcular_milisegundos(tiempo_envio, tiempo_retorno);
					calcular_estimacion(real, proceso_recibido->pcb);
				}
				proceso_recibido->pcb->rafaga_inicial = 0;

				//agregar_a_cola(bloqueado_queue, proceso_recibido->pcb, mbloqueado);
				sem_wait(&mbloqueado);
				queue_push(bloqueado_queue, proceso_recibido);
				sem_post(&mbloqueado);
				log_protegido(string_from_format("EXECUTE:Se agrego el PID: %d a bloqueado.", proceso_recibido->pcb->id));
				sem_post(&bloqueado);
			}
		}else{
			//Exit
			log_protegido(string_from_format("EXECUTE: PID: %d finalizo sus instrucciones.", proceso_recibido->pcb->id));
			agregar_a_cola(exit_queue, proceso_recibido->pcb, mexit);
			log_protegido(string_from_format("EXECUTE: PID: %d agregado a exit.", proceso_recibido->pcb->id));
			free(proceso_recibido);
			sem_post(&procesos_en_exit);
			sem_post(&nivel_multiprogramacion);
		}

	}
}

void suspender_proceso(t_pcb* pcb){
	suspender_proceso_memoria(pcb->tabla_paginas, pcb->id);
	sem_post(&nivel_multiprogramacion);
}

//blocked
void* decrementador_cola_bloqueado(void* arg){
	t_pcb_recibido* elemento_actual;
	t_list_iterator* iterador_cola_bloqueado;

	while(1){
		sleep(1);
		sem_wait(&mbloqueado);
		iterador_cola_bloqueado = list_iterator_create(bloqueado_queue->elements);
		while(list_iterator_has_next(iterador_cola_bloqueado)){
			elemento_actual = (t_pcb_recibido*) list_iterator_next(iterador_cola_bloqueado);
			(elemento_actual->contador) -= 1000;
			if (elemento_actual -> contador == -1000)
				suspender_proceso(elemento_actual->pcb);
		}
		list_iterator_destroy(iterador_cola_bloqueado);
		sem_post(&mbloqueado);
	}
}
void esperar_bloqueado(int* tiempo_espera, int* resto){
	usleep(1000 * (*tiempo_espera));
	*resto -= (*tiempo_espera);
	*tiempo_espera = 0;
}

void *thread_blocked(){
	int tiempo_total_espera;
	bool no_esta_suspendido;
	while(1){
		sem_wait(&bloqueado);

		sem_wait(&mbloqueado);
		t_pcb_recibido* pcb_bloqueado = queue_pop(bloqueado_queue);
		sem_post(&mbloqueado);

		tiempo_total_espera = pcb_bloqueado->milisegundos;
		log_protegido(string_from_format("BLOCKED:Se ha bloqueado el PID: %d por %dms",pcb_bloqueado->pcb->id,tiempo_total_espera));

		if(pcb_bloqueado->contador < 0)
			no_esta_suspendido = false;
		else
			no_esta_suspendido = true;

		if(pcb_bloqueado->contador > 0){

			if(pcb_bloqueado->milisegundos > pcb_bloqueado->contador)
				esperar_bloqueado(&(pcb_bloqueado->contador), &(pcb_bloqueado->milisegundos));
			else{
				esperar_bloqueado(&(pcb_bloqueado->milisegundos), &(pcb_bloqueado->contador));
			}
		}

	
		//Si queda tiempo para seguir esperando, hay que suspender
		if(pcb_bloqueado->milisegundos > 0){
			//Suspender
			if(no_esta_suspendido){
				log_protegido(string_from_format("BLOCKED: Tiempo de espera excedido. Tiempo restante: %dms \n SUSPENDIENDO PID: %d...",pcb_bloqueado->milisegundos, pcb_bloqueado->pcb->id));
				suspender_proceso(pcb_bloqueado->pcb);
			}
			else{
				log_protegido(string_from_format("BLOCKED: El PID: %d ya habia sido SUSPENDIDO", pcb_bloqueado->pcb->id));
			}
			esperar_bloqueado(&(pcb_bloqueado->milisegundos), &(pcb_bloqueado->contador));
			log_protegido(string_from_format("BLOCKED: PID: %d SUSPENDIDO. IO: %dms \t Tiempo total esperado: %dms. Agregando a suspendido ready... ",pcb_bloqueado->pcb->id,tiempo_total_espera,((int)tiempo_max_bloqueado()) - pcb_bloqueado->contador));

			agregar_a_cola(suspendido_ready, pcb_bloqueado->pcb, msuspendido_ready);
			sem_wait(&msuspendido_counter);
			suspendido_counter++;
			sem_post(&msuspendido_counter);
		
			sem_post(&ready_disponible);
		}
		else{
			log_protegido(string_from_format("BLOCKED: Espera de PID: %d por %d completada exitosamente. Agregando a ready...",pcb_bloqueado->pcb->id,tiempo_total_espera));
			agregar_a_ready(pcb_bloqueado->pcb);
		}

		free(pcb_bloqueado);

	}
}

//suspendido-blocked
void *thread_suspendido_blocked(){
	while(1){
		sem_wait(&suspendido);
		log_protegido(string_from_format("SUSPENDIDO_BLOCKED:ESTO NO DEBERIA MOSTRARSE HELP"));
		log_protegido(string_from_format("SUSPENDIDO_BLOCKED:Espera en suspendido completada, agregando a suspendido-ready."));
	}
	return NULL;
}


//exit
void *thread_exit(){
	uint16_t pid;
	while(1){
		sem_wait(&procesos_en_exit);

		t_pcb* pcb = sacar_de_cola(exit_queue, mexit);
		pid = pcb->id;
		log_protegido(string_from_format("EXIT:Finalizando proceso %d.", pid));
		finalizar_proceso_memoria(pcb->tabla_paginas);

		terminar_consola(pcb->fd);

		liberar_pcb(pcb);
		log_protegido(string_from_format("EXIT:Proceso %d finalizado.", pid));
	}
	return NULL;
}
