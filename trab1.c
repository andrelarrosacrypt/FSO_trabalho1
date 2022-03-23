/* 
ainda precisa deletar a fila de mensagens
*/

// bibliotecas
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <string.h>


// estrutura mensagem
int pid_msg;
struct mensagem
{
    long pid_msg;
    char msg[30];
};

// globais
int idsem, idmem, idfila_pai, idfila_filhos;
struct mensagem mensagem_env, mensagem_rec;

// operacao do semaforo
struct sembuf operacao[2];

int p_sem(){
    operacao[0].sem_num = 0;
    operacao[0].sem_op = 0; // operacao[0].sem_op = 0;
    operacao[0].sem_flg = 0;
    operacao[1].sem_num = 0;
    operacao[1].sem_op = 1;    // inicialmente: operacao[1].sem_op = 1;
    operacao[1].sem_flg = 0;
    if ( semop(idsem, operacao, 2) < 0){
        printf("erro no p=%d\n", errno);
    }
}
int v_sem(){
    operacao[0].sem_num = 0;
    operacao[0].sem_op = -1;
    operacao[0].sem_flg = 0;
    if ( semop(idsem, operacao, 1) < 0){
        printf("erro no p=%d\n", errno);
    }   
}

// programa principal
int main(){
    int i, pid =1, estado;
    int *cont;

    // cria semaforo para ser utilizado como lock
    if ((idsem = semget(0x1234, 1, IPC_CREAT | 0x1ff)) < 0) {
        perror("main: semget");
        exit(1);
    }

    // cria memoria compartilhada para ser utilizada como contador
    if ((idmem = shmget(0x555, sizeof(int), IPC_CREAT|0x1ff)) < 0){
        printf("erro na criacao da memoria compartilhada\n");
        exit(1);
    }

    /* TODO: usar apenas uma fila de mensagens ?? */

    // cria fila de mensagem para avisar processo pai
    if((idfila_pai = msgget(0x1223, IPC_CREAT|0x1B6)) < 0){
        printf("erro na criacao da fila\n");
        exit(1);
    }

    // cria fila de mensagem para avisar processos filhos
    if((idfila_filhos = msgget(0x1224, IPC_CREAT|0x1B6)) < 0){
        printf("erro na criacao da fila\n");
        exit(1);
    }

    // attach na memoria compartilhada
    cont = (int *) shmat(idmem, (char *)0, 0);
    if(cont == (int *)-1){
        printf("erro no attach\n");
        exit(1);
    }
    *cont = 0;  // inicializacao do contador

    // criacao dos processos filhos
    for(i=0; i<10 && pid != 0; i++){
        if( (pid = fork()) < 0 ){
            printf("ERRO: falha na criacao do filho\n");
        }
    }

    // filho
    if(pid == 0){
        p_sem();    // lock
            int filho_pid = getpid();
            printf("sou o processo filho, pid = %d\n", filho_pid);
            (*cont)++;
            if(*cont == 10){
                // ultimo filho libera o pai
                mensagem_env.pid_msg = filho_pid;
                strcpy(mensagem_env.msg, "filhos terminaram");
                sleep(rand()%5);    // TODO: tirar antes de enviar
                msgsnd(idfila_pai, &mensagem_env, sizeof(mensagem_env)-sizeof(long), 0);
            }
        v_sem();    // unlock

        printf("%d esperando a liberacao do pai\n", getpid());  // TODO: retirar
        // filhos esperam a liberacao do pai
        msgrcv(idfila_filhos, &mensagem_rec, sizeof(mensagem_rec)-sizeof(long), 0, 0);
        printf("%d foi liberado\n", getpid());  // TODO: retirar

        exit(0);
    }
    // pai
    printf("sou pai\n");    // TODO: retirar
    // pai espera os filhos executarem o print
    printf("pai esperando o print dos filhos\n");   // TODO: retirar
    msgrcv(idfila_pai, &mensagem_rec, sizeof(mensagem_rec)-sizeof(long), 0, 0);
    // pai libera os filhos para terminarem a execucao
    printf("pai recebeu a msg dos filhos\npai vai liberar os filhos para termino\n");   // TODO: retirar
    // pai envia mensagem de liberacao para cada filho
    mensagem_env.pid_msg = getpid();
    strcpy(mensagem_env.msg, "pai liberou");
    for(i=0; i<10; i++){
    msgsnd(idfila_filhos, &mensagem_env, sizeof(mensagem_env)-sizeof(long), 0);
    }
    // pai espera por todos os processos filhos terminarem, entao encerra
    while(wait(&estado) > 0);
    // https://stackoverflow.com/questions/19461744/how-to-make-parent-wait-for-all-child-processes-to-finish
    printf("filhos terminaram\n"); // TODO: retirar
    // DELETAR FILA
    //struct mensagem *buf;
    //int msgctl(idfila, IPC_RMID | buf); // error: expected ‘)’ before numeric constant
    //printf("fila de mensagem deletada\n");  // TODO: retirar

    return 0;
}