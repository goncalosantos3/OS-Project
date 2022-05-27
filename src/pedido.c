#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "../libs/pedido.h"

/*
*   Função que vai construir o array de string transformacoes com toda a informação necessária
*   para executar o pedido atual:
*   - tipo de operação (proc-file ou status)
*   - ficheiros input e output
*   - e finalmente todas as transformações a executar 
*/
void buildPedido(char *command, Pedido pe, int tampedido, int nrpedido, char *fifo_name, int f1, int *pipe){
    char *str1, *str2;
    int i=0;

    pe->prioridade=0;//Valor por defeito (inicial)
    str1=strdup(command);
    while((str2=strsep(&str1," "))!=NULL){
        if(strcmp(str2,"bcompress")==0){
            pe->transNecess[0]++;
        }else if(strcmp(str2,"bdecompress")==0){
            pe->transNecess[1]++;
        }else if(strcmp(str2,"decrypt")==0){
            pe->transNecess[2]++;
        }else if(strcmp(str2,"encrypt")==0){
            pe->transNecess[3]++;
        }else if(strcmp(str2,"gcompress")==0){
            pe->transNecess[4]++;
        }else if(strcmp(str2,"gdecompress")==0){
            pe->transNecess[5]++;
        }else if(strcmp(str2,"nop")==0){
            pe->transNecess[6]++;
        }

        if(strcmp(str2,"-p")==0){
            str2=strsep(&str1," ");   //str2 == prioridade
            pe->prioridade=atoi(str2);
        }else{
            pe->pedido[i++]=str2;
        }
    }
    //Recebe o cliente o nome do fifo para enviar itampedido
    pe->fifo_ouput = open(fifo_name, O_WRONLY);
    if(pe->fifo_ouput==-1){
        printf("%s\n", strerror(errno));
    }
    pe->nrPedido = nrpedido;
    pe->tampedido =tampedido;
    pe->pid =0;//Enquanto que o pedido não é executado o pid é 0
    pe->pipe = pipe;
}

//Esta função serve apenas para debug. Printa todos os parâmetros de um pedido
void printPedido(Pedido pe){
    printf("fifo_output->%d, tampedido->%d\n", pe->fifo_ouput, pe->tampedido);
    printf("TransNecess:\n");
    for(int i=0;i<7;i++){
        printf("%d ", pe->transNecess[i]);
    }
    printf("\n");
    for(int i=0;i<pe->tampedido;i++){
        printf("%s ", pe->pedido[i]);
    }
    printf("\n");
    printf("Prioridade-> %d\n", pe->prioridade);
    printf("NrPedido-> %d\n", pe->nrPedido);
    printf("Pipe-> %d %d\n", pe->pipe[0], pe->pipe[1]);
}