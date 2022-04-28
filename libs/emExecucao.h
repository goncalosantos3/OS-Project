#include "sdstored.h"

typedef struct pedidosEmExecucao{
    Pedido atual;
    PedidosEmExecucao prox;
} * PedidosEmExecucao;  

PedidosEmExecucao initEmExecucao();
int isEmptyEmExecucao(PedidosEmExecucao pexec);
void colocaEmExecucao(Pedido pe, PedidosEmExecucao pexec);
void verificaPedidosConcluidos(PedidosEmExecucao pexec);
