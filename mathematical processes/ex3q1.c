#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

struct pol{
    int size;
    int* pol1;
    int* pol2;
    char word[128];
};

int* add(int *polinome1,int *polinome2,int size);
int* sub(int *polinome1,int *polinome2,int size);
int* mult(int *polinome1,int *polinome2,int size);
void sizeSearch(char* input,char* size1,char* size2);
void elementsSearch(char* input,int* elements1,int* elements2,int size);
void isEnd(char* input,int* check);
void wordCheck(char* input,char* word);
int* operations(int* pol1,int* pol2,int size,int* (*callback)(int*,int*,int));
void printResult(int* result,int size);

int main() {
    char input[128];
    int endCheck=0;
    int max;
    char size1[128]="\0";
    char size2[128]="\0";
    int resultSize=0;
    int* result=NULL;
    while(1) {
        printf("enter your operation:");
        fgets(input, 128, stdin);
        isEnd(input, &endCheck);
        if(endCheck==1){
            break;
        }
        sizeSearch(input, size1, size2);
        int sizee1 = atoi(size1) ;
        int sizee2 = atoi(size2) ;
        if(sizee2>=sizee1){
            max=sizee2;
        }
        else{
            max=sizee1;
        }
        struct pol x;
        x.size=max+1;
        x.pol1=(int*) malloc(sizeof (int)*x.size);
        x.pol2=(int*) malloc(sizeof (int)*x.size);
        for (int i = 0; i < x.size; i++) {
            x.pol1[i] = 0;
            x.pol2[i]=0;
        }
        elementsSearch(input, x.pol1, x.pol2, x.size);
        x.word[0]='\0';
        wordCheck(input,x.word);
        if(strcmp(x.word,"ADD")==0){
            resultSize=x.size;
            result=operations(x.pol1,x.pol2,x.size,add);
        }
        else if(strcmp(x.word,"SUB")==0){
            resultSize=x.size;
            result=operations(x.pol1,x.pol2,x.size,sub);
        }
        else if(strcmp(x.word,"MUL")==0){
            resultSize=x.size*2-1;
            result=operations(x.pol1,x.pol2,x.size,mult);
        }
        printResult(result,resultSize);
        free(x.pol1);
        free(x.pol2);
        free(result);
    }
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

void printResult(int* result,int size){
    int firstElement=0;
    if(result!=NULL){
        int zeros=0;
        for(int f=0;f<size;f++){
            if(result[f]==0){
                zeros++;
            }
        }
        for(int i=size-1;i>=0;i--) {
            if(zeros==size){
                printf("0");
                break;
            }
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