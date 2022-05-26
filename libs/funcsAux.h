#include "pedido.h"
#include "emExecucao.h"

int executeProcFileCommand(char *argv[], char *transformacoes[], int nrargs);
void setTransConfig(char *configFile, int *transConfig);
int verificaPedido (int *transConfig, int transNecess[]);
void statusServer(Pedido pe, PedidosEmExecucao pexec, int *maxTrans, int *transConfig);