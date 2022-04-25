#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

typedef struct pedido{
    int tampedido;
    int transNecess[7];
    char *pedido[];
}*Pedido;

typedef struct filaEspera{
    int nrPedidosFila;
    Pedido fila[30];   
}*FilaEspera;

int executeProcFileCommand(char *argv[], char *transformacoes[], int nrargs);
void setTransConfig(char *configFile, int transConfig[]);
void setTransformacoesArray(char *transformacoes[], char *command, int transNecess[]);
int verificaPedido (int transConfig[], int transNecess[]);