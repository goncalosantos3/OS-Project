#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/*
*   O servidor tem como função receber as tarefas passadas pelo cliente e aplicar as 
*   devidas transformações aos ficheiros para executar as várias transformações pretendidas.
*   
*   O servidor recebe dois argumentos:
*   1º: Um ficheiro de configuração com as instâncias máximas permitidas para cada transformação.
*   2º: O caminho para a pasta onde se encontram os executáveis das transformações
*
*   Nome do executável do servidor: sdstored
*/


int main(int argc, char *argv[]){
    int nragrs;
    char *transformacoes[20];
    int f = open("fifo", O_RDONLY);

    read(f,&nragrs,sizeof(int));
    for(int i=0;i<nragrs;i++){
        //As duas primeiras strings deste array são os ficheiros de input e output 
        read(f,transformacoes[i],sizeof(transformacoes[i]));
    }
    int pipes[nragrs-3][2];
    for(int i=0;i<nragrs-3;i++){
        pipe(pipes[i]);
    }
    for(int i=0;i<nragrs-2;i++){//nragrs-2 porque duas strings não são transformacoes

        if(fork()==0 && i==0){//Primeiro comando
            close(pipes[0][0]);
            int f = open(transformacoes[0],O_RDONLY);
            dup2(f,0);
            dup2(pipes[0][1],1);
            close(f);
            close(pipes[0][1]); 
            execl(argv[2],transformacoes[i+2],NULL);
        }else if(fork()==0 && i>0 && i<nragrs-3){//Comandos intermédios
            close(pipes[i-1][1]);
            close(pipes[i][0]);
            dup2(pipes[i-1][0],0);
            dup2(pipes[i][1],1);
            close(pipes[i-1][0]);
            close(pipes[i][1]);
            execl(argv[2],transformacoes[i+2],NULL);
        }else if(fork()==0 && i==nragrs-3){//Último comando
            int f = open(transformacoes[1],O_CREAT | O_WRONLY | O_TRUNC, 0660);
            close(pipes[i-1][1]);
            dup2(pipes[i-1][0],0);
            dup2(f,1);
            close(f);
            close(pipes[i-1][0]);
            execl(argv[2],transformacoes[i+2],NULL);
        }
    }   
    wait(NULL);
}   