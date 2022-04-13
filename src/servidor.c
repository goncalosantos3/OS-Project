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
    int i=0;
    int nragrs;int tamanho_args[5]; char *path;
    int f = open("fifo", O_RDONLY);

    printf("A ler número de args\n");
    read(f,&nragrs,sizeof(int));

    char *transformacoes[nragrs];
    char command[300]; char *str1; char *str2;
    read(f,command,sizeof(command));
    printf("%s\n", command);
    str1=strdup(command);
    while((str2=strsep(&str1," "))!=NULL){
        transformacoes[i++]=str2;
        printf("%s\n", transformacoes[i-1]);
    }
    int pipes[nragrs-3][2];
    for(int i=0;i<nragrs-3;i++){
        pipe(pipes[i]);
    }

    for(int i=0;i<nragrs-2;i++){//nragrs-2 porque duas strings não são transformacoes

        if(fork()==0 && i==0){//Primeiro comando
            path=strcat(argv[2],transformacoes[i+2]);
            printf("%s\n", path);
            close(pipes[0][0]);
            int f = open(transformacoes[0],O_RDONLY);
            dup2(f,0);
            dup2(pipes[0][1],1);
            close(f);
            close(pipes[0][1]); 
            if(execl(path,transformacoes[i+2],NULL)==-1){
                printf("%s\n", strerror(errno));
            }
            exit(1);
        }else if(fork()==0 && i>0 && i<nragrs-3){//Comandos intermédios
            path=strcat(argv[2],transformacoes[i+2]);
            close(pipes[i-1][1]);
            close(pipes[i][0]);
            dup2(pipes[i-1][0],0);
            dup2(pipes[i][1],1);
            close(pipes[i-1][0]);
            close(pipes[i][1]);
            if(execl(path,transformacoes[i+2],NULL)==-1){
                printf("%s\n", strerror(errno));
            }
            exit(1);
        }else if(fork()==0 && i==nragrs-3){//Último comando
            path=strcat(argv[2],transformacoes[i+2]);
            int f = open(transformacoes[1],O_CREAT | O_WRONLY | O_TRUNC, 0660);
            close(pipes[i-1][1]);
            dup2(pipes[i-1][0],0);
            dup2(f,1);
            close(f);
            close(pipes[i-1][0]);
            if(execl(path,transformacoes[i+2],NULL)==-1){
                printf("%s\n", strerror(errno));
            }
            exit(1);
        }
    }   
    wait(NULL);
    return 0;
}   
