#include "../include/kernel.h"

// int main() {
// 	crear_config();
// 	logger = log_create("kernel.log", "KERNEL", 1, LOG_LEVEL_DEBUG);

// 	int kernel = iniciar_servidor(puerto_escucha());
// 	int consola = esperar_cliente(kernel);
// 	t_pcb *pcb;

// 	log_info(logger, "Consola conectada!");

// 	if (recibir_operacion(consola) == INSTRUCCIONES_CONSOLA) {
// 		log_info(logger, "Se recibió el proceso:\n");
// 		pcb = generar_pcb(consola);
// 		imprimir_pcb(pcb);
// 	}

// 	log_info(logger, "Terminando consola");
// 	terminar_consola(pcb->fd);
// 	liberar_pcb(pcb);
// 	log_info(logger, "Consola terminada");
// 	destruir_config();
// 	log_destroy(logger);
// }

//void funcion_prueba_checkpoint(){
//	memoria_log = log_create("kernel.log", "KERNEL", 1, LOG_LEVEL_DEBUG);
//	config = config_create("kernel.config");
//	inicializar_colas_checkpoint();
//	conectar_memoria(ip_memoria(), puerto_memoria());
//	conectar_cpu(ip_cpu(), puerto_cpu_dispatch(), puerto_cpu_interrupt());
//
//	sem_init(&mutex_new, 0, 1);
//	sem_init(&mutex_suspendido, 0, 1);
//	sem_init(&mutex_exit, 0, 1);
//	sem_init(&mutex_memoria, 0, 1);
//
//	sem_init(&procesos_en_new, 0, 0);
//	sem_init(&procesos_en_exec, 0, 0);
//	sem_init(&procesos_en_suspendido, 0, 0);
//	sem_init(&procesos_en_exit, 0, 0);
//
//	sem_init(&disponible_para_exec, 0, 1);
//
//	pthread_t thread_new, thread_exec,thread_suspendido, thread_exit;
//	pthread_t thread_consola;
//
//	log_info(memoria_log, "Creo thread de new");
//	pthread_create(&thread_new, NULL, &thread_de_new, NULL);
//
//	log_info(memoria_log, "Creo thread de exec");
//	pthread_create(&thread_exec, NULL, &thread_de_exec, NULL);
//
//	log_info(memoria_log, "Creo thread de suspendido");
//	pthread_create(&thread_suspendido, NULL, &thread_de_suspendido, NULL);
//
//	log_info(memoria_log, "Creo thread de exit");
//	pthread_create(&thread_exit, NULL, &thread_de_exit, NULL);
//
//	log_info(memoria_log, "Creo thread de consola");
//	pthread_create(&thread_consola, NULL, &conectar_consola, puerto_escucha());
//
//	pthread_join(thread_new, NULL);
//	pthread_join(thread_exec, NULL);
//	pthread_join(thread_suspendido, NULL);
//	pthread_join(thread_exit, NULL);
//	pthread_join(thread_consola, NULL);
//
//}


int main(int argc, char** argv) {
	//funcion_prueba_checkpoint();

	log_kernel = log_create("kernel.log", "KERNEL", 1, LOG_LEVEL_DEBUG);

	if(argc < 2){
        log_error(log_kernel, "ERROR - Arhcivo de configuracion no especificado");
		log_destroy(log_kernel);
        return EXIT_FAILURE;
    }
	else if(argc > 2){
		log_error(log_kernel, "ERROR - Demasiados argumentos");
		log_destroy(log_kernel);
        return EXIT_FAILURE;
	}
	crear_config(argv[1]);
	//config = config_create("kernel.config");

	inicializar_estructuras();

	conectar_memoria(ip_memoria(), puerto_memoria());
	conectar_cpu(ip_cpu(), puerto_cpu_dispatch(), puerto_cpu_interrupt());
	inicializar_threads();
	int servidor = iniciar_servidor(puerto_escucha());
	conectar_consola(servidor);



	desconectar_memoria();

	log_destroy(memoria_log);
	config_destroy(config);

	return EXIT_SUCCESS;
}
