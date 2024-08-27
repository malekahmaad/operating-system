#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include "sim_mem.h"
#include <string.h>

char main_memory[MEMORY_SIZE];

int main() {
    char* swapName=new char[100];
    strcpy(swapName,"swap_file");
    char* execName=new char[100];
    strcpy(execName,"noice");
    char val;
    sim_mem mem_sm(execName, swapName ,16, 16, 8,16, 4);
    mem_sm.store(1026,'R');
    mem_sm.store(2050,')');
    mem_sm.store(1027,'@');
    mem_sm.store(3082,'9');
    mem_sm.load(2);
    mem_sm.load(3082);
    mem_sm.load(1026);
    mem_sm.print_memory();
    mem_sm.print_swap();
    mem_sm.print_page_table();
    delete execName;
    delete swapName;
}

