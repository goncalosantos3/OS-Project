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

/*
*   Função que executa um pedido de proc-file.
*   Recebe os argumentos passados ao servidor, o array de strings com:
*   - Primero o tipo de pedido que se trata, que neste caso é sempre proc-file
*   - Os ficheiros input e output
*   - E finalmente todas as transformações a serem executadas para completar o pedido
*   Recebe ainda o parametro nrargs que se trata do comprimento do array de strings transformacoes.
*/
int executeProcFileCommand(char *argv[], char *transformacoes[], int nrargs){
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

    }else if(nrpipes==0){
        if((pid=fork())==0){
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
    }
    return pid;
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

/*
*   Função que recebe o array de inteiros com o número atual de instâncias que ainda são 
*   possíveis de usar de cada transformação e o array de inteiros com o número de instâncias 
*   necessárias de cada transformação para a execução do pedido atual.
*   Esta função tem como objetivo verificar se o pedido por ser executado de seguida ou se terá 
*   de ficar em espera. 
*   Para além disso se o pedido for possível executar de seguida esta função ainda diminui o 
*   número de instâncias disponíveis para cada transformação no array transConfig
*   Devolve 1 se o pedido pode ser executado no momento 0 caso contrário.
*/
int verificaPedido (int transConfig[], int transNecess[]){ 
    int r=1;    
    for(int i=0;i<7 && r==1;i++){
        if(transNecess[i]>transConfig[i]){
            r=0;
        }
    }   
    if(r==1){
        for(int i=0;i<7;i++){
            transConfig[i]-=transNecess[i];
        }
    }
    return r;
}

/*
*   Função que vai construir o array de string transformacoes com toda a informação necessária
*   para executar o pedido atual:
*   - tipo de operação (proc-file ou status)
*   - ficheiros input e output
*   - e finalmente todas as transformações a executar 
*/
void buildPedido(char *command, Pedido pe, int tampedido){
    char *str1, *str2;
    int i=0;

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
    pe->tampedido=tampedido;
}

int main(int argc, char *argv[]){
    int n,pid,tampedido;

    int f1 = open("client-server", O_RDONLY);
    if(f1 == -1) {
        printf("%s\n", strerror(errno));
        return 1;
    }

    int f2 = open("server-client", O_WRONLY);
    if(f2 == -1){ 
        printf("%s\n", strerror(errno));
        return 2;
    }

    //int pipefd[2];
    //pipe(pipefd);
    //dup2(f1,pipefd[0]);
    //close(f1);
    //fcntl(pipefd[0], F_SETFL, O_NONBLOCK);//Faz com que o pipe que lê do fifo não bloqueie

    int transConfig[7];
    //Este array de inteiros vai conter o número máximo de cada transformação de acordo com o primeiro argumento do servidor
    setTransConfig(argv[1],transConfig);

    FilaEspera fesp = initFilaEspera();
    char command[300];

    while(1){//Ciclo que executa os pedidos enviados pelo cliente
        n = read(f1,command,sizeof(command));

        if(n > 0){//Recebemos um novo pedido
            printf("%s\n", command);
            read(f1,&tampedido,sizeof(int));

            Pedido pe = malloc(sizeof(struct pedido) + 7 * sizeof(int) + tampedido * sizeof(*pe->pedido)); 
            buildPedido(command,pe,tampedido);
            //Na struct pe vamos ter o tamanho do pedido, o pedido e o número de instâncias necessárias para cada transformação

            if(verificaPedido(transConfig,pe->transNecess)==0){//Pedido tem que ficar em espera
                //write(f2,"Pedido em fila de espera\n", 26 * sizeof(char));
                colocaFilaEspera(pe,fesp);
            }else if(strcmp(pe->pedido[0],"proc-file")==0){  
                //write(f2,"Pedido a ser processado\n", 25 * sizeof(char));
                pid = executeProcFileCommand(argv,pe->pedido,pe->tampedido);

                if(waitpid(pid,NULL,WNOHANG)!=0){
                   //write(f2,"Pedido concluido\n", 17 * sizeof(char));
                }

            }else if(strcmp(pe->pedido[0],"status")==0){//Ainda por definir

            }
        }else if(n < 0){//O pipe está vazio (Não se recebeu nenhum comando)
            
        }
    }
    return 0;
}