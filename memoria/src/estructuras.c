#include "../include/estructuras.h"


void iniciar_estructuras(char* config_path){
	cargar_configuraciones(config_path);
	memoria_principal = calloc(1, configuracion->tam_memoria);
	log_info(memoria_log, "Memoria principal creada");
	//tablas = list_create();

	page_fault_counter = 0;
	pid_actual = -1;

	sem_init(&mutex_pf, 0, 1);
	sem_init(&mutex_pid_actual, 0, 1);

	tabla_planificacion = list_create();
	for(int i = 0; i < configuracion->tam_memoria / configuracion->tam_pagina; i++){
		t_tabla2* tabla2 = NULL;
		list_add(tabla_planificacion, tabla2);
	}
	log_info(memoria_log, "Tabla de planificacion de %d marcos creada", list_size(tabla_planificacion));
	int tamanio_vector_ptrs_marcos = list_size(tabla_planificacion)/configuracion->marcos_por_proceso;
	planificacion_ptrs = malloc(tamanio_vector_ptrs_marcos * sizeof(int));
	for(int i = 0; i < tamanio_vector_ptrs_marcos; i++){
		planificacion_ptrs[i] = 0;
	}
	log_info(memoria_log, "Vector de punteros de planificacion para %d grupos de marcos creado", tamanio_vector_ptrs_marcos);
}

void cargar_configuraciones(char* path){
    t_config *config_memoria = NULL;
    //TODO Liberar configuracion cuando se finalize el programa
    configuracion = malloc(sizeof(t_config_memoria));

    memoria_log = log_create("memoria.log","MEMORIA",1,LOG_LEVEL_INFO);

    if(!(config_memoria = config_create(path))){
        log_error(memoria_log,"ERROR - No se encontro el archivo de configuracion de memoria");
        return;
    }

    if(!es_configuracion_valida(config_memoria)) 
        return;

    log_info(memoria_log,"Leyendo archivo de configuracion:");
    configuracion->tam_memoria = config_get_int_value(config_memoria,"TAM_MEMORIA");
    log_info(memoria_log,"TAM_MEMORIA: %d", configuracion->tam_memoria);
    configuracion->tam_pagina = config_get_int_value(config_memoria,"TAM_PAGINA");
    log_info(memoria_log,"TAM_PAGINA: %d", configuracion->tam_pagina);
    configuracion->entradas_por_tabla = config_get_int_value(config_memoria,"ENTRADAS_POR_TABLA");
    log_info(memoria_log,"ENTRADAS_POR_TABLA: %d", configuracion->entradas_por_tabla);
    configuracion->retardo_memoria = config_get_int_value(config_memoria,"RETARDO_MEMORIA");
    log_info(memoria_log,"RETARDO_MEMORIA: %d", configuracion->retardo_memoria);
    configuracion->retardo_swap = config_get_int_value(config_memoria,"RETARDO_SWAP");
    log_info(memoria_log,"RETARDO_SWAP: %d", configuracion->retardo_swap);
    configuracion->marcos_por_proceso = config_get_int_value(config_memoria,"MARCOS_POR_PROCESO");
    log_info(memoria_log, "MARCOS_POR_PROCESO: %d", configuracion->marcos_por_proceso);
	configuracion->puerto = string_new();//Lo cambie por string new para poder destruir el config tranquilo, sin destruir la estructura
	string_append(&(configuracion->puerto), config_get_string_value(config_memoria,"PUERTO_ESCUCHA"));
	log_info(memoria_log,"PUERTO_ESCUCHA: %s", configuracion->puerto);
	configuracion->algoritmo_reemplazo = string_new();
	string_append(&(configuracion->algoritmo_reemplazo), config_get_string_value(config_memoria,"ALGORITMO_REEMPLAZO"));
	log_info(memoria_log,"ALGORITMO_REEMPLAZO: %s", configuracion->algoritmo_reemplazo);
	configuracion->path_swap = string_new();
	string_append(&(configuracion->path_swap), config_get_string_value(config_memoria,"PATH_SWAP"));
	log_info(memoria_log,"PATH_SWAP: %s", configuracion->path_swap);
	config_destroy(config_memoria);
    
    log_info(memoria_log, "Listo!\n");
}

bool es_configuracion_valida(t_config *config_memoria){
    char claves_validas[9][20+1] = {"TAM_MEMORIA","TAM_PAGINA","ENTRADAS_POR_TABLA",
                                "RETARDO_MEMORIA","RETARDO_SWAP","MARCOS_POR_PROCESO",
                                "PUERTO_ESCUCHA","ALGORITMO_REEMPLAZO","PATH_SWAP"};
    for(int i = 0; i < 0; i++){
        if (!config_has_property(config_memoria, claves_validas[i])){
            log_error(memoria_log,"ERROR - Archivo de configuracion invalido\n");
            return false;
        }
    }
    if (config_keys_amount(config_memoria) >9 ){
        log_error(memoria_log,"ERROR - Archivo de configuracion invalido\n");
        return false;
    }
    return true;
}

FILE* encontrar_SWAP(int id_proceso){
	char* SWAP_path = string_from_format("%s/%d.swap",
				configuracion->path_swap,
				id_proceso);
	FILE* archivo_swap;
	if(!access(SWAP_path, F_OK)){
		archivo_swap =  fopen(SWAP_path, "rb+");
		free(SWAP_path);
		return archivo_swap;
	}
	archivo_swap = fopen(SWAP_path, "wb+");
	free(SWAP_path);
	return archivo_swap;
}

void crear_SWAP(int id_proceso, int tamanio_proceso, void* proceso){
	//Crear directorios si no existen?
	FILE* SWAP_file = encontrar_SWAP(id_proceso);
	if(!SWAP_file){
		log_error(kernel_log, "ERROR - No se pudo crear el archivo .swap");
		return;
	}
	log_info(kernel_log, "Archivo .swap creado");
	fwrite(proceso, tamanio_proceso, 1, SWAP_file);
	//En caso de que el tamanio del archivo no sea divisible por el tamanio de una pagina, se lo redondea
	if(tamanio_proceso % configuracion->tam_pagina != 0){
		int resto = tamanio_proceso % configuracion->tam_pagina;
		void *relleno = malloc(resto);
		fwrite(relleno, resto, 1, SWAP_file);
		free(relleno);
	}
	log_info(kernel_log, "Archivo .swap llenado");

	fclose(SWAP_file);

}



t_tabla2* inicializar_tabla2(int id_proceso, int numero_pagina){
	t_tabla2* tabla = malloc(sizeof(t_tabla2));
	tabla->M = 0;
	tabla->P = 0;
	tabla->U = 0;
	tabla->id = id_proceso;
	tabla->pagina = numero_pagina;
	tabla->marco = -1;
	log_info(kernel_log, "Metadata pagina nro %d creada", numero_pagina);
	return tabla;
}

t_list *crear_metadata(int tamanio_proceso, int id_proceso){
	int cant_paginas = ceil((double) tamanio_proceso / (double) configuracion->tam_pagina);
	int cant_tablas2 = ceil((double) cant_paginas / (double) configuracion->entradas_por_tabla);
	int pagina_counter = 0;

	//Verifico que el tamaño del proceso no sea mayor al maximo
	if(cant_tablas2 > configuracion->entradas_por_tabla){
		log_error(kernel_log, "ERROR - Tamanio de proceso mayor al permitido");
		return 0;
	}

	t_list* tabla1 = list_create();

	for(int i = 0; i < cant_tablas2; i++){
		t_list* tabla2 = list_create();
		log_info(kernel_log, "Tabla NVL 2 nro %d creada", i);
		for(int j = 0; j < configuracion->entradas_por_tabla; j++){
			if(pagina_counter < cant_paginas){
				list_add(tabla2, inicializar_tabla2(id_proceso, pagina_counter));
				pagina_counter++;
			}
		}
		list_add(tabla1, tabla2);
	}

	//list_add(tablas, tabla1);
	return tabla1;
}

t_list *agregar_proceso(int id_proceso, int tamanio_proceso, void* proceso){
	log_info(kernel_log, "Id del proceso: %d, Tamanio proceso: %d", id_proceso, tamanio_proceso);
	t_list *direccion_tabla = crear_metadata(tamanio_proceso, id_proceso);
	if(!direccion_tabla){
		return(direccion_tabla);
	}
	log_info(kernel_log, "Tabla NVL 1 creada en %X", (int)direccion_tabla);

	crear_SWAP(id_proceso, tamanio_proceso, proceso);

	return direccion_tabla;
}




//Funciones viejas

//void crear_metadata(int id_proceso, int tamanio_proceso){
//	proceso* nuevo = malloc(sizeof(proceso));
//	nuevo->id = id_proceso;
//	nuevo->tablas1 = list_create();
//
//
//	int cant_paginas = ceil(tamanio_proceso / configuracion_memoria->tam_pagina);
//	int cant_tablas2 = ceil(cant_paginas / configuracion_memoria->entradas_por_tabla);
//	int cant_tablas1 = ceil(cant_tablas2 / configuracion_memoria->entradas_por_tabla);
//	int page_counter = 0;
//	int tabla2_counter = 0;
//	for(int i = 0; i < cant_tablas1; i++){
//		t_list* tabla1 = list_create();
//		for(int j = 0; j < configuracion_memoria->entradas_por_tabla; j++){
//			if(tabla2_counter < cant_tablas2){
//				t_list* tabla2 = list_create();
//				for(int w = 0; w < configuracion_memoria->entradas_por_tabla; w++){
//					if(page_counter < cant_paginas){
//					list_add(tabla2, inicializar_tabla2());
//					page_counter++;
//					}
//				}
//				list_add(tabla1, tabla2);
//				tabla2_counter++;
//			}
//		}
//		list_add(nuevo->tablas1, tabla1);
//	}
//	list_add(procesos, nuevo);
//}
