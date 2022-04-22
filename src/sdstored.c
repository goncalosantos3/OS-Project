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

/*
*   Nesta função a ideia é atravessar o ficheiro config linha a linha e preencher o array de transConfig
*   com o número máximo de transformações concorrentes permitidas
*   Transformações associadas a cada posição do array transConfig (por ordem alfabética):
*   i=0:  bcompress
*   i=1:  bdecompress
*   i=2:  decrypt
*   i=3:  encrypt
*   i=4:  gcompress
*   i=5:  gdecompress
*   i=6:  nop
*/
void setTransConfig(char *configFile, int transConfig[]){
    char line[100]; char c;
    int n,i=0,j;

    int f = open(configFile,O_RDONLY);
    if(f==-1){
        printf("%s\n", strerror(errno));
    }

    while((n=read(f,&c,sizeof(char)))>0){//O ficheiro config tem sempre 7 linhas (número de transformações)
        if(c == ' '){
            line[i++]='\0';
            //Associa a transformação com a sua posição no array transConfig
            if(strcmp(line,"bcompress")==0){
                j=0;
            }else if(strcmp(line,"bdecompress")==0){
                j=1;
            }else if(strcmp(line,"decrypt")==0){
                j=2;
            }else if(strcmp(line,"encrypt")==0){
                j=3;
            }else if(strcmp(line,"gcompress")==0){
                j=4;
            }else if(strcmp(line,"gdecompress")==0){
                j=5;
            }else if(strcmp(line,"nop")==0){
                j=6;
            }
            i=0;
        }else if(c =='\n'){
            line[i++]='\0';
            transConfig[j]=atoi(line);
            i=0;
        }else{
            line[i++]=c;
        }
    }
    //Como no fim do ficheiro não temos um \n presente então temos que repetir o código no final do ciclo
    line[i++]='\0';
    transConfig[j]=atoi(line);
    i=0;
}

int main(int argc, char *argv[]){
    int i=0; int n;
    int nrargs,tam;
    char *str1; char *str2;
    int f = open("fifo", O_RDONLY);

    int transConfig[7];
    //Este array de inteiros vai conter o número máximo de cada transformação de acordo com o segundo argumento do servidor
    setTransConfig(argv[1],transConfig);

    read(f,&nrargs,sizeof(int));
    read(f,&tam,sizeof(int));
    char *transformacoes[nrargs];
    char command[tam+1];

    while((n=read(f,command,sizeof(command)))>0){
        printf("%s\n", command);
        str1=strdup(command);
        while((str2=strsep(&str1," "))!=NULL){
            transformacoes[i]=str2;
            i++;
        }
        transformacoes[i++]=str2;
        /*
        if(strcmp(transformacoes[0],"proc-file")==0){   
            executeProcFileCommand(argv,transformacoes,nrargs);
        }else if(strcmp(transformacoes[0],"status")==0){//Ainda por definir

        }
        */
    }
    return 0;
}