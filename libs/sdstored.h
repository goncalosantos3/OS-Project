#ifndef SDSTORE_H
#define SDSTORE_H

typedef struct pedido{
    int fifo_input;    //File Descriptor do fifo que recebe informação do client
    int fifo_ouput;    //File Descriptor do fifo que envia informação para o client
    int tampedido;
    int pid;
    int transNecess[7];
    char *pedido[];
}*Pedido;

int executeProcFileCommand(char *argv[], char *transformacoes[], int nrargs, int f);
void setTransConfig(char *configFile, int transConfig[]);
int verificaPedido (int transConfig[], int transNecess[]);
void buildPedido(char *command, Pedido pe, int tampedido, int f1, int f2);

#endif