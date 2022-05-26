#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "../libs/funcsAux.h"
#include "../libs/emExecucao.h"
#include "../libs/sdstored.h"

PedidosEmExecucao initEmExecucao(){
    PedidosEmExecucao pexec = NULL;
    return pexec;
}

int isEmptyEmExecucao(PedidosEmExecucao pexec){

    if((pexec==NULL) || (pexec->atual == NULL && pexec->prox==NULL)){
        return 1;
    }
    return 0;
}

//Insere um novo pedido em execução na lista ligada à cabeça
void colocaEmExecucao(Pedido pe, PedidosEmExecucao *pexec, int *transConfig, char *argv[]){
    write(pe->fifo_ouput,"Pedido a ser processado\n", 25 * sizeof(char));

    //Coloca o comando a executar
    pe->pid = executeProcFileCommand(argv,pe->pedido,pe->tampedido);

    PedidosEmExecucao novo = malloc(sizeof(struct pedidosEmExecucao));
    novo->atual = pe;
    while((*pexec)!=NULL && (*pexec)->atual->prioridade > pe->prioridade){
        pexec = &(*pexec)->prox;
    }
    novo->prox = (*pexec);
    (*pexec) = novo;
    //Ao por em execução um novo pedido temos que reduzir o número de instâncias disponiveis de cada transformaçao
    for(int i=0;i<7;i++){
        transConfig[i] -= pe->transNecess[i];
    }
}

void retiraPedidoConcluido(int pid, PedidosEmExecucao *pexec, int *transConfig){
    PedidosEmExecucao aux;

    while((*pexec) != NULL && (*pexec)->atual->pid != pid){
        pexec = &(*pexec)->prox;
    }

    if((*pexec) != NULL){
        write((*pexec)->atual->fifo_ouput,"Pedido concluído\n", 18 * sizeof(char));
        close((*pexec)->atual->fifo_ouput);
        //Envia o número de bytes input para o cliente (Funcionalidade avançada)
        if(fork()==0){
            dup2((*pexec)->atual->fifo_ouput,1);
            execlp("wc","wc","-c",(*pexec)->atual->pedido[1],NULL);
            exit(1);
        }
        //Envia o número de bytes output para o cliente (Funcionalidade avançada)
        if(fork()==0){
            dup2((*pexec)->atual->fifo_ouput,1);
            execlp("wc","wc","-c",(*pexec)->atual->pedido[2],NULL);
            exit(1);
        }
        for(int i=0;i<7;i++){
            transConfig[i] += (*pexec)->atual->transNecess[i];
        }
        aux = (*pexec);
        (*pexec)=(*pexec)->prox;
        //Retira o pedido que concluiu a sua execução da lista ligada
        free(aux);
        //Liberta a memória associada a esse pedido;
    }
}

//Definir função que verifica quais os pedidos em execucao que já concluiram a sua execucao

void verificaPedidosConcluidos(PedidosEmExecucao *pexec, int *transConfig){
    //Atravessa a lista ligada e verifica quais os pedidos que terminaram e quais não terminaram
    PedidosEmExecucao aux;

    while((*pexec)!=NULL){
        if(waitpid((*pexec)->atual->pid, NULL, WNOHANG) != 0){//O pedido já acabou
            write((*pexec)->atual->fifo_ouput,"Pedido concluído\n", 18 * sizeof(char));
            //Envia o número de bytes input para o cliente (Funcionalidade avançada)
            if(fork()==0){
                dup2((*pexec)->atual->fifo_ouput,1);
                execlp("wc","wc","-c",(*pexec)->atual->pedido[1],NULL);
            }
            //Envia o número de bytes output para o cliente (Funcionalidade avançada)
            if(fork()==0){
                dup2((*pexec)->atual->fifo_ouput,1);
                execlp("wc","wc","-c",(*pexec)->atual->pedido[2],NULL);
            }
            //Como o pedido terminou a sua execução vamos aumentar o número de instâncias disponíveis de cada transformação
            for(int i=0;i<7;i++){
                transConfig[i] += (*pexec)->atual->transNecess[i];
            }
            close((*pexec)->atual->fifo_ouput);
            aux = (*pexec);
            (*pexec) = (*pexec)->prox;
            //Retira o pedido que concluiu a sua execução da lista ligada
            free(aux);
            //Liberta a memória associada a esse pedido;
        }else{
            pexec=&(*pexec)->prox;
        }
    }
}

void printListaLigadaEmExecucao(PedidosEmExecucao pexec){
    PedidosEmExecucao aux = pexec;

    while(aux!=NULL){
        printf("%d ", aux->atual->prioridade);
        aux=aux->prox;
    }
}