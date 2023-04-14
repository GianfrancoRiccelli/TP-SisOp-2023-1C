#include "../../../include/conexiones.h"
#include <commons/collections/list.h>
#define N 10
int tablas_procesos[N];
sem_t sincro_tabla_proceso;
t_log *test_cpu;
t_log *test_kernel;

bool pedir_escritura(int socket_memoria, int tam_escribir, uint32_t dato_a_escribir, int direccion_fisica){
    
    bool* confirmacion = malloc(sizeof(bool));
    t_paquete *paq_instr_memoria = crear_paquete(ESCRIBIR);
    log_info(test_cpu, "Preparando paquete para memoria");
    agregar_a_paquete(paq_instr_memoria, &direccion_fisica, sizeof(int));
    agregar_a_paquete(paq_instr_memoria, &tam_escribir, sizeof(int));
    agregar_a_paquete(paq_instr_memoria, &dato_a_escribir, sizeof(uint32_t));
    log_info(test_cpu, "Enviando paquete a memoria...");
    enviar_paquete(paq_instr_memoria, socket_memoria);
    eliminar_paquete(paq_instr_memoria);

    log_info(test_cpu, "Esperando respuesta...");
    recv(socket_memoria, confirmacion, sizeof(bool), MSG_WAITALL);
    log_info(test_cpu, "Respuesta recibida!!!");
    log_info(test_cpu, "La confirmacion recibida es: %d", *confirmacion);
    bool retorno = *confirmacion;
    free(confirmacion);
    return retorno;
}
uint32_t pedir_lectura(int socket_memoria, int tam_leer, int direccion_fisica){
    uint32_t *valor_leido = malloc(sizeof(int));

    t_paquete *paq_instr_memoria = crear_paquete(LEER);
    log_info(test_cpu, "Preparando paquete para memoria");
    agregar_a_paquete(paq_instr_memoria, &direccion_fisica, sizeof(int));
    agregar_a_paquete(paq_instr_memoria, &tam_leer, sizeof(int));
    log_info(test_cpu, "Enviando paquete a memoria...");
    enviar_paquete(paq_instr_memoria, socket_memoria);
    eliminar_paquete(paq_instr_memoria);

    log_info(test_cpu, "Esperando respuesta...");
    recv(socket_memoria, valor_leido, sizeof(uint32_t), MSG_WAITALL);
    log_info(test_cpu, "Respuesta recibida!!!");
    log_info(test_cpu, "El valor leido es: %d", *valor_leido);
    int retorno = *valor_leido;
    free(valor_leido);
    return retorno;

}
int pedir_marco(int socket_memoria, int direccion_tabla_2, int entrada_lvl_2){
    int *marco = malloc(sizeof(int));

    t_paquete *paq_instr_memoria = crear_paquete(DEVOLVER_MARCO);
    log_info(test_cpu, "Preparando paquete para memoria");
    agregar_a_paquete(paq_instr_memoria, &direccion_tabla_2, sizeof(int));
    agregar_a_paquete(paq_instr_memoria, &entrada_lvl_2, sizeof(int));
    log_info(test_cpu, "Enviando paquete a memoria...");
    enviar_paquete(paq_instr_memoria, socket_memoria);
    eliminar_paquete(paq_instr_memoria);

    log_info(test_cpu, "Esperando respuesta...");
    recv(socket_memoria, marco, sizeof(int), MSG_WAITALL);
    log_info(test_cpu, "Respuesta recibida!!!");
    log_info(test_cpu, "El marco es: %d", *marco);
    int retorno = *marco;
    free(marco);
    return retorno;
}
int pedir_tabla_2(int socket_memoria, int tabla_del_proceso, int entrada_lvl_1){
    int *nro_tabla_2 = malloc(sizeof(int));
    t_paquete *paq_instr_memoria = crear_paquete(DEVOLVER_INDICE_TABLA_NVL2);
    log_info(test_cpu, "Preparando paquete para memoria");
    agregar_a_paquete(paq_instr_memoria, &tabla_del_proceso, sizeof(int));
    agregar_a_paquete(paq_instr_memoria, &entrada_lvl_1, sizeof(int));
    log_info(test_cpu, "Enviando paquete a memoria...");
    enviar_paquete(paq_instr_memoria, socket_memoria);
    eliminar_paquete(paq_instr_memoria);

    log_info(test_cpu, "Esperando respuesta...");
    recv(socket_memoria, nro_tabla_2, sizeof(int), MSG_WAITALL);
    log_info(test_cpu, "Respuesta recibida!!!");
    log_info(test_cpu, "El numero de tabla 2 es: %X", *nro_tabla_2);

    int retorno = *nro_tabla_2;
    free(nro_tabla_2);
    return retorno;
}
int obtener_direccion_fisica(int socket_memoria, int pid, int direccion_logica, int entradas_por_tabla, int tam_pagina){

    int nro_de_pagina;
    int entrada_lvl_1;
    int entrada_lvl_2;
    int desplazamiento;
    int nro_tabla_2;
    int marco;

    nro_de_pagina = floor( ((double) direccion_logica) / ((double) tam_pagina));
    entrada_lvl_1 = floor( ((double) nro_de_pagina) / ((double) entradas_por_tabla));
    entrada_lvl_2 = nro_de_pagina % (entradas_por_tabla);
    desplazamiento = direccion_logica - nro_de_pagina * (tam_pagina);

    nro_tabla_2 = pedir_tabla_2(socket_memoria, tablas_procesos[pid], entrada_lvl_1);
    marco = pedir_marco(socket_memoria, nro_tabla_2, entrada_lvl_2);
    return (marco*tam_pagina) + desplazamiento;
}
void crear_proceso(int socket_memoria, int pid, int tam_proceso){
    int *tabla_proceso = malloc(sizeof(int));
    log_info(test_kernel, "Creando Proceso...");
    log_info(test_kernel, "Enviando paquete a memoria, espero por nueva tabla");
    t_paquete *instrucciones_a_memoria = crear_paquete(CREAR_NUEVA_TABLA);
    agregar_a_paquete(instrucciones_a_memoria, &pid, sizeof(int));
    agregar_a_paquete(instrucciones_a_memoria, &tam_proceso, sizeof(int));
    enviar_paquete(instrucciones_a_memoria, socket_memoria);
    eliminar_paquete(instrucciones_a_memoria);
    
    log_info(test_kernel, "Esperando respuesta...");
    recv(socket_memoria, tabla_proceso, sizeof(int), MSG_WAITALL);
    log_info(test_kernel, "Respuesta Recibida!!!");
    tablas_procesos[pid] = *tabla_proceso;
    log_info(test_kernel, "Mi nueva tabla para el proceso %d es: %X", pid, tablas_procesos[pid]);

    log_info(test_kernel, "Señal al CPU para que lea la tabla");
    sem_post(&sincro_tabla_proceso);
    free(tabla_proceso);

    return;
}
bool boletear_proceso(int socket_memoria, int pid){
    log_info(test_kernel, "Finalizando Proceso...");
    log_info(test_kernel, "Enviando paquete a memoria, espero confirmacion de boleteamiento");
    t_paquete *instrucciones_a_memoria = crear_paquete(LIBERAR);
    agregar_a_paquete(instrucciones_a_memoria, &(tablas_procesos[pid]), sizeof(int));
    enviar_paquete(instrucciones_a_memoria, socket_memoria);
    eliminar_paquete(instrucciones_a_memoria);

    bool* confirmacion = malloc(sizeof(bool));

    log_info(test_kernel, "Esperando respuesta...");
    recv(socket_memoria, confirmacion, sizeof(bool), MSG_WAITALL);
    log_info(test_kernel, "Respuesta Recibida!!!");
    log_info(test_kernel, "Habemos confirmacion de boleteamiento: %d", *confirmacion);

    bool retorno = *confirmacion;
    free(confirmacion);
    return retorno;

}
bool suspension_proceso(int socket_memoria, int pid){
    log_info(test_kernel, "Finalizando Proceso...");
    log_info(test_kernel, "Enviando paquete a memoria, espero confirmacion de suspendimiento");
    t_paquete *instrucciones_a_memoria = crear_paquete(PROCESO_SUSPENDIDO);
    agregar_a_paquete(instrucciones_a_memoria, &(tablas_procesos[pid]), sizeof(int));
    enviar_paquete(instrucciones_a_memoria, socket_memoria);
    eliminar_paquete(instrucciones_a_memoria);

    bool* confirmacion = malloc(sizeof(bool));

    log_info(test_kernel, "Esperando respuesta...");
    recv(socket_memoria, confirmacion, sizeof(bool), MSG_WAITALL);
    log_info(test_kernel, "Respuesta Recibida!!!");
    log_info(test_kernel, "Habemos confirmacion de suspendimiento: %d", *confirmacion);

    bool retorno = *confirmacion;
    free(confirmacion);
    return retorno;

}
void *simulacion_kernel(void* nada){
    test_kernel = log_create ("test_cpu.log", "KERNEL_TEST", true, LOG_LEVEL_INFO);
    log_info(test_kernel, "Iniciando test de KERNEL...");

    char ip[]="127.0.0.2",
        puerto_memoria[]="8002";

    int socket_memoria;
    socket_memoria = crear_conexion(ip,puerto_memoria);

    while(socket_memoria == -1){
        log_warning(test_kernel, "No se ha podido conectar a memoria. Reintentando...");
        sleep(5);
        socket_memoria = crear_conexion(ip,puerto_memoria);
    }
    log_info(test_kernel, "Conexion exitosa!!!");

    int handshake = 1;
    
    bool *confirmacion = malloc(sizeof(bool));
    
    t_buffer *buffer = malloc(sizeof(t_buffer));

    log_info(test_kernel, "Enviando mensaje...");
    send(socket_memoria, &handshake, sizeof(int), 0);
    log_info(test_kernel, "Mensaje enviado!");
    log_info(test_kernel, "Esperando respuesta...");
    recv(socket_memoria, confirmacion, sizeof(bool), MSG_WAITALL);
    log_info(test_kernel, "Respuesta recibida!!!");

    printf("El valor de operacion es %d\n", *confirmacion);
    if (confirmacion)
        log_info(test_kernel, "Conexion exitosa!!!");
    else{
        log_error(test_kernel, "Ha habido un error en la comunicacion");
        return NULL;
    }
    //PID=0 TAM=10
    int procesos[N] = {10,5,10,7,20,33,21,25,10.10};//pid es la posicion i y procesos[i] es el tamaño
    int pid = 0;//0<pid<N
    crear_proceso(socket_memoria, pid, procesos[pid]);

    free(confirmacion);
    free(buffer);
    sleep(10);
    liberar_conexion(socket_memoria);
    return NULL;
}
void *simulacion_cpu(void* nada){
    test_cpu = log_create ("test_cpu.log", "CPU_TEST", true, LOG_LEVEL_INFO);
    log_info(test_cpu, "Iniciando test de CPU...");

    char ip[]="127.0.0.1",
        puerto_memoria[]="8002";

    int socket_memoria;
    socket_memoria = crear_conexion(ip,puerto_memoria);

    while(socket_memoria == -1){
        log_warning(test_cpu, "No se ha podido conectar a memoria. Reintentando...");
        sleep(5);
        socket_memoria = crear_conexion(ip,puerto_memoria);
    }
    log_info(test_cpu, "Conexion exitosa!!!");

    int handshake = 0,
        operacion,
        *entradas_por_tabla,
        *tam_pagina;

    t_buffer *buffer = malloc(sizeof(t_buffer));

    log_info(test_cpu, "Enviando mensaje...");
    send(socket_memoria, &handshake, sizeof(int), 0);
    log_info(test_cpu, "Mensaje enviado!");
    log_info(test_cpu, "Esperando respuesta...");
    operacion = recibir_operacion(socket_memoria);
    log_info(test_cpu, "Respuesta recibida!!!");

    printf("El valor de operacion es %d\n", operacion);
    if (operacion == 932)
        log_info(test_cpu, "Conexion exitosa!!!");
    else{
        log_error(test_cpu, "Ha habido un error en la comunicacion");
        return NULL;
    }

    log_info(test_cpu, "Recibiendo buffer...");
    buffer -> stream = recibir_buffer(&(buffer->size), socket_memoria);
    log_info(test_cpu, "Buffer Recibido!");

    printf("Size del buffer es %d \n", buffer -> size);
    log_info(test_cpu, "Sacando datos del buffer...");

    entradas_por_tabla = sacar_de_buffer(buffer, sizeof(int));
    tam_pagina = sacar_de_buffer(buffer, sizeof(int));

    log_info(test_cpu, "Datos extraidos!!!");
    printf("Las paginas son de %d bytes y hay %d entradas por tabla", *tam_pagina, *entradas_por_tabla);

    //Simulaciones de instruccion del proceso 5
    log_info(test_cpu, "Esperando que el Kernel escriba en la variable compartida");
    sem_wait(&sincro_tabla_proceso);
    log_info(test_cpu, "Ya puedo leer la tabla");
    log_info(test_cpu, "La tabla del pid 0 es: %X", tablas_procesos[0]);

    int pid;
    int direccion_logica;
    int direccion_fisica;
    int tam_leer;
    int tam_escribir;
    uint32_t dato_a_escribir;
    
    uint32_t valor_leido;
    bool confirmacion;

    //Simulo que recibo una instruccion WRITE 5 15 del proceso 0
    pid = 0;
    direccion_logica = 5;
    dato_a_escribir = 15;
    tam_escribir = sizeof(uint32_t);

    direccion_fisica = obtener_direccion_fisica(socket_memoria, pid, direccion_logica, *entradas_por_tabla, *tam_pagina);
    confirmacion = pedir_escritura(socket_memoria, tam_escribir, dato_a_escribir, direccion_fisica);

    //Simulo que recibo una instruccion READ 5 del proceso 0
    pid = 0;
    direccion_logica = 5;
    tam_leer = sizeof(uint32_t);

    direccion_fisica = obtener_direccion_fisica(socket_memoria, pid, direccion_logica, *entradas_por_tabla, *tam_pagina);
    valor_leido = pedir_lectura(socket_memoria, tam_leer, direccion_fisica);

    free(entradas_por_tabla);
    free(tam_pagina);
    free(buffer);
    sleep(10);
    liberar_conexion(socket_memoria);
    return NULL;
}
/*
int main(void){
    pthread_t hilo_cpu;
    pthread_t hilo_kernel;
    sem_init(&sincro_tabla_proceso, 0, 0);

    pthread_create(&hilo_cpu, NULL, &simulacion_cpu, NULL);
    pthread_create(&hilo_kernel, NULL, &simulacion_kernel, NULL);

    pthread_join(hilo_cpu, NULL);
    pthread_join(hilo_kernel, NULL);

    return EXIT_SUCCESS;
}*/
