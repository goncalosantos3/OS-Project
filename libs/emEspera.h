#ifndef FILAESPERA_H
#define FILAESPERA_H

#include "sdstored.h"

typedef struct emEspera{
    Pedido atual;
    struct emEspera *prox;   
}* PedidosEmEspera;

PedidosEmEspera initEmEspera();
int isEmptyEmEspera(PedidosEmEspera esp);
PedidosEmEspera colocaEmEspera(Pedido pe, PedidosEmEspera esp);
PedidosEmEspera retiraPedidosParaExecucao(PedidosEmEspera esp, PedidosEmExecucao pexec, int transConfig[], char *argv[]);

#endif