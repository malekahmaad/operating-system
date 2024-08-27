#include <iostream>
#include "sim_mem.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <cmath>

sim_mem::sim_mem(char exe_file_name[], char swap_file_name[], int text_size, int data_size,int bss_size,
                 int heap_stack_size, int page_size){
    this->text_size=text_size;
    this->data_size=data_size;
    this->bss_size=bss_size;
    this->heap_stack_size=heap_stack_size;
    this->page_size=page_size;
    this->swapfile_fd=open(swap_file_name,O_RDWR | O_CREAT | O_TRUNC,
                           S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if(this->swapfile_fd==-1){
        perror("couldnt open file");
    }
    this->program_fd=open(exe_file_name,O_RDONLY );
    if(this->program_fd==-1){
        perror("file doesnt exist");
        exit(0);
    }
    for(int i=0;i<200;i++) {
        main_memory[i] = '0';
    }
    this->page_table=new page_descriptor *[4];
    this->page_table[0]=new page_descriptor[this->text_size/this->page_size];
    this->page_table[1]=new page_descriptor[this->data_size/this->page_size];
    this->page_table[2]=new page_descriptor[this->bss_size/this->page_size];
    this->page_table[3]=new page_descriptor[this->heap_stack_size/this->page_size];
    for(int j=0;j<this->text_size/this->page_size;j++){
        this->page_table[0][j].dirty=false;
        this->page_table[0][j].valid=false;
        this->page_table[0][j].frame=-1;
        this->page_table[0][j].swap_index=-1;
    }
    for(int j=0;j<this->data_size/this->page_size;j++){
        this->page_table[1][j].dirty=false;
        this->page_table[1][j].valid=false;
        this->page_table[1][j].frame=-1;
        this->page_table[1][j].swap_index=-1;
    }
    for(int j=0;j<this->bss_size/this->page_size;j++){
        this->page_table[2][j].dirty=false;
        this->page_table[2][j].valid=false;
        this->page_table[2][j].frame=-1;
        this->page_table[2][j].swap_index=-1;
    }
    for(int j=0;j<this->heap_stack_size/this->page_size;j++){
        this->page_table[3][j].dirty=false;
        this->page_table[3][j].valid=false;
        this->page_table[3][j].frame=-1;
        this->page_table[3][j].swap_index=-1;
    }
    int y = this->bss_size + this->data_size + this->heap_stack_size;
    char letter[3]="0";
    for(int k=0;k<y;k++){
        if(write(this->swapfile_fd,letter,sizeof(char))==-1){
            perror("couldnt write");
        }
    }
    swapCheck=new int[y/page_size];
    for(int q=0;q<y/page_size;q++){
        swapCheck[q]=-1;
    }
    this->infos=new frame_info[MEMORY_SIZE/this->page_size];
    for(int l=0;l<MEMORY_SIZE/this->page_size;l++){
        infos[l].address=-1;
        infos[l].counter=-1;
    }
}

void sim_mem::print_memory() {
    int i;
    printf("\n Physical memory\n");
    for(i = 0; i < MEMORY_SIZE; i++) {
        printf("[%c]\n", main_memory[i]);
    }
}

void sim_mem::print_swap() {
    char* str = (char*)malloc(this->page_size *sizeof(char));
    int i;
    printf("\n Swap memory\n");
    lseek(swapfile_fd, 0, SEEK_SET); // go to the start of the file
    while(read(swapfile_fd, str, this->page_size) == this->page_size) {
        for(i = 0; i < page_size; i++) {
            printf("%d - [%c]\t", i, str[i]);
        }
        printf("\n");
    }
    delete str;
}

void sim_mem::print_page_table() {
    int i;
    int num_of_txt_pages = text_size / page_size;
    int num_of_data_pages = data_size / page_size;
    int num_of_bss_pages = bss_size / page_size;
    int num_of_stack_heap_pages = heap_stack_size / page_size;
    printf("Valid\t Dirty\t Frame\t Swap index\n");
    for (i = 0; i < num_of_txt_pages; i++) {
        printf("[%d]\t[%d]\t[%d]\t[%d]\n",
               page_table[0][i].valid,
               page_table[0][i].dirty,
               page_table[0][i].frame,
               page_table[0][i].swap_index);

    }
    printf("Valid\t Dirty\t Frame\t Swap index\n");
    for (i = 0; i < num_of_data_pages; i++) {
        printf("[%d]\t[%d]\t[%d]\t[%d]\n",
               page_table[1][i].valid,
               page_table[1][i].dirty,
               page_table[1][i].frame,
               page_table[1][i].swap_index);

    }
    printf("Valid\t Dirty\t Frame\t Swap index\n");
    for (i = 0; i < num_of_bss_pages; i++) {
        printf("[%d]\t[%d]\t[%d]\t[%d]\n",
               page_table[2][i].valid,
               page_table[2][i].dirty,
               page_table[2][i].frame,
               page_table[2][i].swap_index);

    }
    printf("Valid\t Dirty\t Frame\t Swap index\n");
    for (i = 0; i < num_of_stack_heap_pages; i++) {
        printf("[%d]\t[%d]\t[%d]\t[%d]\n",
               page_table[3][i].valid,
               page_table[3][i].dirty,
               page_table[3][i].frame,
               page_table[3][i].swap_index);
    }
}

void sim_mem::store(int address, char value){
    int offset,out,in,position,position2,offset2,out2,in2;
    findingAddress(address,&offset,&in,&out,page_size);
    if(out==0){
        perror("you cant store a text");
        return ;
    }
    else if(out==1){
        if(in>=data_size/page_size){
            perror("this address is more than the pages in the data");
            return ;
        }
    }
    else if(out==2){
        if(in>=bss_size/page_size){
            perror("this address is more than the pages in the bss");
            return ;
        }
    }
    else if(out==3){
        if(in>=heap_stack_size/page_size){
            perror("this address is more than the pages in the heap_stack");
            return ;
        }
    }
    if(out>1 && page_table[out][in].valid==false && page_table[out][in].dirty==false){
        position=memoryCheck(page_size,infos);
        if(position==-1){
            position2=oldestCheck(page_size,infos);
            findingAddress(infos[position2].address,&offset2,&in2,&out2,page_size);
            if(page_table[out2][in2].dirty==true){
                int counter=0;
                char x;
                for(int i=0;i<(data_size+bss_size+heap_stack_size)/page_size;i++){
                    if(swapCheck[i]==-1){
                        swapCheck[i]=1;
                        break;
                    }
                    counter++;
                }
                if(lseek(swapfile_fd,counter*page_size,SEEK_SET)==-1){
                    perror("couldnt do lseek");
                }
                char letter[2]="\0";
                for(int j=position2*page_size;j<(position2*page_size)+page_size;j++){
                    letter[0]=main_memory[j];
                    if(write(swapfile_fd,letter,sizeof(char))==-1){
                        perror("couldnt write");
                    }
                }
                page_table[out2][in2].swap_index=counter;
            }
            infos[position2].address=address;
            for(int i=0;i<page_size;i++){
                main_memory[(page_size*position2)+i]='0';
            }
            page_table[out][in].valid=true;
            page_table[out][in].dirty=true;
            page_table[out][in].frame=position2;
            page_table[out2][in2].valid=false;
            page_table[out2][in2].frame=-1;
            infos[position2].counter=-1;
            addCounter(page_size,infos);
            main_memory[(page_size*position2)+offset]=value;
        }
        else{
            infos[position].address=address;
            for(int i=0;i<page_size;i++){
                char r='0';
                main_memory[(page_size*position)+i]=r;
            }
            page_table[out][in].valid=true;
            page_table[out][in].dirty=true;
            page_table[out][in].frame=position;
            addCounter(page_size,infos);
            main_memory[(position*page_size)+offset]=value;
        }
    }
    else {
        load(address);
        main_memory[(page_table[out][in].frame*page_size)+offset]=value;
        page_table[out][in].dirty=true;
    }
}

char sim_mem::load(int address){
    int offset,out,in,position,position2,offset2,out2,in2;
    findingAddress(address,&offset,&in,&out,page_size);
    if(out==0){
        if(in>=text_size/page_size){
            perror("this address is more than the pages in the text");
            return '\0';
        }
    }
    else if(out==1){
        if(in>=data_size/page_size){
            perror("this address is more than the pages in the data");
            return '\0';
        }
    }
    else if(out==2){
        if(in>=bss_size/page_size){
            perror("this address is more than the pages in the bss");
            return '\0';
            }
    }
    else if(out==3){
        if(in>=heap_stack_size/page_size){
            perror("this address is more than the pages in the heap_stack");
            return '\0';
        }
    }
    if(page_table[out][in].valid==true){
        infos[page_table[out][in].frame].counter=-1;
        addCounter(page_size,infos);
        return main_memory[((page_table[out][in].frame)*page_size)+offset];
    }
    else if(page_table[out][in].dirty==true && page_table[out][in].swap_index!=-1){
        position2=oldestCheck(page_size,infos);
        findingAddress(infos[position2].address,&offset2,&in2,&out2,page_size);
        if(page_table[out2][in2].dirty==true){
            int counter=0;
            for(int i=0;i<(data_size+bss_size+heap_stack_size)/page_size;i++){
                if(swapCheck[i]==-1){
                    swapCheck[i]=1;
                    break;
                }
                counter++;
            }
            if(lseek(swapfile_fd,counter*page_size,SEEK_SET)==-1){
                perror("couldnt do lseek");
            }
            char letter[2]="\0";
            for(int j=position2*page_size;j<(position2*page_size)+page_size;j++){
                letter[0]=main_memory[j];
                if(write(swapfile_fd,letter,sizeof(char))==-1){
                    perror("couldnt write");
                }
            }
            page_table[out2][in2].swap_index=counter;
        }
        infos[position2].address=address;
        if(lseek(swapfile_fd,(page_table[out][in].swap_index)*page_size,SEEK_SET)==-1){
            perror("couldnt do lssek");
        }
        for(int i=0;i<page_size;i++){
            char r;
            if(read(swapfile_fd, &r, sizeof(char))==-1){
                perror("couldnt read");
            }
            main_memory[(page_size*position2)+i]=r;
        }
        if(lseek(swapfile_fd,(page_table[out][in].swap_index)*page_size,SEEK_SET)==-1){
            perror("couldnt do lseek");
        }
        char word[2]="0";
        for(int i=0;i<page_size;i++){
            if(write(swapfile_fd, word, sizeof(char))==-1){
                perror("couldnt write");
            }
        }
        swapCheck[page_table[out][in].swap_index]=-1;
        page_table[out][in].valid=true;
        page_table[out][in].frame=position2;
        page_table[out2][in2].valid=false;
        page_table[out2][in2].frame=-1;
        infos[position2].counter=-1;
        addCounter(page_size,infos);
        page_table[out][in].swap_index=-1;
        return main_memory[(page_size*position2)+offset];
    }
    else{
        if(out==3){
            perror("its a heap_stack cant do load firstly\n");
            return '\0';
        }
        else{
            position=memoryCheck(page_size,infos);
            int filePos=0;
            if(out==1){
                filePos=text_size;
            }
            else if(out==2){
                filePos=text_size+data_size;
            }
            if(position==-1){
                position2=oldestCheck(page_size,infos);
                findingAddress(infos[position2].address,&offset2,&in2,&out2,page_size);
                if(page_table[out2][in2].dirty==true){
                    int counter=0;
                    char x;
                    for(int i=0;i<(data_size+bss_size+heap_stack_size)/page_size;i++){
                        if(swapCheck[i]==-1){
                            swapCheck[i]=1;
                            break;
                        }
                        counter++;
                    }
                    if(lseek(swapfile_fd,counter*page_size,SEEK_SET)==-1){
                        perror("couldnt do lseek");
                    }
                    char letter[2]="\0";
                    for(int j=position2*page_size;j<(position2*page_size)+page_size;j++){
                        letter[0]=main_memory[j];
                        if(write(swapfile_fd,letter,sizeof(char))==-1){
                            perror("couldnt write");
                        }
                    }
                    page_table[out2][in2].swap_index=counter;
                }
                infos[position2].address=address;
                if(lseek(program_fd,(in*page_size)+filePos,SEEK_SET)==-1){
                    perror("couldnt do lseek");
                }
                for(int i=0;i<page_size;i++){
                    char r;
                    if(read(program_fd, &r, sizeof(char))==-1){
                        perror("couldnt read");
                    }
                    main_memory[(page_size*position2)+i]=r;
                }
                page_table[out][in].valid=true;
                page_table[out][in].frame=position2;
                page_table[out2][in2].valid=false;
                page_table[out2][in2].frame=-1;
                infos[position2].counter=-1;
                addCounter(page_size,infos);
                return main_memory[(page_size*position2)+offset];
            }
            else{
                infos[position].address=address;
                if(lseek(program_fd,(in*page_size)+filePos,SEEK_SET)==-1){
                    perror("couldnt do lseek");
                }
                for(int i=0;i<page_size;i++){
                    char r;
                    if(read(program_fd, &r, sizeof(char))==-1){
                        perror("couldnt read");
                    }
                    main_memory[(page_size*position)+i]=r;
                }
                page_table[out][in].valid=true;
                page_table[out][in].frame=position;
                addCounter(page_size,infos);
                return main_memory[(page_size*position)+offset];
            }
        }
    }
    return '\0';
}

sim_mem::~sim_mem (){
    if(close(swapfile_fd)==-1){
        perror("couldnt close swap file");
    }
    if(close(program_fd)==-1){
        perror("couldnt close exec file");
    }
    for(int i=0;i<4;i++) {
        delete page_table[i];
    }
    delete[] page_table;
    delete infos;
    delete swapCheck;
}

void findingAddress(int address,int* offset,int* inPage,int* outPage,int pageSize){
    int binDigits=0;
    int copy=address;
    while(copy!=0){
        copy=copy/2;
        binDigits++;
    }
    if(binDigits<=12){
        binDigits=12;
    }
    int div=pow(2,binDigits-2);
    copy=address % div;
    *outPage=address / div;
    *offset=copy % pageSize;
    *inPage=copy / pageSize;
}

int memoryCheck(int pageSize,frame_info* infos){
    for(int i=0;i<MEMORY_SIZE/pageSize;i++){
        if(infos[i].address==-1){
            return i;
        }
    }
    return -1;
}

void addCounter(int pageSize,frame_info* infos){
    for(int i=0;i<MEMORY_SIZE/pageSize;i++){
        if(infos[i].address!=-1){
            infos[i].counter++;
        }
    }
}

int oldestCheck(int pageSize,frame_info* infos){
    int max=0,position=0;
    for(int i=0;i<MEMORY_SIZE/pageSize;i++){
        if(infos[i].counter>=max){
            max=infos[i].counter;
            position=i;
        }
    }
    return position;
}

