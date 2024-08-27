#include <stdio.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

struct operations{
    int check;
    char operation[128];
};

void isEnd(char* input,int* check);

int main() {
    char input[128];
    int endCheck=0;
    struct operations* ptr;
    key_t key;
    int shm_id;
    int counter=0;
    sem_t *full;
    sem_t *empty;
    sem_t *mutex;
    empty = sem_open("/empty",O_CREAT,0644,10);
    full = sem_open("/full",O_CREAT,0644,0);
    mutex = sem_open("/mutex",O_CREAT,0644,1);
    if(empty==SEM_FAILED || full==SEM_FAILED || mutex==SEM_FAILED){
        perror("failed to open semaphore...");
        return EXIT_FAILURE;
    }
    key = ftok("/tmp",922);
    if((shm_id= shmget(key,10*sizeof (struct operations),IPC_CREAT|IPC_EXCL|0600))==-1){
        perror("failed to get sh_memory");
        exit(1);
    }
    if((ptr=(struct operations*)shmat(shm_id,NULL,0))<0){
        perror("failed to attach sh_memory");
        exit(1);
    }
    for(int i=0;i<10;i++){
        ptr[i].check=-1;
    }
    while(1){
        printf("enter you operation:");
        fgets(input, 128, stdin);
        sem_wait(empty);
        sem_wait(mutex);
        for(int i=0;i<10;i++){
            if(ptr[i].check==-1){
                counter=i;
                break;
            }
        }
        isEnd(input, &endCheck);
        if(endCheck==1){
            strcpy(ptr[counter].operation,"END");
            sem_post(full);
            sem_post(mutex);
            break;
        }
        struct operations x;
        strcpy(x.operation,input);
        strcpy(ptr[counter].operation,x.operation);
        x.check=1;
        ptr[counter].check=x.check;
        sem_post(mutex);
        sem_post(full);
    }
    sem_close(empty);
    sem_unlink("/empty");
    sem_close(full);
    sem_unlink("/full");
    sem_close(mutex);
    sem_unlink("/mutex");
    shmdt(ptr);
}

void isEnd(char* input,int* check){
    for(int i=0;i< strlen(input);i++){
        if(input[i]==' '){
            continue;
        }
        else if(input[i]=='E' && i< strlen(input)-3){
            if(input[i+1]=='N' && input[i+2]=='D'){
                *check=1;
            }
        }
        else{
            break;
        }
    }
}