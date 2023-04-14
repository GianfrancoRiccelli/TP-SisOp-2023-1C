#include "../include/operaciones.h"

bool pedir_escritura(int tam_escribir, void* dato_a_escribir, int direccion_fisica){
    t_paquete *paq_instr_memoria = crear_paquete(ESCRIBIR);

    log_debug(cpu_log, "Preparando paquete para memoria");
    agregar_a_paquete(paq_instr_memoria, &direccion_fisica, sizeof(int));
    agregar_a_paquete(paq_instr_memoria, &tam_escribir, sizeof(int));
    agregar_a_paquete(paq_instr_memoria, dato_a_escribir, tam_escribir);

    log_debug(cpu_log, "Enviando paquete a memoria...");
    enviar_paquete(paq_instr_memoria, socket_memoria);

    log_info(cpu_log, "Esperando respuesta de memoria...");
    bool confirmacion;
    recv(socket_memoria, &confirmacion, sizeof(bool), MSG_WAITALL);
    log_debug(cpu_log, "Respuesta recibida!!!");
    log_debug(cpu_log, "La confirmacion recibida es: %d", confirmacion);

    return confirmacion;
}
void* pedir_lectura(int tam_leer, int direccion_fisica){
    t_paquete *paq_instr_memoria = crear_paquete(LEER);

    log_debug(cpu_log, "Preparando paquete para memoria");
    agregar_a_paquete(paq_instr_memoria, &direccion_fisica, sizeof(int));
    agregar_a_paquete(paq_instr_memoria, &tam_leer, sizeof(int));

    log_debug(cpu_log, "Enviando paquete a memoria...");
    enviar_paquete(paq_instr_memoria, socket_memoria);

    log_info(cpu_log, "Esperando respuesta de memoria...");
    void *valor_leido = malloc(tam_leer);
    recv(socket_memoria, valor_leido, tam_leer, MSG_WAITALL);
    log_debug(cpu_log, "Respuesta recibida!!!");

    return valor_leido;
}
int pedir_marco(int direccion_tabla_2, int entrada_lvl_2) {
    t_paquete *paq_instr_memoria = crear_paquete(DEVOLVER_MARCO);
    log_debug(cpu_log, "Preparando paquete para memoria");
    agregar_a_paquete(paq_instr_memoria, &direccion_tabla_2, sizeof(int));
    agregar_a_paquete(paq_instr_memoria, &entrada_lvl_2, sizeof(int));
    log_debug(cpu_log, "Enviando paquete a memoria...");
    enviar_paquete(paq_instr_memoria, socket_memoria);

    log_info(cpu_log, "Esperando respuesta de memoria...");
    int marco;
    recv(socket_memoria, &marco, sizeof(int), MSG_WAITALL);
    log_debug(cpu_log, "Respuesta recibida!!!");
    log_info(cpu_log, "El marco es: %d", marco);

    return marco;
}
int pedir_tabla_2(int tabla_del_proceso, int entrada_lvl_1){
    int nro_tabla_2;
    t_paquete *paq_instr_memoria = crear_paquete(DEVOLVER_INDICE_TABLA_NVL2);
    log_debug(cpu_log, "Preparando paquete para memoria");
    agregar_a_paquete(paq_instr_memoria, &tabla_del_proceso, sizeof(int));
    agregar_a_paquete(paq_instr_memoria, &entrada_lvl_1, sizeof(int));
    log_debug(cpu_log, "Enviando paquete a memoria...");
    enviar_paquete(paq_instr_memoria, socket_memoria);

    log_info(cpu_log, "Esperando respuesta de memoria...");
    recv(socket_memoria, &nro_tabla_2, sizeof(int), MSG_WAITALL);
    log_debug(cpu_log, "Respuesta recibida!!!");
    log_info(cpu_log, "El numero de tabla 2 es: %X", nro_tabla_2);

    return nro_tabla_2;
}
int obtener_direccion_fisica(int tabla_paginas, uint32_t direccion_logica, int entradas_por_tabla, int tam_pagina){

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

    log_warning(cpu_log,"Pido Pagina: %d",nro_de_pagina);

    marco = chequear_tlb(nro_de_pagina);
    if(marco < 0){
        acc_memoria += 2;
        nro_tabla_2 = pedir_tabla_2(tabla_paginas, entrada_lvl_1);
        marco = pedir_marco(nro_tabla_2, entrada_lvl_2);
        actualizar_tlb(nro_de_pagina, marco);
    }
    return (marco*tam_pagina) + desplazamiento;
}
