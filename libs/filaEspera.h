#ifndef FILAESPERA_H
#define FILAESPERA_H

#include "sdstored.h"

typedef struct filaEspera{
    int sizeFila;       //Contêm o tamanho total da fila de espera
    int nrPedidosFila;  //Contêm o número atual de pedidos em fila de espera
    Pedido *fila;   
}* FilaEspera;

FilaEspera initFilaEspera();
int isEmptyFilaEspera(FilaEspera fesp);
void colocaFilaEspera(Pedido pe, FilaEspera fesp);
Pedido retiraFilaEspera(FilaEspera fesp);

#endif