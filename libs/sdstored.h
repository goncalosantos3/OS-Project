#ifndef SDSTORE_H
#define SDSTORE_H

typedef struct pedido{
    int tampedido;
    int transNecess[7];
    char *pedido[];
}*Pedido;

int executeProcFileCommand(char *argv[], char *transformacoes[], int nrargs);
void setTransConfig(char *configFile, int transConfig[]);
int verificaPedido (int transConfig[], int transNecess[]);
void buildPedido(char *command, Pedido pe, int tampedido);

#endif