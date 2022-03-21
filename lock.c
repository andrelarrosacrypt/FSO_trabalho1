/*
Entender como funciona os semaforos utilizados como lock para proteger uma regia critia
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>

int idsem;
struct sembuf operacao[2];

int p_sem()
{
     operacao[0].sem_num = 0;
     operacao[0].sem_op = 0;
     operacao[0].sem_flg = 0;
     operacao[1].sem_num = 0;
     operacao[1].sem_op = 1;
     operacao[1].sem_flg = 0;
     if ( semop(idsem, operacao, 2) < 0)
       printf("erro no p=%d\n", errno);
}
int v_sem()
{
     operacao[0].sem_num = 0;
     operacao[0].sem_op = -1;
     operacao[0].sem_flg = 0;
     if ( semop(idsem, operacao, 1) < 0)
       printf("erro no p=%d\n", errno);
}

int main(){

    int i, pid = 1, estado;

    /* cria semaforo*/
   if ((idsem = semget(0x1223, 1, IPC_CREAT|0x1ff)) < 0)
   {
     printf("erro na criacao da fila\n");
     exit(1);
   }

    // criacao dos processos filhos
    for(i=0; i<10 && pid != 0; i++){
        if( (pid = fork()) < 0 ){
            printf("ERRO: falha na criacao do filho\n");
        }
    }
    
    if(pid == 0){
        // regiao critica
        p_sem();
            printf("processo = %d\n", getpid());
            sleep(5);
        v_sem();
        exit(0);
    }

    printf("sou pai\n");
    while(wait(&estado) > 0);
    printf("filhos morreram, pai vai morrer\n");

    return 0;
}