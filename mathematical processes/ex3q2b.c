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

void sizeSearch(char* input,char* size1,char* size2);
void elementsSearch(char* input,int* elements1,int* elements2,int size);
void wordCheck(char* input,char* word);
int* add(int *polinome1,int *polinome2,int size);
int* sub(int *polinome1,int *polinome2,int size);
int* mult(int *polinome1,int *polinome2,int size);
void isEnd(char* input,int* check);
int* operations(int* pol1,int* pol2,int size,int* (*callback)(int*,int*,int));
void printResult(int* result,int size);

int main(){
    char word[128]="\0";
    int endCheck=0;
    int max;
    int *pol1;
    int* pol2;
    char size1[128]="\0";
    char size2[128]="\0";
    int resultSize=0;
    int* result=NULL;
    struct operations* ptr;
    key_t key;
    int shm_id;
    sem_t *full;
    sem_t *empty;
    sem_t *mutex;
    mutex= sem_open("/mutex",0);
    empty= sem_open("/empty",0);
    full= sem_open("/full",0);
    if(empty==SEM_FAILED || full==SEM_FAILED || mutex==SEM_FAILED){
        perror("this semaphore doesnt exist");
        return EXIT_FAILURE;
    }
    key = ftok("/tmp",922);
    if((shm_id= shmget(key,0,0600))==-1){
        perror("failed to get sh_memory");
        exit(1);
    }
    if((ptr=(struct operations*)shmat(shm_id,NULL,0))<0){
        perror("failed to attach sh_memory");
        exit(1);
    }
    while(1){
        sem_wait(full);
        sem_wait(mutex);
        isEnd(ptr[0].operation, &endCheck);
        if(endCheck==1){
            break;
        }
        sizeSearch(ptr[0].operation, size1, size2);
        int sizee1 = atoi(size1) ;
        int sizee2 = atoi(size2) ;
        if(sizee2>=sizee1){
            max=sizee2;
        }
        else{
            max=sizee1;
        }
        pol1=(int*) malloc(sizeof (int)*(max+1));
        pol2=(int*) malloc(sizeof (int)*(max+1));
        for (int i = 0; i < max+1; i++) {
            pol1[i] = 0;
            pol2[i]=0;
        }
        elementsSearch(ptr[0].operation, pol1, pol2, max+1);
        wordCheck(ptr[0].operation,word);
        if(strcmp(word,"ADD")==0){
            resultSize=max+1;
            result=operations(pol1,pol2,max+1,add);
        }
        else if(strcmp(word,"SUB")==0){
            resultSize=max+1;
            result=operations(pol1,pol2,max+1,sub);
        }
        else if(strcmp(word,"MUL")==0){
            resultSize=((max+1)*2)-1;
            result=operations(pol1,pol2,max+1,mult);
        }
        printResult(result,resultSize);
        for(int i=0;i<10;i++) {
            if (ptr[i].check == -1) {
                break;
            }
            if (i == 0) {
                ptr[i].check=-1;
                strcpy(ptr[i].operation, "\0");
            } else {
                ptr[i - 1].check = ptr[i].check;
                ptr[i].check = -1;
                strcpy(ptr[i - 1].operation, ptr[i].operation);
                strcpy(ptr[i].operation, "\0");
            }
        }
        free(pol1);
        free(pol2);
        free(result);
        sem_post(empty);
        sem_post(mutex);
    }
    sem_close(empty);
    sem_unlink("/empty");
    sem_close(full);
    sem_unlink("/full");
    sem_close(mutex);
    sem_unlink("/mutex");
    shmdt(ptr);
    shmctl(shm_id,IPC_RMID,NULL);
}

int* add(int *polinome1,int *polinome2,int size){
    int* result=(int*) malloc(sizeof (int)*size);
    for(int i=0;i<size;i++){
        result[i]=polinome1[i]+polinome2[i];
    }
    return result;
}

int* sub(int *polinome1,int *polinome2,int size){
    int* result=(int*) malloc(sizeof (int)*size);
    for(int i=0;i<size;i++){
        result[i]=polinome1[i]-polinome2[i];
    }
    return result;
}

int* mult(int *polinome1,int *polinome2,int size){
    int* result=(int*) malloc(sizeof(int)*(2*size-1));
    for(int k=0;k<2*size-1;k++){
        result[k]=0;
    }
    for(int i=0;i<size;i++){
        for(int j=0;j<size;j++){
            result[i+j]+=polinome1[i]*polinome2[j];
        }
    }
    return result;
}

void printResult(int* result,int size){
    int firstElement=0;
    if(result!=NULL){
        int zeros=0;
        for(int f=0;f<size;f++){
            if(result[f]==0){
                zeros++;
            }
        }
        if(zeros==size){
            printf("0\n");
            return;
        }
        for(int i=size-1;i>=0;i--) {
            if(firstElement==0){
                if(i==0){
                    if (result[i] != 0) {
                        firstElement=1;
                        printf("%d", result[i]);
                    }
                }
                else if(i==1){
                    if (result[i] != 0) {
                        firstElement=1;
                        printf("%dx", result[i]);
                    }
                }
                else {
                    if (result[i] != 0) {
                        firstElement=1;
                        printf("%dx^%d", result[i], i);
                    }
                }
                continue;
            }
            if(i==0){
                if (result[i] > 0) {
                    printf(" + %d", result[i]);
                }
                else if(result[i]<0){
                    printf(" - %d", -1*result[i]);
                }
            }
            else if(i==1){
                if (result[i] > 0) {
                    printf(" + %dx", result[i]);
                }
                else if(result[i]<0){
                    printf(" - %dx", -1*result[i]);
                }
            }
            else{
                if (result[i] > 0) {
                    printf(" + %dx^%d", result[i], i);
                }
                else if(result[i]<0){
                    printf(" - %dx^%d", -1*result[i], i);
                }
            }
        }
        printf("\n");
    }
}

int* operations(int* pol1,int* pol2,int size,int* (*callback)(int*,int*,int)){
    return callback(pol1,pol2,size);
}

void sizeSearch(char* input,char* size1,char* size2){
    int sizeCount=2;
    int count=0;
    for(int i=0;i< strlen(input);i++){
        if(input[i]=='('){
            for(int j=i+1;j< strlen(input);j++){
                if(input[j]>=48 && input[j]<=57){
                    if(sizeCount==2){
                        size1[count]=input[j];
                        count++;
                    }
                    else if(sizeCount==1){
                        size2[count]=input[j];
                        count++;
                    }
                }
                else if(input[j]==' '){
                    continue;
                }
                else if(input[j]==':'){
                    break;
                }
            }
            sizeCount--;
            count=0;
        }
        else {
            continue;
        }
    }
}

void elementsSearch(char* input,int* elements1,int* elements2,int size){
    int counter=size-1;
    int searchCount=2;
    int k;
    char x[1]="\0";
    for(int i=0;i< strlen(input);i++){
        if(input[i]==':'){
            for(int j=i+1;j< strlen(input);j++){
                if(input[j]==')'){
                    i+=j;
                    if(counter!=0 && searchCount==2){
                        for(k=0;k<size-counter;k++){
                            elements1[k]=elements1[counter+k];
                        }
                        for(int m=k;m<size;m++){
                            elements1[m]=0;
                        }
                    }
                    else if(counter!=0 && searchCount==1){
                        for(k=0;k<size-counter;k++){
                            elements2[k]=elements2[counter+k];
                        }
                        for(int m=k;m<size;m++){
                            elements2[m]=0;
                        }
                    }
                    counter=size-1;
                    break;
                }
                else if(input[j]==','){
                    counter--;
                }
                else if(input[j]==' '){
                    continue;
                }
                else{
                    if(searchCount==2) {
                        if (input[j] == '-') {
                            x[0] = input[j+1];
                            elements1[counter] = elements1[counter] * 10 + atoi(x);
                            elements1[counter]=elements1[counter]*-1;
                            j++;
                        }
                        else{
                            if(elements1[counter]>=0) {
                                x[0] = input[j];
                                elements1[counter] = elements1[counter] * 10 + atoi(x);
                            }
                            else{
                                x[0] = input[j];
                                elements1[counter] = elements1[counter] * 10 - atoi(x);
                            }
                        }
                    }
                    else if(searchCount==1){
                        if (input[j] == '-') {
                            x[0] = input[j+1];
                            elements2[counter] = elements2[counter] * 10 + atoi(x);
                            elements2[counter]=elements2[counter]*-1;
                            j++;
                        }
                        else{
                            if(elements2[counter]>=0) {
                                x[0] = input[j];
                                elements2[counter] = elements2[counter] * 10 + atoi(x);
                            }
                            else{
                                x[0] = input[j];
                                elements2[counter] = elements2[counter] * 10 - atoi(x);
                            }
                        }
                    }
                }
            }
            searchCount--;
        }
    }
}

void isEnd(char* input,int* check){
    for(int i=0;i< strlen(input);i++){
        if(input[i]==' '){
            continue;
        }
        else if(input[i]=='E' && i<= strlen(input)-3){
            if(input[i+1]=='N' && input[i+2]=='D'){
                *check=1;
            }
        }
        else{
            break;
        }
    }
}

void wordCheck(char* input,char* word){
    int counter=0;
    for(int i=0;i< strlen(input);i++){
        if(input[i]==')'){
            for(int j=i+1;j< strlen(input);j++){
                if(input[j]==' '){
                    continue;
                }
                else if(input[j]>=65 && input[j]<=90){
                    word[counter]=input[j];
                    counter++;
                }
                else{
                    break;
                }
            }
            break;
        }
    }
}