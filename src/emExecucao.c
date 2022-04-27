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
    PedidosEmExecucao pexec = malloc(sizeof(struct pedidosEmExecucao) + 30 * sizeof(Pedido));
    pexec->nrPedidosExecucao=0;
    pexec->sizeArray=30;

    for(int i=0; i<30; i++){
        pexec->emExecucao[i]=NULL;
    }
    return pexec;
}

int isEmptyEmExecucao(PedidosEmExecucao pexec){
    if(pexec->nrPedidosExecucao==0 && pexec->emExecucao[0]==NULL){
        return 1;
    }
    return 0;
}

void colocaEmExecucao(Pedido pe, PedidosEmExecucao pexec){

    if(pexec->nrPedidosExecucao==pexec->sizeArray){
        //O array da fila de espera está cheio
        pexec->emExecucao = realloc(pexec->emExecucao,sizeof(struct pedidosEmExecucao) + 2 * pexec->sizeArray * sizeof(Pedido));
        pexec->sizeArray*=2;
    }
    pexec->emExecucao[pexec->nrPedidosExecucao++]=pe;
}

//Definir método que verifica quais os pedidos em execucao que já concluiram a sua execucao