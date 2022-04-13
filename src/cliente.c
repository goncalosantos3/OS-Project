#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

/*
*   O cliente serve tem como função implementar uma interface de comunicação
*   com o utilizador que quer aplicar as transformações ao seu ficheiro.
*   O cliente ainda tem como função passar para o servidor as respetivas transformações
*   que ele terá de aplicar ao ficheiro dado pelo utilizador.
*
*   Nome do executável do cliente: sdstore
*/


int main(int argc, char *argv[]){
    int i=0,j=0;
    char pedido[50]; char *transformacoes[20]; char *str1,*str2;

    if(mkfifo("fifo",0777)==-1){//Cria o fifo
        if(errno != EEXIST){//Quando o erro não é o erro de o fifo já existir
            printf("Erro ao construir fifo\n");
            return 1;
        }
    }
    int f = open("fifo", O_WRONLY);//O cliente, por agora, apenas vai escrever no fifo 
    read(0,pedido,sizeof(pedido));
    str1=strdup(pedido);
    while((str2=strsep(str1," "))!=NULL){
        //As duas primeiras strings deste array são os ficheiros de input e output 
        if(i>=3){
            transformacoes[j++]=str2;
        }
        i++;
    }
    write(f,&j,sizeof(int));//Manda para o servidor o número de argumentos no comando input
    for(int j=0;j<i;j++){
        write(f,transformacoes[j],sizeof(transformacoes[i]));
    }
}