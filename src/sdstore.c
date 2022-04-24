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

int main(int argc, char *argv[]){
    int nrargs=argc-1, tam=0, n;

    int p=mkfifo("fifo",0777);//Cria o fifo
    if(p==-1){
        if(errno != EEXIST){//Quando o erro não é o erro de o fifo já existir
            printf("Erro ao construir fifo\n");
            return 1;
        }
    }

    int f = open("fifo", O_WRONLY);//O cliente, por agora, apenas vai escrever no fifo
    if(f==-1){
        printf("%s\n", strerror(errno));
        return 2;
    }

    if(strcmp(argv[1],"proc-file")==0){
        //Executa o primeiro pedido (recebido como argumento do programa)
        printf("A escrever número de args\n");

        write(f,&nrargs,sizeof(int));//Manda para o servidor o número de argumentos no comando input
        for(int j=0;j<nrargs;j++){
            tam+=strlen(argv[j+1]);
        }
        tam+=nrargs-1;
        char command[tam+1];
        write(f,&tam,sizeof(int));
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
        write(f,command,sizeof(command));
    }else if(strcmp(argv[1],"status")==0){
        
    }
    close(f);
    return 0;
}