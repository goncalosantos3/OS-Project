#include "sdstored.h"

typedef struct pedidosEmExecucao{
    int nrPedidosExecucao;
    int sizeArray;
    Pedido *emExecucao;
} * PedidosEmExecucao