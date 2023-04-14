#include "../include/consola.h"

t_log *logger;

int main(int argc, char **argv) {
	FILE *f = son_argumentos_validos(argc, argv);
	if(!f) return EXIT_FAILURE;
	int socket_kernel = conectar_a_kernel();
	if(!conexion_exitosa(socket_kernel)) return EXIT_FAILURE;
	t_paquete *paquete = crear_paquete_instrucciones(f, atoi(argv[2]));
	enviar_paquete_instrucciones(paquete, socket_kernel);
	if(terminar(socket_kernel)) return EXIT_SUCCESS;
	return EXIT_FAILURE;
}

FILE *son_argumentos_validos(int cantidad_argumentos, char **argv) {
	logger = log_create("consola.log", "CONSOLA", true, LOG_LEVEL_INFO);

	if (cantidad_argumentos < 3) {
		cantidad_argumentos < 2 ?
				log_error(logger, "Archivo no especificado.\n") :
				log_error(logger, "Tamaño no especificado.\n");
		return NULL;
	}

	if (atoi(argv[2]) <= 0) {
		log_error(logger, "El tamaño es inválido.\n");
		return NULL;
	}

	FILE *f;
	if (!(f = fopen(argv[1], "r"))) {
		log_error(logger, "No se encontró el archivo de instrucciones");
		return NULL;
	}

	return f;
}
