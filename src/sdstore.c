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
    int nrargs=argc-2;

    if(mkfifo("fifo",0777)==-1){//Cria o fifo
        if(errno != EEXIST){//Quando o erro não é o erro de o fifo já existir
            printf("Erro ao construir fifo\n");
            return 1;
        }
    }
    int f = open("fifo", O_WRONLY);//O cliente, por agora, apenas vai escrever no fifo
    if(f==-1){
        printf("Merda %s\n", strerror(errno));
        return 2;
    }
    printf("A escrever número de args\n");
    write(f,&nrargs,sizeof(int));//Manda para o servidor o número de argumentos no comando input
    for(int j=0;j<nrargs;j++){
        printf("%s\n", argv[j+2]);
        write(f,argv[j+2],sizeof(char) * strlen(argv[j+2]));
        if(j!=nrargs-1){
            write(f," ",sizeof(char));
        }
    }
   return 0;
}