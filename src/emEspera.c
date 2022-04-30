#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "../libs/sdstored.h"
#include "../libs/emEspera.h"

PedidosEmEspera initEmEspera(){
    PedidosEmEspera esp = NULL;
    return esp;
}

int isEmptyEmEspera(PedidosEmEspera esp){

    if((esp->atual == NULL && esp->prox==NULL) || (esp==NULL)){
        return 1;
    }
    return 0;
}

void colocaEmEspera(Pedido pe, PedidosEmEspera *esp){
    write(pe->fifo_ouput,"Pedido em fila de espera\n", 25 * sizeof(char));

    PedidosEmEspera aux = malloc(sizeof(struct emEspera));
    aux->atual=pe;
    aux->prox=(*esp);
    (*esp)=aux;
}

//Função que atravessa a lista ligada dos pedidos que estão em espera e retira os pedidos 
//que podem ir para execução inserindo-os na lista ligada de pedidos para execução
void retiraPedidosParaExecucao(PedidosEmEspera *esp, PedidosEmExecucao pexec, int transConfig[], char *argv[]){

    while((*esp)!=NULL){
        if(verificaPedido(transConfig,(*esp)->atual->transNecess)==1){
            //O pedido pode ser executado
            executeProcFileCommand(argv,(*esp)->atual->pedido,(*esp)->atual->tampedido);
            colocaEmExecucao((*esp)->atual,&pexec,transConfig);
            (*esp)=(*esp)->prox;
        }else{
            esp=&(*esp)->prox;
        }
    }
}