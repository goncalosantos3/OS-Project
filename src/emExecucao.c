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

    if((pexec->atual == NULL && pexec->prox==NULL) || pexec==NULL){
        return 1;
    }
    return 0;
}

//Insere um novo pedido em execução na lista ligada à cabeça
void colocaEmExecucao(Pedido pe, PedidosEmExecucao *pexec, int transConfig[]){
    write(pe->fifo_ouput,"Pedido a ser processado\n", 25 * sizeof(char));

    PedidosEmExecucao novo = malloc(sizeof(struct pedidosEmExecucao));
    novo->atual = pe;
    novo->prox = (*pexec);
    //Ao por em execução um novo pedido temos que reduzir o número de instâncias disponiveis de cada transformaçao
    for(int i=0;i<7;i++){
        transConfig[i] -= pe->transNecess[i];
    }
    (*pexec) = novo;
}


//Definir função que verifica quais os pedidos em execucao que já concluiram a sua execucao

void verificaPedidosConcluidos(PedidosEmExecucao *pexec, int transConfig[]){
    //Atravessa a lista ligada e verifica quais os pedidos que terminaram e quais não terminaram

    while((*pexec)!=NULL){
        if(waitpid((*pexec)->atual->pid,NULL,WNOHANG)!=0){//O pedido já acabou
            write((*pexec)->atual->fifo_ouput,"Pedido concluído\n",18 * sizeof(char));
            //Como o pedido terminou a sua execução vamos aumentar o número de instâncias disponíveis de cada transformação
            for(int i=0;i<7;i++){
                transConfig[i] += (*pexec)->atual->transNecess[i];
            }
            (*pexec)=(*pexec)->prox;//Retira o pedido que concluiu a sua execução da lista ligada
        }else{
            pexec=&(*pexec)->prox;
        }
    }
}
