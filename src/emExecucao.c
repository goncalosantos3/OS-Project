#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "../libs/emExecucao.h"
#include "../libs/sdstored.h"

PedidosEmExecucao initEmExecucao(){
    PedidosEmExecucao pexec = NULL;
    return pexec;
}

int isEmptyEmExecucao(PedidosEmExecucao pexec){

    if(pexec->atual == NULL && pexec->prox==NULL){
        return 1;
    }
    return 0;
}

//Insere um novo pedido em execução na lista ligada à cabeça
PedidosEmExecucao colocaEmExecucao(Pedido pe, PedidosEmExecucao pexec, int transConfig[]){
    PedidosEmExecucao aux = malloc(sizeof(struct pedidosEmExecucao));
    aux->atual = pe;
    aux->prox=pexec;
    //Ao por em execução um novo pedido temos que reduzir o número de instâncias disponiveis de cada transformaçao
    for(int i=0;i<7;i++){
        transConfig[i] -= pe->transNecess[i];
    }
    return aux;
}


//Definir função que verifica quais os pedidos em execucao que já concluiram a sua execucao

PedidosEmExecucao verificaPedidosConcluidos(PedidosEmExecucao pexec, int transConfig[]){
    //Atravessa a lista ligada e verifica quais os pedidos que terminaram e quais não terminaram
    PedidosEmExecucao *aux = &pexec;

    while((*aux)!=NULL){
        if(waitpid((*aux)->atual->pid,NULL,WNOHANG)!=0){//O pedido já acabou
            //Manda mensagem para o cliente a dizer que o pedido acabou a sua execução
            //Como o pedido terminou a sua execução vamos aumentar o número de instâncias disponíveis de cada transformação
            for(int i=0;i<7;i++){
                transConfig[i] += (*aux)->atual->transNecess[i];
            }
            (*aux)=(*aux)->prox;//Retira o pedido que concluiu a sua execução da lista ligada
        }
        aux=&(*aux)->prox;
    }
    return pexec;
}
