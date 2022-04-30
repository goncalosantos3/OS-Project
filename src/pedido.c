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
void buildPedido(char *command, Pedido pe, int tampedido, int f1){
    char *str1, *str2;
    int i=0;
    char fifo_name[30];

    str1=strdup(command);
    while((str2=strsep(&str1," "))!=NULL){
        pe->pedido[i]=str2;
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
        i++;
    }
    printf("ola\n");
    //Recebe o cliente o nome do fifo para enviar informação e abre o fifo
    read(f1,fifo_name,sizeof(fifo_name));
    printf("Nome do fifo -> %s\n", fifo_name);
    pe->fifo_ouput = open(fifo_name, O_WRONLY);
    if(pe->fifo_ouput==-1){
        printf("%s\n", strerror(errno));
    }

    pe->tampedido=tampedido;
    pe->pid=0;//Enquanto que o pedido não é executado o pid é 0
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
}
