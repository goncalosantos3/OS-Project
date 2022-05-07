#ifndef EMESPERA_H
#define EMESPERA_H

#include "sdstored.h"

typedef struct emEspera{
    Pedido atual;
    struct emEspera *prox;   
}* PedidosEmEspera;

PedidosEmEspera initEmEspera();
int isEmptyEmEspera(PedidosEmEspera esp);
void colocaEmEspera(Pedido pe, PedidosEmEspera *esp);
void retiraPedidosParaExecucao(PedidosEmEspera *esp, PedidosEmExecucao *pexec, int *transConfig, char *argv[]);
void printListaLigadaEmEspera(PedidosEmEspera esp);
#endif