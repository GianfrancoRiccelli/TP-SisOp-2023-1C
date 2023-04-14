#include "../include/tlb.h"

int chequear_tlb(int pagina){
    t_tlb* entrada_tlb = NULL;
    int indice = 0;
    bool no_se_encontro=true;
    t_list_iterator* iterador_tlb = list_iterator_create(TLB);

    log_warning(cpu_log, "TLB:");
    while(list_iterator_has_next(iterador_tlb)){
        entrada_tlb = list_iterator_next(iterador_tlb);    
        indice = iterador_tlb->index;
        log_warning(cpu_log, "%d \tP:%d \tM:%d",indice,entrada_tlb->pagina,entrada_tlb->marco);
        if(entrada_tlb->pagina == pagina){
            log_warning(cpu_log,"TLB HIT!");
            tlb_hits++;
            no_se_encontro=false;
            break;
        }
    }
    
    list_iterator_destroy(iterador_tlb);
    if(no_se_encontro)
        return -1;
    
    if(strcmp((cpuconfig.reemplazo_tlb),"LRU")==0){
        list_remove(TLB, indice);
        list_add(TLB,entrada_tlb);
    }
    
    return entrada_tlb->marco;
}

void actualizar_tlb(int pagina, int marco){

    t_tlb* nueva_entrada = malloc(sizeof(t_tlb));
    nueva_entrada->pagina = pagina;
    nueva_entrada->marco = marco;

    t_list_iterator* iterador_tlb;

    iterador_tlb = list_iterator_create(TLB);

    while(list_iterator_has_next(iterador_tlb)){
        if( ( (t_tlb*)list_iterator_next(iterador_tlb) )->marco == marco ){
            list_replace_and_destroy_element(TLB,iterador_tlb->index,nueva_entrada,free);
            list_iterator_destroy(iterador_tlb);
            return;
        }
    }
    if(list_size(TLB) < cpuconfig.entradas_tlb){
        list_add(TLB, nueva_entrada);
        return;
    }
    list_remove_and_destroy_element(TLB, 0,free);
    list_add(TLB, nueva_entrada);
    return;
}

