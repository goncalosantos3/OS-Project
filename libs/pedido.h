#ifndef PEDIDO_H
#define PEDIDO_H

typedef struct pedido{
    int nrPedido;
    int prioridade;
    int fifo_ouput;    //File Descriptor do fifo que envia informação para o client
    int tampedido;
    int pid;
    int transNecess[7];
    int *pipe;
    char *pedido[];
}*Pedido;

void buildPedido(char *command, Pedido pe, int tampedido, int nrpedido, char *fifo_name, int f1, int *pipe);
void freePedido(Pedido pe);
void printPedido(Pedido pe);

#endif