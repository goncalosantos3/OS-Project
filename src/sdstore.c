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
*   O cliente serve tem como função implementar uma interface de comunicação
*   com o utilizador que quer aplicar as transformações ao seu ficheiro.
*   O cliente ainda tem como função passar para o servidor as respetivas transformações
*   que ele terá de aplicar ao ficheiro dado pelo utilizador.
*
*   Nome do executável do cliente: sdstore
*/


//Ainda falta a implementação do <priority> no comando inicial

/*
*   f1 é o file descriptor do fifo que comunica entre o cliente e o servidor em que o cliente
*   envia informação e o servidor recebe a informação.
*
*   f2 é o file descriptor do fifo que comunica entre o servidor e o cliente em que o cliente
*   recebe informação enviada pelo servidor.
*/
int main(int argc, char *argv[]){
    int nrargs=argc-1, tam=0, n;

    int p=mkfifo("client-server",0777);//Cria o fifo que comunica entre o cliente e o servidor
    if(p==-1){
        if(errno != EEXIST){//Quando o erro não é o erro de o fifo já existir
            printf("Erro ao construir fifo\n");
            return 1;
        }
    }

    p=mkfifo("server-client",0777);//Cria o fifo que comunica entre o cliente e o servidor
    if(p==-1){
        if(errno != EEXIST){//Quando o erro não é o erro de o fifo já existir
            printf("Erro ao construir fifo\n");
            return 2;
        }
    }

    int f1 = open("client-server", O_WRONLY);//O cliente, por agora, apenas vai escrever no fifo
    if(f1==-1){
        printf("%s\n", strerror(errno));
        return 3;
    }

    int f2 = open("server-client", O_RDONLY);
    if(f2 == -1){
        printf("%s\n", strerror(errno));
        return 4;
    }

    if(strcmp(argv[1],"proc-file")==0){//./sdstore proc-file input_file output_file bcompress ...
        //Executa o primeiro pedido (recebido como argumento do programa)
        printf("A escrever número de args\n");

        write(f1,&nrargs,sizeof(int));//Manda para o servidor o número de argumentos no comando input
        for(int j=0;j<nrargs;j++){
            tam+=strlen(argv[j+1]);
        }
        tam+=nrargs-1;
        char command[tam+1];
        write(f1,&tam,sizeof(int));
        for(int j=0;j<nrargs;j++){
            if(j==0){
                strcpy(command,argv[1]);
            }else{
                strcat(command,argv[j+1]);
            }
            if(j!=nrargs-1){
                strcat(command," ");
            }
        }
        printf("%s\n", command);
        write(f1,command,sizeof(command));

        char info[100];
        n = read(f2,info,sizeof(info));
        //Recebe a informação do servidor sobre o estado o pedido (Em espera ou a ser processado) 
        info[n]='\n';
        write(1,info, n * sizeof(char));
    }else if(strcmp(argv[1],"status")==0){//./sdstore status
        write(f1,"status", 7 * sizeof(char));
    }
    close(f1);
    close(f2);
    return 0;
}