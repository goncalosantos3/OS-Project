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

void enviaInfoServer(int tampedido, char *argv[], char *fifo_name ,int f1){        
    char command[300];

    for(int j=0;j<tampedido;j++){

        if(j==0){
            strcpy(command,argv[1]);
        }else{
            strcat(command,argv[j+1]);
        }
        if(j!=tampedido-1){
            strcat(command," ");
        }
    }
    printf("%s\n", command);
    write(f1,command,sizeof(command));

    write(f1,&tampedido,sizeof(int));
    //Manda para o servidor o número de argumentos no comando input
    printf("%s\n", fifo_name);
    write(f1,fifo_name,30 * sizeof(char));
}

void enviaInfoServerStatus(char *fifo_name, int f1){
    int tampedido=1;
    write(f1,"status", 7 * sizeof(char));
    write(f1,&tampedido,sizeof(int));
    write(f1,fifo_name,30 * sizeof(char));
}

int recebeInfoServer(char info[], char *fifo_name){//Por desenvolver

    int f2 = open(fifo_name, O_RDONLY);
    if(f2 == -1){
        printf("%s\n", strerror(errno));
        return 4;
    }

    int n = read(f2,info,100 * sizeof(char));
    //Recebe a informação do servidor sobre o estado o pedido (Em espera ou a ser processado) 
    info[n]='\n';
    if(strcmp(info,"Pedido a ser processado\n")==0){
        write(1,info, n * sizeof(char));//Escreve: "Pedido a ser processado"
        n = read(f2,info,100 * sizeof(char));
        info[n]='\n';
        write(1,info,n*sizeof(char));//Escreve: "Pedido concluido"
    }else if (strcmp(info,"Pedido em fila de espera\n")==0){
        write(1,info, n * sizeof(char));//Escreve: "Pedido em fila de espera"
        n = read(f2,info,100 * sizeof(char));
        info[n]='\n';
        write(1,info,n * sizeof(char));//Escreve: "Pedido a ser processado"
        n = read(f2,info,100 * sizeof(char));
        info[n]='\n';
        write(1,info,n*sizeof(char));//Escreve: "Pedido concluido"
    }
    return f2;
}

int recebeInfoServerStatus(char *info, char *fifo_name){
    int f2 = open(fifo_name, O_RDONLY);
    if(f2 == -1){
        printf("%s\n", strerror(errno));
        return 4;
    }

    return f2;
}

int main(int argc, char *argv[]){
    int tampedido=argc-1;   
    char fifo_name[30];
    char string[20];
    int f1,f2,p;

    f1 = open("clients-to-server", O_WRONLY);//O cliente, por agora, apenas vai escrever no fifo
    if(f1==-1){
        printf("%s\n", strerror(errno));
        return 3;
    }
    strcpy(fifo_name,"fifo-");
    sprintf(string,"%d", getpid());
    strcat(fifo_name,string);

    p = mkfifo(fifo_name,0777);//Cria o fifo que comunica entre o cliente e o servidor
    if(p==-1){
        if(errno != EEXIST){//Quando o erro não é o erro de o fifo já existir
            printf("Erro ao construir fifo\n");
            return 2;
        }
    }   

    if(strcmp(argv[1],"proc-file")==0){//./sdstore proc-file input_file output_file bcompress ...
        //Executa o primeiro pedido (recebido como argumento do programa)
        enviaInfoServer(tampedido,argv,fifo_name,f1);

        char info[100];
        f2 = recebeInfoServer(info,fifo_name);

    }else if(strcmp(argv[1],"status")==0){//./sdstore status
        enviaInfoServerStatus(fifo_name,f1);

        char info[100];
        f2 = recebeInfoServerStatus(info,fifo_name);
    }
    close(f1);
    close(f2);
    return 0;
}
