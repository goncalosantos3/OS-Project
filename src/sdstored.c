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

void executeProcFileCommand(char *argv[], char *transformacoes[], int nrargs){
    char *path;

    if(nrargs>=4){//Só precisamos de pipes se houverem 2 ou mais transformações
        int pipes[nrargs-3][2];
            for(int i=0;i<nrargs-3;i++){
                pipe(pipes[i]);
        }
        for(int i=0;i<nrargs-2;i++){//nragrs-2 porque duas strings não são transformacoes
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
            }else if(fork()==0 && i>0 && i<nrargs-3){//Comandos intermédios
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
            }else if(fork()==0 && i==nrargs-3){//Último comando
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
    }else if(nrargs==3){
        if(fork()==0){
            path =strcat("/home/goncalo/Desktop/SO/Trabalho-de-SO/",argv[2]);
            path = strcat(path,"/");
            path = strcat(path,transformacoes[2]);
            printf("%s\n", path);
            int f1=open(transformacoes[0],O_RDONLY);
            if(f1==-1){
                printf("%s\n", strerror(errno));
            }
            int f2=open(transformacoes[1],O_CREAT | O_WRONLY | O_TRUNC, 0660);
            if(f2==-1){
                printf("%s\n", strerror(errno));
            }
            dup2(f1,0);
            dup2(f2,1);
            close(f1);close(f2);
            if(execl(path,transformacoes[2],NULL)==-1){
                    printf("%s\n", strerror(errno));
            }
            exit(1);
        }
    }
}

int main(int argc, char *argv[]){
    int i=0;
    int nrargs=3;
    int f = open("fifo", O_RDONLY);

    printf("A ler número de args\n");
    read(f,&nrargs,sizeof(int));

    char *transformacoes[nrargs];
    char *str1; char *str2;
    char command[300];
    int n = read(f,command,sizeof(command));
    command[n]='\0';
    printf("%s\n", command);

    str1=strdup(command);
    while((str2=strsep(&str1," "))!=NULL){
        transformacoes[i++]=str2;
        printf("%s\n", transformacoes[i-1]);
    }
    executeProcFileCommand(argv,transformacoes,nrargs);
    return 0;
}   
