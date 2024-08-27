#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

pid_t son=0;
pid_t stoppedSon=0;
int* bgPids;
int length2=0;
int status=0;
int stoppedArg=0;
int numOfCommand=0;
int argumentsNum=0;
int commandNum=0;

void deletingCoatations(char* command){
    char commandNew[512]="\0";
    int counter=0;
    for(int m=0;m<strlen(command);m++){
        if(command[m]=='"'){
            continue;
        }
        else{
            commandNew[counter]=command[m];
            counter++;
        }
    }
    strcpy(command,commandNew);
}

void echoEditing(char* command,int* arguementCount){
    int l;
    char commandNew[512]="\0";
    int position=0;
    int counter=0;
    int colsCounter=2;
    char x[510]="\0";
    for(int a=0;a< strlen(command);a++) {
        for (l = a; l < strlen(command); l++) {
            if (command[l] == 'e' && command[l + 1] == 'c' && command[l + 2] == 'h' && command[l + 3] == 'o') {
                l += 4;
                break;
            }
        }
        for (int i = l; i < strlen(command); i++) {
            if (command[i] == 34) {
                colsCounter--;
                for (int c = i + 1; c < strlen(command); c++) {
                    if (command[c] == 34) {
                        colsCounter--;
                        a+=c;
                        break;
                    } else if (command[c] == ' ') {
                        command[c] = '\xE1';
                    }else if(command[c]==';'){
                        command[c]='\xE2';
                    }
                    else if(command[c]=='>'){
                        command[c]='\xE3';
                    }
                    else if(command[c]=='|'){
                        command[c]='\xE4';
                    }
                    else if(command[c]=='&'){
                        command[c]='\xE5';
                    }
                }
                colsCounter=2;
                break;
            }
        }
    }
}

void printCheck(char** variable,char** value,char* command,int length,int* cont,int* commandCount,int* argumentCount){
    int y;
    int f;
    int d;
    int counter=0;
    int contFound=0;
    char added[510]="\0";
    char x;
    int b;
    char commandNew[510]="\0";
    for(y=0;y< strlen(command);y++){
        if(command[y]=='$'){
            y+=1;
            break;
        }
    }
    for(d=y;d< strlen(command);d++){
        if(command[d]!=' ' && command[d]!='"' && command[d]!='$') {
            x = command[d];
            strncat(added, &x, 1);
        }
        else{
            break;
        }
    }
    added[strlen(added)]='\0';
    for(f=0;f<length;f++) {
        if(strcmp(variable[f],added)==0){
            contFound=1;
            break;
        }
    }
    if(contFound==1){
        for(b=0;b<strlen(command);b++){
            if(command[b]=='$'){
                break;
            }
            commandNew[counter]=command[b];
            counter++;
        }
        strncat(commandNew,value[f], strlen(value[f]));
        counter+= strlen(value[f]);
        for(int u=b+ strlen(added)+1;u< strlen(command);u++){
            commandNew[counter]=command[u];
            counter++;
        }
        strcpy(command,commandNew);
    }
    else{
        for(b=0;b<strlen(command);b++){
            if(command[b]=='$'){
                break;
            }
            commandNew[counter]=command[b];
            counter++;
        }
        for(int u=b+ strlen(added)+1;u< strlen(command);u++){
            commandNew[counter]=command[u];
            counter++;
        }
        strcpy(command,commandNew);
    }
}

void runCommand(char** variable,char** value,char* command,int length,int* exists){
    int s;
    char added[510]="\0";
    char x;
    int u=0;
    for(s=0;s< strlen(command);s++){
        if(command[s]=='$'){
            s+=1;
            break;
        }
    }
    for(int d=s;d< strlen(command);d++) {
        if (command[d]!=' ' && command[d]!='\n') {
            x=command[d];
            strncat(added,&x,1);
        }
        else{
            continue;
        }
    }
    for(int f=0;f<length;f++) {
        if(strcmp(variable[f],added)==0){
            strcpy(command,value[f]);
            u=1;
            break;
        }
    }
    if(u==0){
        *exists=1;
    }
}

void saving(char** variables,char ** values,char* command,int* length,int* cont){
    *cont=1;
    *length=*length+1;
    int s;
    int e;
    int exist=0;
    char added3[510]="\0";
    char x3;
    char added2[510]="\0";
    char x2;
    for(s=0;s< strlen(command);s++){
        if(command[s]=='='){
            s+=1;
            break;
        }
    }
    for(int k=0;k<s-1;k++) {
        if (command[k] != ' ') {
            x3 = command[k];
            strncat(added3, &x3, 1);
        }
    }
    for(int d=s;d< strlen(command);d++) {
        if (command[d] != ' ') {
            x2 = command[d];
            strncat(added2, &x2, 1);
        }
    }
    for(e=0;e<*length-1;e++){
        if(strcmp(variables[e],added3)==0){
            exist=1;
            break;
        }
    }
    if(exist==1){
        strcpy(values[e],added2);
    }
    else {
        if (*length == 1) {
            variables[0] = (char *) malloc(sizeof(char) * 510);
            strcpy(variables[0], added3);
            values[0] = (char *) malloc(sizeof(char) * 510);
            strcpy(values[0], added2);
        } else {
            variables = realloc(variables, sizeof(char *) * (*length));
            variables[*length - 1] = (char *) malloc(sizeof(char) * 510);
            strcpy(variables[*length - 1], added3);
            values = realloc(values, sizeof(char *) * (*length));
            values[*length - 1] = (char *) malloc(sizeof(char) * 510);
            strcpy(values[*length - 1], added2);
        }
    }
}

int hasBigSymbol(char* command,int* exist,int* file){
    char name[510]="\0";
    char commandNew[510]="\0";
    int counter=0;
    int i;
    for(i=0;i< strlen(command);i++){
        if(command[i]=='>'){
            *exist=1;
            for(int j=i+1;j< strlen(command);j++){
                if(command[j]==' '){
                    continue;
                }
                else{
                    for(int k=j;k< strlen(command);k++){
                        if(command[k]==' '){
                            break;
                        }
                        name[counter]=command[k];
                        counter++;
                    }
                    break;
                }
            }
            break;
        }
    }
    for(int m=0;m<i;m++){
        commandNew[m]=command[m];
    }
    strcpy(command,commandNew);
    *file=open(name,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
}

void unpersentEnd(char* command,int* exist){
    char commandNew[510]="\0";
    for(int i=0;i< strlen(command);i++){
        if(command[i]=='&'){
            for(int j=i+1;j< strlen(command);j++){
                if(command[j]!=' '){
                    *exist=0;
                    break;
                }
                else{
                    *exist=1;
                    if(j== strlen(command)-1 && command[j]==' '){
                        for(int k=0;k<i;k++){
                            commandNew[k]=command[k];
                        }
                        strcpy(command,commandNew);
                    }
                }
            }
        }
    }
}

void TSTP_handler(int sigNum){
    stoppedSon=son;
    stoppedArg=commandNum;
    kill(stoppedSon,SIGTSTP);
    printf("\n%d stopped\n",stoppedSon);
}

void CHLD_handler(int sigNum){
    int bgPid=0;
    for(int i=0;i<length2;i++){
        bgPid= waitpid(bgPids[i],&status,WNOHANG);
        if(bgPid==bgPids[i]){
            bgPids[i]=-1;
        }
    }
}

void bgCheck(char* command,int* bgExist){
    for(int i=0;i< strlen(command);i++){
        if(command[i]=='b' && command[i+1]=='g'){
            for(int j=i+2;j< strlen(command);j++){
                if(command[j]!=' '){
                    break;
                }
                else{
                    *bgExist=1;
                }
            }
            break;
        }
        else{
            break;
        }
    }
}

int main() {
    char input[510];
    char* new[10];
    char* commands[11];
    char** variables=(char**)malloc(sizeof (char*));
    char** values=(char**)malloc(sizeof (char*));
    char* new2[10];
    char* cwd= getcwd(NULL,0);
    int length=0;
    int cont=0;
    int cont2=0;
    int enterCounter=0;
    int fileNum=0;
    int Pipe[2];
    int previousPipe[2];
    signal(SIGTSTP,TSTP_handler);
    signal(SIGCHLD,CHLD_handler);
    while(1) {
        int counter2 = 0;
        int ctrlZ=0;
        printf("#cmd:%d|#args:%d@%s ", numOfCommand, argumentsNum, cwd);
        if(!fgets(input, 510, stdin)){
            continue;
        }
        if (strcmp(input, "\n") == 0) {
            enterCounter++;
            if (enterCounter == 3) {
                free(bgPids);
                free(cwd);
                for(int v=0;v<length;v++){
                    free(variables[v]);
                    free(values[v]);
                }
                free(variables);
                free(values);
                exit(0);
            }
            continue;
        } else {
            enterCounter = 0;
        }
        echoEditing(input,&argumentsNum);
        new[counter2] = strtok(input, ";");
        while (new[counter2] != NULL) {
            counter2++;
            new[counter2] = strtok(NULL, ";");
        }
        for (int p = 0; p < counter2; p++) {
            int counter3 = 0;
            new2[counter3] = strtok(new[p], "|");
            while (new2[counter3] != NULL) {
                counter3++;
                new2[counter3] = strtok(NULL, "|");
            }
            for (int l = 0; l < counter3; l++) {
                commandNum = 0;
                int bgExist=0;
                cont = 0;
                int unpersentExist = 0;
                int bigSymbol = 0;
                cont2 = 0;
                int echoChecking = 0;
                int enoughArg = 0;
                int exists = 0;
                if(l<counter3-1) {
                    pipe(Pipe);
                }
                if (new2[l][strlen(new2[l]) - 1] == '\n') {
                    new2[l][strlen(new2[l]) - 1] = ' ';
                }
                bgCheck(new2[l],&bgExist);
                if(bgExist==1){
                    length2++;
                    if(length2-1==0){
                        bgPids=(int*)malloc(sizeof (int)*length2);
                        bgPids[0]=stoppedSon;
                    }
                    else{
                        bgPids= realloc(bgPids,sizeof (int)*length2);
                        bgPids[length2-1]=stoppedSon;
                    }
                    kill(stoppedSon,SIGCONT);
                    numOfCommand++;
                    argumentsNum+=stoppedArg;
                    stoppedArg=0;
                    stoppedSon=0;
                    continue;
                }
                for (int j = 0; j < strlen(new2[l]); j++) {
                    if (new2[l][j] == 'e' && new2[l][j + 1] == 'c' && new2[l][j + 2] == 'h' && new2[l][j + 3] == 'o') {
                        for (int m = j + 4; m < strlen(new2[l]); m++) {
                            if (new2[l][m] == ' ') {
                                continue;
                            } else if (new2[l][m] == '$') {
                                printCheck(variables, values, new2[l], length, &cont, &numOfCommand, &argumentsNum);
                            }
                        }
                    }
                }
                if (cont == 1) {
                    continue;
                }
                for (int w = 0; w < strlen(new2[l]); w++) {
                    if (new2[l][w] == 'e' && new2[l][w + 1] == 'c' && new2[l][w + 2] == 'h' && new2[l][w + 3] == 'o') {
                        deletingCoatations(new2[l]);
                        break;
                    }
                }
                if (echoChecking == 1) {
                    continue;
                }
                for (int o = 0; o < strlen(new2[l]); o++) {
                    if (new2[l][o] == ' ') {
                        continue;
                    } else if (new2[l][o] == '$') {
                        runCommand(variables, values, new2[l], length, &exists);
                    } else {
                        break;
                    }
                }
                if (exists == 1) {
                    continue;
                }
                for (int n = 0; n < strlen(new2[l]); n++) {
                    if (new2[l][n] == '=') {
                        for (int h = 0; h < n; h++) {
                            if (new2[l][h] == ' ') {
                                continue;
                            } else {
                                for (int m = n; m < strlen(new2[l]); m++) {
                                    if (new2[l][m] == ' ') {
                                        continue;
                                    } else {
                                        saving(variables, values, new2[l], &length, &cont2);
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
                if (cont2 == 1) {
                    continue;
                }
                hasBigSymbol(new2[l], &bigSymbol, &fileNum);
                unpersentEnd(new2[l], &unpersentExist);
                commands[commandNum] = strtok(new2[l], " ");
                while (commands[commandNum] != NULL) {
                    for (int s = 0; s < strlen(commands[commandNum]); s++) {
                        if (commands[commandNum][s] == '\xE1') {
                            commands[commandNum][s] = ' ';
                        } else if (commands[commandNum][s] == '\xE2') {
                            commands[commandNum][s] = ';';
                        }
                        else if (commands[commandNum][s] == '\xE3') {
                            commands[commandNum][s] = '>';
                        }
                        else if (commands[commandNum][s] == '\xE4') {
                            commands[commandNum][s] = '|';
                        }
                        else if (commands[commandNum][s] == '\xE5') {
                            commands[commandNum][s] = '&';
                        }
                    }
                    commandNum++;
                    if (commandNum >= 11) {
                        printf("too many arguments\n");
                        enoughArg = 1;
                        break;
                    }
                    commands[commandNum] = strtok(NULL, " ");
                }
                commands[commandNum] = NULL;
                if (enoughArg == 1) {
                    continue;
                }
                if (strcmp(commands[0], "cd") == 0) {
                    printf("cd is not supported\n");
                    continue;
                }
                son = fork();
                if (son == 0) {
                    if (bigSymbol == 1) {
                        dup2(fileNum, STDOUT_FILENO);
                        close(fileNum);
                    }
                    if(l > 0) {
                        dup2(previousPipe[0], STDIN_FILENO);
                        close(previousPipe[0]);
                        close(previousPipe[1]);
                    }
                    if(l < counter3-1) {
                        dup2(Pipe[1], STDOUT_FILENO);
                        close(Pipe[0]);
                        close(Pipe[1]);
                    }
                    if (execvp(commands[0], commands) == -1) {
                        perror("ERR");
                    }
                } else if (son > 0) {
                    if(l > 0) {
                        close(previousPipe[0]);
                        close(previousPipe[1]);
                    }
                    if(l < counter3-1) {
                        previousPipe[0] = Pipe[0];
                        previousPipe[1] = Pipe[1];
                    }
                    if (unpersentExist == 0) {
                        waitpid(son,&status,WUNTRACED);
                        if(WIFEXITED(status) && WEXITSTATUS(status) == 0){
                            numOfCommand++;
                            argumentsNum += commandNum;
                        }
                    }
                    else if(unpersentExist==1){
                        argumentsNum+=commandNum+1;
                        numOfCommand++;
                        length2++;
                        if(length2-1==0){
                            bgPids=(int*)malloc(sizeof (int)*length2);
                            bgPids[0]=son;
                        }
                        else{
                            bgPids= realloc(bgPids,sizeof (int)*length2);
                            bgPids[length2-1]=son;
                        }
                    }
                    if(bigSymbol==1){
                        argumentsNum+=2;
                        close(fileNum);
                    }
                } else {
                    perror("ERR");
                    free(bgPids);
                    free(cwd);
                    for(int v=0;v<length;v++){
                        free(variables[v]);
                        free(values[v]);
                    }
                    free(variables);
                    free(values);
                    exit(0);
                }
            }
        }
    }
}
