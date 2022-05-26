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

void pedidoConcluido(int signal){
    //Se este signal handler for ativado é porque recebemos um sinal SIGCHILD 
    //o que significa que um dos pedidos terminou
    int pid = wait(NULL);
    //O resultado deste wait vai ser o pid o processo associado com um certo pedido que acabou
}

void paraExecucao(int signal){
    acabouExecucao = 1;
}

int main(int argc, char *argv[]){
    int p, n, tampedido, f1, nrpedido=0;

    //As próximas 4 linhas de código servem para implementar o termino gracioso do programa.
    //Especificam o que o programa deve fazer ao receber o sinal SIGTERM. 
    signal(SIGTERM, &paraExecucao);

    signal(SIGCHLD, &pedidoConcluido);
    p  = mkfifo("clients-to-server",0777);
    if(p == -1){
        perror("Error");
        //if(errno != EEXIST){//Quando o erro não é o erro de o fifo já existir
        //    printf("Erro ao construir fifo\n");
        //    return 2;
        //}
    }

    f1 = open("clients-to-server", O_RDONLY | O_NONBLOCK);
    if(f1 == -1) {
        printf("%s\n", strerror(errno));
        return 2;
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
    char command[300];

    while((acabouExecucao == 0) || 
    (acabouExecucao == 1 && (isEmptyEmEspera(esp) == 0 || isEmptyEmExecucao(pexec) == 0))){
        //Se a variável acabouExecucao for 0 o ciclo continua
        //Se a variável acabouExecucao for 1 não aceitamos novos pedidos e quando a fila de espera
        //e a fila de pedidos a executar estiverem vazias acaba a execução do programa

        n = read(f1,command,sizeof(command));
        if(n > 0 && acabouExecucao == 0){
            //Recebemos um novo pedido
            //O acabouExecucao ter o valor 0 significa que o servidor ainda não recebeu o sinal SIGTERM
            printf("Recebi pedido\n");
            read(f1,&tampedido,sizeof(int));

            Pedido pe = malloc(sizeof(struct pedido) + 7 * sizeof(int) + tampedido * sizeof(*pe->pedido)); 
            buildPedido(command,pe,tampedido, nrpedido, f1);
            nrpedido++;
            //Na struct pe vamos ter o tamanho do pedido, o pedido e o 
            //número de instâncias necessárias para cada transformação

            if(strcmp(pe->pedido[0],"proc-file")==0){
                //Comando em fila de espera
                if(verificaPedido(transConfig,pe->transNecess)==0){
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

            }
            //Status command
            else if(strcmp(pe->pedido[0],"status")==0){
                printf("Status\n");
                statusServer(pe,pexec,maxTrans,transConfig);
            }
        }  
        else if(n < 0){
            //O pipe está vazio (Não se recebeu nenhum comando)
            //Se não recebermos num novo comando vamos verificar 
            //primeiro se algum pedido já acabou ou não
            verificaPedidosConcluidos(&pexec,transConfig);
            retiraPedidosParaExecucao(&esp,&pexec,transConfig,argv);
            //Depois verificamos se podemos mandar executar pedido que estivessem na fila de espera   
        
        }else if( n > 0 && acabouExecucao == 1){
            //O servidor já recebeu o sinal SIGTERM e então novos comandos são rejeitados
            read(f1,&tampedido,sizeof(int));
            Pedido pe = malloc(sizeof(struct pedido) + 7 * sizeof(int) + tampedido * sizeof(*pe->pedido)); 
            buildPedido(command,pe,tampedido, nrpedido, f1);
            nrpedido++;
            write(pe->fifo_ouput, "Pedido rejeitado\n", 18 * sizeof(char));
            close(pe->fifo_ouput);
            free(pe);
        }
    }
    return 0;
}