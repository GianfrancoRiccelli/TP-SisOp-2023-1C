#include "../include/crear_paquete.h"

void empaquetar_linea(t_paquete *paquete, char **instruccion) {
	uint8_t identificador = get_identificador(instruccion[0]);

	if (identificador == NO_RECONOCIDO) {
		log_error(logger, "Identificador \"%s\" no reconocido", instruccion[0]);
		return;
	}

	uint8_t cant_parametros = get_cant_parametros(identificador);
	uint32_t *parametros;

	if (!cant_parametros) parametros = NULL;
	else {
		parametros = (uint32_t *) malloc(sizeof(uint32_t) * cant_parametros);
		for (int j = 0; j < cant_parametros; j++)
			parametros[j] = atoi(instruccion[j + 1]);
	}

	t_instruccion *i = crear_instruccion(identificador, cant_parametros, parametros);

	if (identificador == NO_OP)
		for (int j = 0; j < atoi(instruccion[1]); j++)
			empaquetar_instruccion(paquete, i);
	else empaquetar_instruccion(paquete, i);

	liberar_instruccion(i);
}

t_paquete *crear_paquete_instrucciones(FILE *f, uint32_t tamanio) {
	log_info(logger, "Creando paquete de instrucciones...");
	t_paquete *paquete = crear_paquete(INSTRUCCIONES_CONSOLA);

	agregar_a_paquete(paquete, &tamanio, sizeof(uint32_t));

	char *linea = NULL;
	size_t lon = 0;

	while (getline(&linea, &lon, f) != -1) {
		empaquetar_linea(paquete, string_split(linea, " "));
	}

	fclose(f);
	free(linea);

	log_info(logger, "Archivo de instrucciones leído correctamente");
	log_info(logger, "Paquete creado exitosamente");
	return paquete;
}
