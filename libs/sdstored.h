#ifndef SDSTORED_H
#define SDSTORED_H
#include "emExecucao.h"

int executeProcFileCommand(char *argv[], char *transformacoes[], int nrargs, int f);
void setTransConfig(char *configFile, int transConfig[]);
int verificaPedido (int transConfig[], int transNecess[]);
#endif