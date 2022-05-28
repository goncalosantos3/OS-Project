#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "../libs/funcsAux.h"

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
int executeProcFileCommand(char *argv[], Pedido pe, int ppid){
    char *path; 
    int nrpipes = pe->tampedido-4;
    int pid;

    //Este pid é o pid do processo associado ao pedido
    pid = fork();
    if(pid == 0){
        //Só precisamos de pipes se houverem 2 ou mais transformações
        if(nrpipes >= 1){

            int pipes[nrpipes][2];
            for(int i = 0; i < nrpipes; i++){
                if(pipe(pipes[i]) < 0){
                    printf("%s\n", strerror(errno));
                }
            }

            for(int i = 0; i <= nrpipes; i++){
                //Primeiro comando
                if(i == 0 && fork() == 0){
                    path = strcat(argv[2],"/");
                    path = strcat(path,pe->pedido[i+3]);
                    //printf("%s\n", path);
                    int f = open(pe->pedido[1],O_RDONLY);
                    if(f == -1){
                        printf("%s\n", strerror(errno));
                    }
                    dup2(f, 0);
                    dup2(pipes[0][1], 1);
                    close(f);

                    //Fecha todos os pipes
                    for(int i = 0; i < nrpipes; i++){
                        close(pipes[i][0]);
                        close(pipes[i][1]);
                    }

                    if(execl(path, pe->pedido[i+3], NULL) == -1){
                        printf("%s\n", strerror(errno));
                        exit(1);
                    }
                }
                //Comandos intermédios
                if(i > 0 && i < nrpipes && fork() == 0){
                    path = strcat(argv[2], "/");
                    path = strcat(path, pe->pedido[i+3]);
                    //printf("%s\n", path);
                    dup2(pipes[i-1][0],0);
                    dup2(pipes[i][1],1);

                    //Fecha todos os pipes
                    for(int i = 0; i < nrpipes; i++){
                        close(pipes[i][0]);
                        close(pipes[i][1]);
                    }

                    if(execl(path,pe->pedido[i+3], NULL) == -1){
                        printf("%s\n", strerror(errno));
                        exit(1);
                    }
                }
                //Último comando
                else if(i == nrpipes && ((pid = fork()) == 0)){
                    path = strcat(argv[2],"/");
                    path = strcat(path,pe->pedido[i+3]);

                    int f = open(pe->pedido[2],O_CREAT | O_WRONLY | O_TRUNC, 0660);
                    if(f == -1){
                        printf("%s\n", strerror(errno));
                    }
                    dup2(pipes[i-1][0],0);
                    dup2(f, 1);
                    close(f);

                    //Fecha todos os pipes
                    for(int i = 0; i < nrpipes; i++){
                        close(pipes[i][0]);
                        close(pipes[i][1]);
                    }

                    if(execl(path, pe->pedido[i+3], NULL) == -1){
                        printf("%s\n", strerror(errno));
                        exit(1);
                    }
                }
            }  
            //Fecha todos os pipes
            for(int i = 0; i < nrpipes; i++){
                close(pipes[i][0]);
                close(pipes[i][1]);
            }

        }
        else if(nrpipes == 0){
            if((pid = fork()) == 0){
                path = strcat(argv[2],"/");
                path = strcat(path,pe->pedido[3]);

                int f1 = open(pe->pedido[1],O_RDONLY);
                if(f1 == -1){
                    printf("%s\n", strerror(errno));
                }
                int f2 = open(pe->pedido[2],O_CREAT | O_WRONLY | O_TRUNC, 0660);
                if(f2 == -1){
                    printf("%s\n", strerror(errno));
                }

                dup2(f1,0);
                dup2(f2,1);
                close(f1); close(f2);
                if(execl(path,pe->pedido[3],NULL) == -1){
                    printf("%s\n", strerror(errno));
                    exit(1);
                }
            }
        }
        //O processo associado ao pedido espera pelo processo associado à última transformação
        //Quando este terminar o pedido foi concluído
        waitpid(pid, NULL, 0);
        write(pe->fifo_ouput,"Pedido concluído\n", 18 * sizeof(char));

        int f1 = open(pe->pedido[1], O_RDONLY);
        if(f1 == -1){
            printf("Erro ao abrir ficheiro\n");
        }
        int f2 = open(pe->pedido[2], O_RDONLY);
        if(f2 == -1){
            perror("Erro ao abrir ficheiro output\n");
        }
        int bytes_in = lseek(f1,0,SEEK_END);
        int bytes_out = lseek(f2,0,SEEK_END);
        char str[50];
        sprintf(str, "(Bytes Input: %d, Bytes Output: %d)\n", bytes_in, bytes_out);
        write(pe->fifo_ouput, str, strlen(str) * sizeof(char));
        close(pe->fifo_ouput);
        _exit(0);
    }
    close(pe->fifo_ouput);
    //Devolve o pid do processo associado ao pedido
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
void setTransConfig(char *configFile, int *transConfig){
    char line[100]; char c;
    int n, i = 0, j;

    int f = open(configFile,O_RDONLY);
    if(f == -1){
        printf("%s\n", strerror(errno));
    }

    //O ficheiro config tem sempre 7 linhas (número de transformações)
    while((n = read(f, &c, sizeof(char))) > 0){
        if(c == ' '){
            line[i++] = '\0';
            //Associa a transformação com a sua posição no array transConfig
            if(strcmp(line,"bcompress")   == 0){
                j = 0;
            }
            if(strcmp(line,"bdecompress") == 0){
                j = 1;
            }
            if(strcmp(line,"decrypt")     == 0){
                j = 2;
            }
            if(strcmp(line,"encrypt")     == 0){
                j = 3;
            }
            if(strcmp(line,"gcompress")   == 0){
                j = 4;
            }
            if(strcmp(line,"gdecompress") == 0){
                j = 5;
            }
            if(strcmp(line,"nop")         == 0){
                j = 6;
            }
            i = 0;

        }
        if(c == '\n'){
            line[i++] = '\0';
            transConfig[j] = atoi(line);
            i = 0;
        }
        else{
            line[i++] = c;
        }
    }
    //Como no fim do ficheiro não temos um \n presente então temos que repetir o código no final do ciclo
    line[i++] = '\0';
    transConfig[j] = atoi(line);
    i = 0;
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
int verificaPedido (int *transConfig, int transNecess[]){ 

    for(int i = 0; i < 7; i++){
        if(transNecess[i] > transConfig[i]){
            return 0;
        }
    }   
    return 1;
}

//O status está com uns problemas estranhos
void statusServer(Pedido pe, PedidosEmExecucao pexec, int *maxTrans, int *transConfig){
    PedidosEmExecucao aux = pexec;
    int tam; char string[300];

    while(aux!=NULL){//Manda ao cliente os pedidos que estão em execução
        sprintf(string, "task #%d: ", aux->atual->nrPedido);
        tam=aux->atual->tampedido;
        for(int i=0; i<tam; i++){
            strcat(string,aux->atual->pedido[i]);
            if(i<tam-1){
                strcat(string," ");
            }else{
                strcat(string, "\n");
            }
        }
        string[strlen(string)] = '\0';
        tam = strlen(string)+1;
        write(pe->fifo_ouput, &tam, sizeof(int));
        write(pe->fifo_ouput, string, strlen(string) + 1);
        aux=aux->prox;
    } 

    for(int i=0;i<7;i++){//Manda ao cliente o estado de todas as transformações
        char *trans[7]={"bcompress","bdecompress","decrypt","encrypt","gcompress","gdecompress","nop"};

        sprintf(string, "transf %s: %d/%d (running/max)\n", trans[i], maxTrans[i]-transConfig[i], maxTrans[i]);
        string[strlen(string)]='\0';
        tam = strlen(string)+1;
        write(pe->fifo_ouput, &tam, sizeof(int));
        write(pe->fifo_ouput, string, strlen(string)+1);
    }
    
    close(pe->fifo_ouput);
    free(pe);
}

void copiaArray(int *maxTrans, int *transConfig, int n){

    for(int i=0;i<n;i++){
        maxTrans[i] = transConfig[i];
    }
}