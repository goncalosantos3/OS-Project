#ifndef PEDIDO_H
#define PEDIDO_H

typedef struct pedido{
    int fifo_ouput;    //File Descriptor do fifo que envia informação para o client
    int tampedido;
    int pid;
    int transNecess[7];
    char *pedido[];
}*Pedido;

void buildPedido(char *command, Pedido pe, int tampedido, int f1);
void printPedido(Pedido pe);

#endif