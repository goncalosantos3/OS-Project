#ifndef EMEXECUCAO_H
#define EMEXECUCAO_H
#include "pedido.h"

typedef struct pedidosEmExecucao{
    Pedido atual;
    struct pedidosEmExecucao * prox;
}*PedidosEmExecucao;

PedidosEmExecucao initEmExecucao();
int isEmptyEmExecucao(PedidosEmExecucao pexec);
void colocaEmExecucao(Pedido pe, PedidosEmExecucao *pexec, int transConfig[]);
void verificaPedidosConcluidos(PedidosEmExecucao *pexec, int transConfig[]);

#endif