// processo pai cria 10 filho

// bibliotecas
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include<errno.h>
#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include <sys/wait.h>

// operacao do semaforo
struct sembuf operacao[2];
int idsem;

int p_sem(){
    operacao[0].sem_num = 0;
    operacao[0].sem_op = 0;
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


int main(){
    int i, pid =1, estado, idmem;
    int *cont;

    // cria 2 semaforos
    // semaforo 1: pai espera os filhos
    // semaforo 2: filhos esperam o pai
    // semaforo que funciona como lock
    if((idsem = semget(0x1223, 1, IPC_CREAT|0x1ff)) < 0){   // quantos semaforos criar?
        printf("erro na criacao do semaforo\n");
        exit(1);
    }

    // cria memoria compartilhada
    if ((idmem = shmget(0x555, sizeof(int), IPC_CREAT|0x1ff)) < 0){
        printf("erro na criacao da memoria compartilhada\n");
        exit(1);
    }



    // criacao dos processos filhos
    for(i=0; i<10 && pid != 0; i++){
        if( (pid = fork()) < 0 ){
            printf("ERRO: falha na criacao do filho\n");
        }
    }

    // attach
    cont = (int *) shmat(idmem, (char *)0, 0);
    if(cont == (int *)-1){
        printf("erro no attach\n");
        exit(1);
    }
    *cont = 0;

    // filho
    if(pid == 0){
        p_sem();
            printf("sou o processo filho, pid = %d\n", getpid());
            *cont = *cont+1;
            printf("cont = %d\n", *cont);
            if(*cont == 10){
                //v_sem(1);
                printf("cont = 10\n");
            }
        v_sem();

        //p_sem(2);
        // avisa o pai que imprimiu
        // espera a liberacao do pai
        // termina
        exit(0);
    }
    // pai
    //p_sem(1);
    printf("sou pai\n");
    //v_sem(2);
    // espera os avisos dos filhos
    // libera os filhos para terminarem
    // espera os filhos terminarem
    // termina



    //p_sem(0);   // espera os filhos executarem
    //printf("filhos executaram\n");
    //v_sem(1);   // permite o termino dos filhos
    //printf("esperando os filhos terminarem\n");
    //wait(&estado);  // so espera o primeiro filho terminar
    while(wait(&estado) > 0); // espera por todos os processos filhos terminarem
    // https://stackoverflow.com/questions/19461744/how-to-make-parent-wait-for-all-child-processes-to-finish
    printf("depois do wait()\n");


    // esperar receber todos os avisos dos filho
    // recebeu aviso de todos os filhos, envia aviso para todos os filhos
    // espera todos os filhos encerrarem
    // encerra

  return 0;
}