// processo pai cria 10 filho

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

int main(){
    int i, pid =1;

    for(i=0; i<10 && pid != 0; i++){
        if( (pid = fork()) < 0 ){
            printf("ERRO: falha na criacao do filho\n");
        }
    }
    if(pid == 0){
        int pid_filho = getpid();
        printf("sou o processo filho, pid = %d\n", pid_filho);
    }
    else{
        printf("processo pai\n");
        sleep(20);
    }

  return 0;
}