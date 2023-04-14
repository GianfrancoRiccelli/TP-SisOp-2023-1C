#include "../include/memoria.h"

void imprimir_puntero_void(void* a_imprimir, int tamanio_a_imprimir) {
	for(int i = 0; i < tamanio_a_imprimir; i++)
		printf("%p\n", a_imprimir + i);
	printf("\n");
}

void funcion_test1(){
	void* proceso = malloc(16);
	imprimir_puntero_void(proceso, 16);
	imprimir_puntero_void(proceso, configuracion->tam_pagina);
	t_list *proceso1 = agregar_proceso(1, 16, proceso);
	t_list *proceso2 = agregar_proceso(2, 33, proceso);
	printf("Puntero proceso 2: %p\n", proceso2);
	free(proceso);
	proceso = malloc(12);
	t_list *proceso3 = agregar_proceso(3, 12, proceso);
	printf("Puntero proceso 3: %p\n", proceso3);
	free(proceso);

	t_list *tabla2 = obtener_tabla2(proceso1, 1);
	printf("%p\n", tabla2);
	int marco = obtener_marco(tabla2, 1);
	printf("%d\n", marco);
	int direccion = (marco * configuracion->tam_pagina);

	proceso = leer_de_memoria(direccion, configuracion->tam_pagina);
	imprimir_puntero_void(proceso, configuracion->tam_pagina);
	free(proceso);

	//finalizar_proceso(proceso1);

	//Prueba Suspender proceso
	t_list* tabla1_ptr = proceso1;
		for(int i = 0; i < 16/configuracion->tam_pagina; i++){
			t_list* tabla2_ptr = list_get(tabla1_ptr, i / configuracion->entradas_por_tabla);
			t_tabla2* pagina_ptr = list_get(tabla2_ptr, i % configuracion->entradas_por_tabla);
			printf("Proceso: %d, pagina: %d, bit de presencia: %d\n", pagina_ptr->id, pagina_ptr->pagina, pagina_ptr->P);
		}
	obtener_marco(tabla2, 0);
	for(int i = 0; i < 16/configuracion->tam_pagina; i++){
				t_list* tabla2_ptr = list_get(tabla1_ptr, i / configuracion->entradas_por_tabla);
				t_tabla2* pagina_ptr = list_get(tabla2_ptr, i % configuracion->entradas_por_tabla);
				printf("Proceso: %d, pagina: %d, bit de presencia: %d\n", pagina_ptr->id, pagina_ptr->pagina, pagina_ptr->P);
			}
	suspender_proceso2(proceso1);
	for(int i = 0; i < 16/configuracion->tam_pagina; i++){
		t_list* tabla2_ptr = list_get(tabla1_ptr, i / configuracion->entradas_por_tabla);
		t_tabla2* pagina_ptr = list_get(tabla2_ptr, i % configuracion->entradas_por_tabla);
		printf("Proceso: %d, pagina: %d, bit de presencia: %d\n", pagina_ptr->id, pagina_ptr->pagina, pagina_ptr->P);
	}




	//Prueba lectura y escritura de memoria
//	 int a = 55456;
//	 void* numero = &a;
//	 for(int i = 0; i < 4; i++){
//	 	escribir_a_memoria(direccion + i, 1, numero + i);
//	 	proceso = leer_de_memoria(direccion, configuracion->tam_pagina);
//	 	imprimir_puntero_void(proceso, configuracion->tam_pagina);
//	 	free(proceso);
//	 }
//	 proceso = leer_de_memoria(direccion, sizeof(int));
//	 int b = *(int*) numero;
//	 printf("%d\n", b);
//
//
//	 tabla2 = obtener_tabla2(proceso3, 1);
//	 printf("%d\n", tabla2);
//	 marco = obtener_marco(tabla2, 0);
//	 printf("%d\n", marco);
//	 direccion = (marco * configuracion->tam_pagina);
//
//	 int c = 99999;
//	 numero = &c;
//	 for(int i = 0; i < 4; i++){
//	 	escribir_a_memoria(direccion + i, 1, numero + i);
//	 	proceso = leer_de_memoria(direccion, configuracion->tam_pagina);
//	 	imprimir_puntero_void(proceso, configuracion->tam_pagina);
//	 	free(proceso);
//	 }
//	 proceso = leer_de_memoria(direccion, sizeof(int));
//	 int d = *(int*) numero;
//	 printf("%d\n", d);



//	printf("Direccion tabla1 proceso 1: %d\n", proceso1);
//	printf("Direccion tabla1 proceso 2: %d\n", proceso2);
//	printf("Direccion tabla1 proceso 3: %d\n", proceso3);
//	printf("direccion tabla2: %d\n", tabla2);
}

int main(int argc, char** argv) {
    configuracion = NULL;

	if(argc < 2){
        puts("ERROR - Arhcivo de configuracion no especificado");
        return EXIT_FAILURE;
    }
    else if(argc > 2){
        puts("ERROR - Demasiados argumentos");
        return EXIT_FAILURE;
    }

    iniciar_estructuras(argv[1]);
	iniciar_conexiones(configuracion);

    if(!configuracion)
        return EXIT_FAILURE;

    printf("Memoria finalizo correctamente");

    //iniciar_conexion(configuracion_memoria);

    //funcion_test1();

    //t_tabla tablas_de_paginas[entradas_por_tabla][entradas_por_tabla];
    //No se puede usar parametros para crear arrays o tablas de tamaño variable
    //memset(tablas_de_paginas,0,entradas_por_tabla*entradas_por_tabla*sizeof(t_tabla));

    //memoria_principal = malloc(tam_memoria);

    return EXIT_SUCCESS;
}
