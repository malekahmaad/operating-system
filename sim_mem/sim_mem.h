#ifndef EX4_SIM_MEM_H
#define EX4_SIM_MEM_H
#define MEMORY_SIZE 200
extern char main_memory[MEMORY_SIZE];

typedef struct page_descriptor
{
    bool valid;
    int frame;
    bool dirty;
    int swap_index;
} page_descriptor;

typedef struct frame_info
{
    int counter;
    int address;
} frame_info;

class sim_mem {
    int swapfile_fd; //swap file fd
    int program_fd; //executable file fd
    int text_size;
    int data_size;
    int bss_size;
    int heap_stack_size;
    int num_of_pages;
    int page_size;
    int num_of_proc;
    int* swapCheck;
    page_descriptor **page_table; //pointer to page table
    frame_info* infos;
public:
    sim_mem(char exe_file_name[], char swap_file_name[], int text_size, int data_size,int bss_size,
            int heap_stack_size, int page_size);

    ~sim_mem();

    char load(int address);

    void store(int address, char value);

    void print_memory();

    void print_swap();

    void print_page_table();
};

void addCounter(int pageSize,frame_info* infos);
int memoryCheck(int pageSize,frame_info* infos);
void findingAddress(int address,int* offset,int* inPage,int* outPage,int pageSize);
int oldestCheck(int pageSize,frame_info* infos);
#endif

