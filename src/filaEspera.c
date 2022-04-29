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
#include "../libs/filaEspera.h"

FilaEspera initFilaEspera(){
    FilaEspera fesp = malloc(sizeof(struct filaEspera));
    fesp->fila = malloc( 30 * sizeof(Pedido));
    fesp->nrPedidosFila=0;
    fesp->sizeFila=30;

    for(int i=0; i<30; i++){
        fesp->fila[i]=NULL;
    }
    return fesp;
}

int isEmptyFilaEspera(FilaEspera fesp){
    if(fesp->nrPedidosFila==0 && fesp->fila[0]==NULL){
        return 1;
    }
    return 0;
}

void colocaFilaEspera(Pedido pe, FilaEspera fesp){

    if(fesp->nrPedidosFila==fesp->sizeFila){
        //O array da fila de espera está cheio
        fesp->fila = realloc(fesp->fila,sizeof(struct filaEspera) + 2*fesp->sizeFila * sizeof(Pedido));
        fesp->sizeFila*=2;
    }
    fesp->fila[fesp->nrPedidosFila++]=pe;
}

Pedido retiraFilaEspera(FilaEspera fesp){
    Pedido proximo = fesp->fila[0];
    
    for(int i=0;i<fesp->nrPedidosFila-1;i++){
        fesp->fila[i]=fesp->fila[i+1];
    }
    fesp->nrPedidosFila--;
    return proximo;
}

//Implementar uma função que atravessa a queue e verifica quais os pedidos que podem ser executados
