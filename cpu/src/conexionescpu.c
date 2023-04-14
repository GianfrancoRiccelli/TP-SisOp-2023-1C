#include "../include/conexionescpu.h"

void *iniciar_conexion_memoria (){

    socket_memoria = -1;
    while((socket_memoria = crear_conexion(cpuconfig.ip_memoria, cpuconfig.puerto_memoria)) <= 0){
        log_warning(cpu_log, "No se ha podido conectar con la MEMORIA");
        sleep(5);
    }
    int* identificador = malloc(sizeof(int));
    *identificador = 0;
    send(socket_memoria, identificador, sizeof(int), 0);
    free (identificador);

    //[932][Tamaño][Entradas por tabla][Tamaño de pagina] son int
    if (recibir_operacion(socket_memoria) != 932){
        log_error(cpu_log, "ERROR - No se ha podido establecer una comunicacion confiable con la Memoria");
        return NULL; 
    }
    log_info(cpu_log, "Conexion con memoria exitosa!!!");
    memoria_log = log_create("logs/cpu_memoria.log","MEMORIA", true, LOG_LEVEL_INFO);

    t_buffer* buffer = malloc(sizeof(t_buffer));
    buffer-> stream = recibir_buffer(&(buffer -> size), socket_memoria);
    log_info(memoria_log, "Recibiendo Entradas por Tabla y Tamaño de pagina...");
    entradas_por_tabla = sacar_de_buffer(buffer, sizeof(int)); // Hay que hacer un free cuando lo termine de usar
    tam_de_pagina = sacar_de_buffer(buffer, sizeof(int)); // Hay que hacer un free cuando lo termine de usar
    log_info(memoria_log, "Listo!!!");
    log_info(memoria_log, "Entradas por tabla: %d \tTam Pagina: %d", *entradas_por_tabla, *tam_de_pagina);
    return NULL;
};


void *iniciar_conexion_dispatch(){
    //socket servidor
    int servidor_dispatch; 
    t_buffer* buffer = crear_buffer();
    t_pcb *pcb;
    log_info(cpu_log,"Iniciando conexion dispatch...");
    servidor_dispatch = iniciar_servidor(cpuconfig.puerto_escucha_dispatch);
    //
    log_info(cpu_log,"Esperando DISPATCH...");
    socket_dispatch = esperar_cliente(servidor_dispatch);
    log_info(cpu_log,"Conexion establecida!!!");
    log_info(cpu_log,"Esperando la conexion de interrupcion...");
    sem_wait(&mutex_interrupt);
    log_info(cpu_log,"Todo listo, inicializando...");
    sem_post(&mutex_interrupt);
    while(true){
        log_info(cpu_log,"No hay PCB, esperando...");
        if(recibir_operacion(socket_dispatch) == -1)
            break;
        buffer-> stream = recibir_buffer(&(buffer -> size), socket_dispatch);
        log_info(cpu_log,"Llego un PCB");
        pcb = desempaquetar_pcb(buffer->stream);
        free(buffer->stream);
        log_info(cpu_log,"PCB obtenido!!!");
        ciclo_de_instruccion(pcb);
    }

    free(buffer);
    log_error(cpu_log, "ERROR DISPATCH - El KERNEL se ha desconectado");
    return NULL;
};   

void *iniciar_conexion_interrupt(){
    //socket servidor 
    interrupt_log = log_create("logs/interrupt.log","INTERRUPT", true, LOG_LEVEL_INFO);
    log_info(interrupt_log, "Iniciando conexion INTERRUPT");
    int server_interrupt = iniciar_servidor(cpuconfig.puerto_escucha_interrupt);
    socket_interrupt = esperar_cliente(server_interrupt);
    log_info(interrupt_log, "Conexion Establecida!!!");
    bool *hay_interrupcion = malloc(sizeof(bool));
    sem_post(&mutex_interrupt);
    while(true){
        if(recv(socket_interrupt, hay_interrupcion, sizeof(bool), MSG_WAITALL) <= 0)
            break;
        sem_wait(&mutex_interrupt);
        log_info(interrupt_log, "El KERNEL ha solicitado una interrupcion");
        check_interrupt = *hay_interrupcion;
        sem_post(&mutex_interrupt);
    }

    log_error(cpu_log, "ERROR INTERRUPT - El KERNEL se ha desconectado");
    log_destroy(interrupt_log);
    free(hay_interrupcion);
    return NULL;
}
void iniciar_conexiones(){
    log_info(cpu_log, "Iniciando las conexiones...");
    iniciar_conexion_memoria();
    pthread_t dispatch;
    pthread_t interrupt;

    sem_init(&mutex_interrupt, 0, 0);
    pthread_create(&dispatch, NULL, (void *) iniciar_conexion_dispatch, NULL);
    pthread_create(&interrupt, NULL, (void *) iniciar_conexion_interrupt, NULL);

    pthread_join(dispatch, NULL);
    pthread_join(interrupt, NULL);
}
