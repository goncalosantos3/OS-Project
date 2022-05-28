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
#include "../libs/emEspera.h"
#include "../libs/emExecucao.h"
#include "../libs/pedido.h"
#include "../libs/funcsAux.h"

volatile sig_atomic_t acabouExecucao = 0;

//Esta variável global sinal vai indicar qual o sinal que foi recebido
// sinal == 0 -> recebeu o sinal SIGUSR1 (temos um novo pedido para processar)
// sinal == 1 -> recebeu o sinal SIGCHLD (um pedido foi terminado)
volatile sig_atomic_t sinal = 0; 

//Se este signal handler for ativado é porque recebemos um sinal SIGCHILD 
//o que significa que um dos pedidos terminou
void pedidoConcluido(int signal){
    sinal = 1;
}

void novoPedido(int signal){
    sinal = 0;
}

void paraExecucao(int signal){
    acabouExecucao = 1;
    sinal = -1;
}

void recebeNovosPedidos(int f, int *pipefd, int pid){
    int tampedido, n1, n2, nrpedido;
    char fifo_name[30], command[300];

    if(fork() == 0){ 
        printf("Processo que recebe pedidos-> %d\n", getpid());   
        close(pipefd[0]);
        while(1){
            //Este processo só vai escrever o input que recebe do cliente para o processo principal
            read(f, &n1, sizeof(int));
            read(f, command, n1 * sizeof(char));
            read(f, &tampedido, sizeof(int));
            read(f, &n2, sizeof(int));
            read(f, fifo_name, n2 * sizeof(char));
            //Lê os inputs do cliente

            printf("Vou mandar o sinal SIGUSR1\n");
            kill(pid, SIGUSR1);
            //Acorda o processo principal

            write(pipefd[1], &n1, sizeof(int));
            write(pipefd[1], command, strlen(command) + 1);
            write(pipefd[1], &tampedido, sizeof(int));
            write(pipefd[1], &n2, sizeof(int));
            write(pipefd[1], fifo_name, strlen(fifo_name) + 1);
            //Manda toda a informação necessária ao processo principal
            nrpedido++;
        }
        exit(0);
    }
    //O processo principal só vai ler do processo que vai receber inputs dos clientes
    close(pipefd[1]);
}

int main(int argc, char *argv[]){
    int p, tampedido, fifo_in, fifo_out, nrpedido=0, pid,n;
    char fifo_name[30];
    char command[300];

    //As próximas 4 linhas de código servem para implementar o termino gracioso do programa.
    //Especificam o que o programa deve fazer ao receber o sinal SIGTERM. 
    signal(SIGTERM, &paraExecucao);
    signal(SIGUSR1, &novoPedido);
    signal(SIGCHLD, &pedidoConcluido);
    
    p  = mkfifo("clients-to-server",0777);
    if(p == -1){
        if(errno != EEXIST){//Quando o erro não é o erro de o fifo já existir
            printf("Erro ao construir fifo\n");
            return 1;
        }
    }

    fifo_in = open("clients-to-server", O_RDONLY);
    if(fifo_in == -1) {
        printf("%s\n", strerror(errno));
        return 2;
    }

    fifo_out = open("clients-to-server", O_WRONLY);
    if(fifo_out == -1) {
        printf("%s\n", strerror(errno));
        return 3;
    }

    int *transConfig, *maxTrans;
    transConfig = malloc(7 * sizeof(int));
    maxTrans = malloc(7 * sizeof(int));
    //Este array de inteiros vai conter o número máximo de cada transformação 
    //de acordo com o primeiro argumento do servidor
    setTransConfig(argv[1],transConfig);
    copiaArray(maxTrans,transConfig,7);

    PedidosEmEspera esp = initEmEspera();
    PedidosEmExecucao pexec = initEmExecucao();

    printf("Processo principal do server-> %d", getpid());

    //Este pipe serve para comunicar entre o processo que recebe novos pedidos
    // e o processo principal do programa
    int pipe1[2];
    pipe(pipe1);
    recebeNovosPedidos(fifo_in,pipe1,getpid());

    while((acabouExecucao == 0) || 
    (acabouExecucao == 1 && (isEmptyEmEspera(esp) == 0 || isEmptyEmExecucao(pexec) == 0))){
        printf("Pause\n");
        pause();        
        printf("Recebi um sinal!!!\n");

        if(sinal == 0 && acabouExecucao != 1){
            //Recebemos um novo pedido;
            printf("Novo pedido\n");

            read(pipe1[0], &n, sizeof(int));
            read(pipe1[0], command, n * sizeof(char));
            read(pipe1[0], &tampedido, sizeof(int));
            read(pipe1[0], &n, sizeof(int));
            read(pipe1[0], fifo_name, n * sizeof(char));

            Pedido pe = malloc(sizeof(struct pedido) + tampedido * sizeof(*pe->pedido)); 
            buildPedido(command, pe, tampedido, nrpedido, fifo_name);

            if(strcmp(pe->pedido[0], "proc-file") == 0){
                //Comando em fila de espera
                if(verificaPedido(transConfig,pe->transNecess) == 0){
                    printf("Para a fila de espera\n");
                    colocaEmEspera(pe,&esp);
                }
                //Comando vai ser executado
                else{
                    printf("Vai executar\n");
                    colocaEmExecucao(pe,&pexec,transConfig,argv);
                    //Dentro desta função o pedido é colocado na lista ligada dos pedidos 
                    //em execução e o pedido é executado
                }
            }else if(strcmp(pe->pedido[0], "status") == 0){
                printf("Status\n");
                statusServer(pe,pexec,maxTrans,transConfig);
            }
        }else if(sinal == 0 && acabouExecucao == 1){
            read(pipe1[0], &n, sizeof(int));
            read(pipe1[0], command, n * sizeof(char));
            read(pipe1[0], &tampedido, sizeof(int));
            read(pipe1[0], &n, sizeof(int));
            read(pipe1[0], fifo_name, n * sizeof(char));

            Pedido pe = malloc(sizeof(struct pedido) + tampedido * sizeof(*pe->pedido)); 
            buildPedido(command, pe, tampedido, nrpedido, fifo_name);
            write(pe->fifo_ouput, "Pedido rejeitado\n", 18*sizeof(char));
            close(pe->fifo_ouput);
            free(pe);
        }else if(sinal == 1){
            printf("Um pedido terminou a sua execução\n");
            pid = wait(NULL);
            retiraPedidoConcluido(pid, &pexec, transConfig);
            retiraPedidosParaExecucao(&esp, &pexec, transConfig, argv);
        }
    }
    printf("Saiu do ciclo\n");
    close(pipe1[0]);
    close(fifo_in);
    close(fifo_out);
    return 0;
}