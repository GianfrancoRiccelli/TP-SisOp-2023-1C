#ifndef TLB_H_
#define TLB_H_

#include <commons/collections/list.h>
#include <stdlib.h>

#include "estructurascpu.h"

typedef struct {
    int pagina;
    int marco;
} t_tlb;

t_list* TLB;
int tlb_hits;
int acc_memoria;

int chequear_tlb(int pagina);
void actualizar_tlb(int pagina, int marco);

#endif
