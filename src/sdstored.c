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
    char *path; int pid;
    int nrpipes=nrargs-4;

    if(nrpipes>=1){//Só precisamos de pipes se houverem 2 ou mais transformações

        int pipes[nrpipes][2];
        for(int i=0;i<nrpipes;i++){
            if(pipe(pipes[i])<0){
                printf("%s\n", strerror(errno));
            }
        }

        for(int i=0;i<=nrpipes;i++){

            if(i==0 && fork()==0){//Primeiro comando
                path = strcat(argv[2],"/");
                path = strcat(path,transformacoes[i+3]);
                //printf("%s\n", path);
                int f = open(transformacoes[1],O_RDONLY);
                if(f==-1){
                    printf("%s\n", strerror(errno));
                }
                dup2(f,0);
                dup2(pipes[0][1],1);
                close(f);

                for(int i=0;i<nrpipes;i++){//Fecha todos os pipes
                    close(pipes[i][0]);
                    close(pipes[i][1]);
                }

                if(execl(path,transformacoes[i+3],NULL)==-1){
                    printf("%s\n", strerror(errno));
                    exit(1);
                }
            }
            if(i>0 && i<nrpipes && fork()==0){//Comandos intermédios
                path = strcat(argv[2],"/");
                path = strcat(path,transformacoes[i+3]);
                //printf("%s\n", path);
                dup2(pipes[i-1][0],0);
                dup2(pipes[i][1],1);

                for(int i=0;i<nrpipes;i++){//Fecha todos os pipes
                    close(pipes[i][0]);
                    close(pipes[i][1]);
                }

                if(execl(path,transformacoes[i+3],NULL)==-1){
                    printf("%s\n", strerror(errno));
                    exit(1);
                }
            }else if(i==nrpipes && ((pid=fork())==0)){//Último comando
                path = strcat(argv[2],"/");
                path = strcat(path,transformacoes[i+3]);
                //printf("%s\n", path);
                int f = open(transformacoes[2],O_CREAT | O_WRONLY | O_TRUNC, 0660);
                if(f==-1){
                    printf("%s\n", strerror(errno));
                }
                dup2(pipes[i-1][0],0);
                dup2(f,1);
                close(f);

                for(int i=0;i<nrpipes;i++){//Fecha todos os pipes
                    close(pipes[i][0]);
                    close(pipes[i][1]);
                }
                
                if(execl(path,transformacoes[i+3],NULL)==-1){
                    printf("%s\n", strerror(errno));
                    exit(1);
                }
            }
        }  

        for(int i=0;i<nrpipes;i++){//Fecha todos os pipes
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        waitpid(pid,NULL,0);
    }else if(nrpipes==0){
        if(fork()==0){
            path = strcat(argv[2],"/");
            path = strcat(path,transformacoes[3]);
            //printf("%s\n", path);
            int f1 = open(transformacoes[1],O_RDONLY);
            printf("%s\n", transformacoes[1]);
            if(f1==-1){
                printf("%s\n", strerror(errno));
            }
            int f2 = open(transformacoes[2],O_CREAT | O_WRONLY | O_TRUNC, 0660);
            if(f2==-1){
                printf("%s\n", strerror(errno));
            }
            dup2(f1,0);
            dup2(f2,1);
            close(f1); close(f2);
            if(execl(path,transformacoes[3],NULL)==-1){
                printf("%s\n", strerror(errno));
                exit(1);
            }
        }
        wait(NULL);
    }
}

int main(int argc, char *argv[]){
    int i=0;
    int nrargs,tam;
    int f = open("fifo", O_RDONLY);

    printf("A ler número de args\n");
    read(f,&nrargs,sizeof(int));
    read(f,&tam,sizeof(int));
    printf("%d\n", nrargs);
    printf("%d\n", tam);
    char *transformacoes[nrargs];
    char *str1; char *str2;
    char command[tam+1];

    int n=read(f,command,sizeof(command));
    printf("%d\n", n);
    //command[n]='\n';
    printf("%s\n", command);
    str1=strdup(command);
    while((str2=strsep(&str1," "))!=NULL){
        transformacoes[i]=str2;
        printf("%s\n", transformacoes[i]);
        i++;
    }
    transformacoes[i++]=str2;
    
    if(strcmp(transformacoes[0],"proc-file")==0){   
        executeProcFileCommand(argv,transformacoes,nrargs);
    }else if(strcmp(transformacoes[0],"status")==0){
        
    }
    return 0;
}