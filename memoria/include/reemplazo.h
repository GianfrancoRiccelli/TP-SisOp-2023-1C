#ifndef REEMPLAZO_H_
#define REEMPLAZO_H_
#include "estructuras.h"
#include "operaciones.h"
/**
	* @NAME: ejecutar_algoritmo_de_reemplazo
	* @DESC: Retorna el numero de marco
    * dada la pagina que quiere entrar a
    * memoria y su pid correspondiente, segun el
    * algoritmo a utilizar, se selecciona un
    * marco en memoria, se desaloja a quien lo
    * ocupa y ahora lo ocupa esta pagina
*/
int ejecutar_algoritmo_de_reemplazo(int pagina, int id_proceso);
#endif
