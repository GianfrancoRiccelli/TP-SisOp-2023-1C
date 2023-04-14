#include "../include/cpu.h"

int main(int argc, char** argv){

    if(argc < 2){
        puts("ERROR - Arhcivo de configuracion no especificado");
        return EXIT_FAILURE;
    }
    else if(argc > 2){
        puts("ERROR - Demasiados argumentos");
        return EXIT_FAILURE;
    }

	t_config *config = cargar_config(argv[1]);
    TLB = list_create();
    iniciar_conexiones();
    config_destroy(config);
    log_destroy(memoria_log);
    log_info(cpu_log, "El CPU finalizo correctamente");
    log_destroy(cpu_log);
};
