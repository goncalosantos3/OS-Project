#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "emExecucao.h"

PedidosEmExecucao initEmExecucao(){
    PedidosEmExecucao pexec = malloc(sizeof(struct pedidosEmExecucao));
    pexec->atual=NULL;  
    return pexec;
}

int isEmptyEmExecucao(PedidosEmExecucao pexec){

    if(pexec->atual == NULL){
        return 1;
    }
    return 0;
}

void colocaEmExecucao(Pedido pe, PedidosEmExecucao pexec){

    PedidosEmExecucao aux = malloc(sizeof(struct pedidosEmExecucao));
    aux->atual = pe;
    aux->prox=pexec;
    pexec=aux;
}


//Definir função que verifica quais os pedidos em execucao que já concluiram a sua execucao

void verificaPedidosConcluidos(PedidosEmExecucao pexec){
    //Atravessa a lista ligada e verifica quais os pedidos que terminaram e quais não terminaram
}