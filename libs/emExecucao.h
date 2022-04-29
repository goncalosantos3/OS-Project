#include "sdstored.h"

typedef struct pedidosEmExecucao{
    Pedido atual;
    PedidosEmExecucao prox;
} * PedidosEmExecucao;  

PedidosEmExecucao initEmExecucao();
int isEmptyEmExecucao(PedidosEmExecucao pexec);
PedidosEmExecucao colocaEmExecucao(Pedido pe, PedidosEmExecucao pexec, int transConfig[]);
PedidosEmExecucao verificaPedidosConcluidos(PedidosEmExecucao pexec, int transConfig[]);
