#include "../include/reemplazo.h"

//int clock_normal(int id_proceso, int pagina){
//
//    //int size_tabla_planificacion = list_size(tabla_planificacion);
//    //Tengo entendido que el tamaño de esta tabla es el que puse aca
//    int size_tabla_planificacion = (configuracion->tam_memoria / configuracion->tam_pagina);
//    t_tabla2 *pagina_actual;
//
//    //Si en un ciclo previo se lo aumento a mas de la cantidad de marcos, se lo devuelve a 0
//    if(planificacion_ptr == size_tabla_planificacion)
//    	planificacion_ptr = 0;
//
//    sem_wait(&mutex_tabla_planificacion);
//    for(int j=0; j<2; j++){
//        for(;planificacion_ptr < size_tabla_planificacion ;planificacion_ptr++){//Recorre toda la lista de marcos creada por vos
//
//            pagina_actual = list_get(tabla_planificacion, planificacion_ptr);
//            if (!pagina_actual){ //Si el marco actual esta vacio se lo llena
//            	leer_pagina_SWAP(id_proceso, pagina, planificacion_ptr);
//            	planificacion_ptr++;
//                sem_post(&mutex_tabla_planificacion);
//            	return planificacion_ptr - 1;
//            }
//
//            if (pagina_actual->U)
//                pagina_actual->U = false; //Si U=1 -> U=0
//            else{
//                //Se escribe la pagina en swap por si hubo modificacion:
//                escribir_pagina_SWAP(pagina_actual->id, pagina_actual->pagina, pagina_actual->marco);
//                //Se lee la pagina de swap y se la pone en el marco (en memoria):
//                leer_pagina_SWAP(id_proceso, pagina, pagina_actual -> marco);
//                //Ahora se marca esa pagina como ausente, se deja la actualizacion de las tablas con la nueva pagina cargada a cargo
//                //de quien me haya llamado y tenga los datos necesarios, yo no los tengo
//                pagina_actual -> P = false;
//                //Se aumenta el planificacion_ptr para que apunte al proximo marco
//                planificacion_ptr++;
//                //Devuelvo el marco para que los que me llamaron sepan cual de todos elegi reemplazar:
//                sem_post(&mutex_tabla_planificacion);
//                return pagina_actual -> marco;
//            }
//        }
//        planificacion_ptr = 0;
//    }//El for de "j" asegura que si no se puede elegir un marco para reemplazar a la primera vuelta, se haga una segunda.
//    //Si a las 2da vuelta no se reemplaza nada, se rompe todo
//    sem_post(&mutex_tabla_planificacion);
//    return -1;
//
//}

//int clock_modificado(int id_proceso, int pagina){
//    //int size_tabla_planificacion = list_size(tabla_planificacion);
//    //Tengo entendido que el tamaño de esta tabla es el que puse aca
//    int size_tabla_planificacion = (configuracion->tam_memoria / configuracion->tam_pagina);
//    t_tabla2 *pagina_actual;
//
//    if(planificacion_ptr == size_tabla_planificacion)
//    	planificacion_ptr = 0;
//
//    sem_wait(&mutex_tabla_planificacion);
//    for (int j=0; j<2; j++){
//        for(int i = 0; i < size_tabla_planificacion; i++){//Recorre toda la lista de marcos creada por vos
//
//            pagina_actual = list_get(tabla_planificacion, planificacion_ptr);
//            if (!pagina_actual){//Si el marco esta vacio se lo llena
//            	leer_pagina_SWAP(id_proceso, pagina, planificacion_ptr);
//            	planificacion_ptr++;
//                sem_post(&mutex_tabla_planificacion);
//            	return planificacion_ptr - 1;
//
//            }
//
//            if(!pagina_actual->U && !pagina_actual->M){//Si los dos son 0 da falso y se reemplaza
//                leer_pagina_SWAP(id_proceso, pagina, pagina_actual -> marco);//En este caso no escribimos porque M=0
//                pagina_actual -> P = false;
//                planificacion_ptr++;
//                sem_post(&mutex_tabla_planificacion);
//                return pagina_actual -> marco;
//            }
//
//            if(planificacion_ptr == size_tabla_planificacion - 1)
//            	planificacion_ptr = 0;
//            else
//            	planificacion_ptr++;
//        }//Si llega aca, estamos en el paso (2)
//
//        for(int i=0; i < size_tabla_planificacion ;i++){//Recorre toda la lista de marcos creada por vos
//
//            pagina_actual = list_get(tabla_planificacion, planificacion_ptr);
//            if (!pagina_actual){//Si es null se salta la iteracion
//            	if(planificacion_ptr == size_tabla_planificacion - 1)
//            		planificacion_ptr = 0;
//            	else
//            	    planificacion_ptr++;
//            	continue;
//            }
//
//            if(pagina_actual->M && !pagina_actual->U){//Solo se cumple si M=1 y U=0
//                escribir_pagina_SWAP(id_proceso, pagina, pagina_actual -> marco);//Ahora si escribimos
//                leer_pagina_SWAP(id_proceso, pagina, pagina_actual -> marco);
//                pagina_actual -> P = false;
//                planificacion_ptr++;
//                sem_post(&mutex_tabla_planificacion);
//                return pagina_actual -> marco;
//            }
//            else
//                pagina_actual -> U = false; //En este paso si se cambian los bits U
//            if(planificacion_ptr == size_tabla_planificacion - 1)
//            	planificacion_ptr = 0;
//            else
//            	planificacion_ptr++;
//        }
//    }//Si llega aca tiene que hacer otra vuelta, sino se rompe todo como antes
//    sem_post(&mutex_tabla_planificacion);
//    return -1;
//}

int encontrar_grupo_de_marcos_de_proceso(int id_proceso){
	int size_tabla_planificacion = (configuracion->tam_memoria / configuracion->tam_pagina);
	t_tabla2 *pagina_actual;
	int inicio_grupo_marcos = 0;


	sem_wait(&mutex_tabla_planificacion);

	//Encuentra el grupo de marcos asignados a el proceso
	//Se asume que no van a llegar mas procesos de los que puede contener la memoria
	while(inicio_grupo_marcos < size_tabla_planificacion){
		pagina_actual = list_get(tabla_planificacion, inicio_grupo_marcos);
		if(pagina_actual && pagina_actual->id == id_proceso){
			sem_post(&mutex_tabla_planificacion);
			log_info(cpu_log, "Grupo de marcos del PID: %d encontrado en marcos %d a %d",id_proceso, inicio_grupo_marcos, inicio_grupo_marcos + configuracion->marcos_por_proceso-1);
			return inicio_grupo_marcos;
		}
		inicio_grupo_marcos += configuracion->marcos_por_proceso;
	}

	//De no encontrarselo se asigna el primer grupo vacio al proceso
	inicio_grupo_marcos = 0;
	while(inicio_grupo_marcos < size_tabla_planificacion){
		pagina_actual = list_get(tabla_planificacion, inicio_grupo_marcos);
		if(!pagina_actual){
			sem_post(&mutex_tabla_planificacion);
			log_info(cpu_log, "Marcos %d a %d asignado al PID: %d", inicio_grupo_marcos, inicio_grupo_marcos + configuracion->marcos_por_proceso-1,id_proceso);
			return inicio_grupo_marcos;
		}
		inicio_grupo_marcos += configuracion->marcos_por_proceso;
	}
	//Si llega aca ocurrio un error
	sem_post(&mutex_tabla_planificacion);
	log_error(cpu_log, "ERROR - No se encontro espacio disponible para el PID: %d", id_proceso);
	return -1;
}

int clock_normal(int id_proceso, int pagina, int marco_inicio){
	int posicion_ptr = marco_inicio/configuracion->marcos_por_proceso;
	t_tabla2 *pagina_actual;

	if(planificacion_ptrs[posicion_ptr] == configuracion->marcos_por_proceso)
		planificacion_ptrs[posicion_ptr] = 0;

	sem_wait(&mutex_tabla_planificacion);

	//Chequeo si hay marcos vacios y lleno el primero que encuentre
	for(int i= 0; planificacion_ptrs[posicion_ptr] + i < configuracion->marcos_por_proceso; i++){
		pagina_actual = list_get(tabla_planificacion, marco_inicio + planificacion_ptrs[posicion_ptr] + i);
		if(!pagina_actual){
			sem_post(&mutex_tabla_planificacion);
			leer_pagina_SWAP(id_proceso, pagina, marco_inicio + planificacion_ptrs[posicion_ptr]);
			log_info(cpu_log, "Marco vacio encontrado en marco %d", marco_inicio + planificacion_ptrs[posicion_ptr]);
			planificacion_ptrs[posicion_ptr] +=  ++i;
			return marco_inicio + planificacion_ptrs[posicion_ptr] - 1;
		}
	}

	for(int i=0; i<2; i++){
		for(; planificacion_ptrs[posicion_ptr] < configuracion->marcos_por_proceso; planificacion_ptrs[posicion_ptr]++){
			pagina_actual = list_get(tabla_planificacion, marco_inicio + planificacion_ptrs[posicion_ptr]);
			log_info(cpu_log, "Imprimo las posiciones que estoy chequeando:");
			log_info(cpu_log, "%d \t PAG: %d  U=%d", marco_inicio+planificacion_ptrs[posicion_ptr], pagina_actual->pagina, pagina_actual->U);

			if(pagina_actual->U){
				pagina_actual->U = false;
				log_info(cpu_log, "Bit de uso modificado en marco %d", pagina_actual->marco);
			}else{
				log_info(cpu_log, "Marco a reemplazar encontrado en marco %d", pagina_actual->marco);
				sem_post(&mutex_tabla_planificacion);
				//Se escribe la pagina en swap por si hubo modificacion:
			    escribir_pagina_SWAP(pagina_actual->id, pagina_actual->pagina, pagina_actual->marco);
				//Espera por acceso a disco
				usleep(1000 * configuracion->retardo_swap);

			    //Se lee la pagina de swap y se la pone en el marco (en memoria):
			    leer_pagina_SWAP(id_proceso, pagina, pagina_actual -> marco);
			    //Ahora se marca esa pagina como ausente, se deja la actualizacion de las tablas con la nueva pagina cargada a cargo
			    //de quien me haya llamado y tenga los datos necesarios, yo no los tengo
			    pagina_actual -> P = false;
			    //Se aumenta el planificacion_ptr para que apunte al proximo marco
			    planificacion_ptrs[posicion_ptr]++;
			    //Devuelvo el marco para que los que me llamaron sepan cual de todos elegi reemplazar:
			    return pagina_actual -> marco;
			}
		}
		planificacion_ptrs[posicion_ptr] = 0;
	}
	//El for de "j" asegura que si no se puede elegir un marco para reemplazar a la primera vuelta, se haga una segunda.
	//Si a las 2da vuelta no se reemplaza nada, se rompe todo
	sem_post(&mutex_tabla_planificacion);
	log_error(cpu_log, "ERROR - Marco valido no encontrado tras 2 iteraciones");

	return -1;
}

int clock_modificado(int id_proceso, int pagina, int marco_inicio){
	int posicion_ptr = marco_inicio/configuracion->marcos_por_proceso;
	t_tabla2 *pagina_actual;

	if(planificacion_ptrs[posicion_ptr] == configuracion->marcos_por_proceso)
		planificacion_ptrs[posicion_ptr] = 0;

	sem_wait(&mutex_tabla_planificacion);

	//Chequeo si hay marcos vacios y lleno el primero que encuentre
	for(int i=0; planificacion_ptrs[posicion_ptr] + i < configuracion->marcos_por_proceso; i++){
		pagina_actual = list_get(tabla_planificacion, marco_inicio + planificacion_ptrs[posicion_ptr] + i);
		if(!pagina_actual){
			sem_post(&mutex_tabla_planificacion);
			log_info(cpu_log, "Marco vacio encontrado en marco %d", marco_inicio + planificacion_ptrs[posicion_ptr]);
			leer_pagina_SWAP(id_proceso, pagina, marco_inicio + planificacion_ptrs[posicion_ptr]);
			planificacion_ptrs[posicion_ptr] += ++i;
			return marco_inicio + planificacion_ptrs[posicion_ptr] - 1;
		}
	}

	for(int i=0; i<2; i++){
		log_info(cpu_log, "Ejecuto paso 1");
		log_info(cpu_log, "Imprimo las posiciones que estoy chequeando: ");
		for(int j=0; j < configuracion->marcos_por_proceso; j++){//Recorre todo el grupo de marcos
			pagina_actual = list_get(tabla_planificacion, marco_inicio + planificacion_ptrs[posicion_ptr]);
			log_info(cpu_log, "%d \t PAG: %d  U=%d  M=%d", marco_inicio+planificacion_ptrs[posicion_ptr], pagina_actual->pagina, pagina_actual->U, pagina_actual->M);

			if(!pagina_actual->U && !pagina_actual->M){//Si los dos son 0 da falso y se reemplaza
				sem_post(&mutex_tabla_planificacion);
				log_info(cpu_log, "Marco a reemplazar encontrado en marco %d", pagina_actual->marco);
				leer_pagina_SWAP(id_proceso, pagina, pagina_actual -> marco);//En este caso no escribimos porque M=0
				pagina_actual -> P = 0;
				planificacion_ptrs[posicion_ptr]++;
			    return pagina_actual -> marco;
			}

			if(planificacion_ptrs[posicion_ptr] == configuracion->marcos_por_proceso - 1)
				planificacion_ptrs[posicion_ptr] = 0;
			else
				planificacion_ptrs[posicion_ptr]++;
		}//Si llega aca estamos en el paso (2)

		log_info(cpu_log, "Ejecuto paso 2");
		for(int j=0; j<configuracion->marcos_por_proceso; j++){
			pagina_actual = list_get(tabla_planificacion, marco_inicio + planificacion_ptrs[posicion_ptr]);
			log_info(cpu_log, "%d \t PAG: %d  U=%d  M=%d", marco_inicio+planificacion_ptrs[posicion_ptr], pagina_actual->pagina, pagina_actual->U, pagina_actual->M);

			if(pagina_actual->M && !pagina_actual->U){//Solo se cumple si M=1 y U=0
				sem_post(&mutex_tabla_planificacion);
				log_info(cpu_log, "Marco a reemplazar encontrado en marco %d", pagina_actual->marco);
				escribir_pagina_SWAP(id_proceso, pagina, pagina_actual -> marco);//Ahora si escribimos
				//Espera por acceso a disco
				usleep(1000 * configuracion->retardo_swap);

			    leer_pagina_SWAP(id_proceso, pagina, pagina_actual -> marco);
			    pagina_actual -> P = 0;
				pagina_actual -> M = 0;
			    planificacion_ptrs[posicion_ptr]++;
			    return pagina_actual -> marco;
			}
			pagina_actual -> U = 0; //En este paso si se cambian los bits U
			log_info(cpu_log, "Bit de uso modificado en marco %d", pagina_actual->marco);

			if(planificacion_ptrs[posicion_ptr] == configuracion->marcos_por_proceso - 1)
				planificacion_ptrs[posicion_ptr] = 0;
			else
				planificacion_ptrs[posicion_ptr]++;
		}
	}//Si llega aca se rompe
	sem_post(&mutex_tabla_planificacion);
	log_error(cpu_log, "ERROR - Marco valido no encontrado tras 2 iteraciones");
	return -1;

}

int ejecutar_algoritmo_de_reemplazo(int pagina, int id_proceso){

	int marco_inicial = encontrar_grupo_de_marcos_de_proceso(id_proceso);
	if(marco_inicial == -1)
		return -1;

	log_info(cpu_log, "Ejecuto algoritmo de reemplazo");
    if (strcmp(configuracion -> algoritmo_reemplazo, "CLOCK") == 0)
        return clock_normal(id_proceso, pagina, marco_inicial);

    return clock_modificado(id_proceso, pagina, marco_inicial);

}

