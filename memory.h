#ifndef MEMORY_H
#define MEMORY_H

#define TOTAL_MEMORY_SIZE 128 // Total physical memory in KB (example)
#define PAGE_SIZE 16          // Page size in KB (example)
#define NUM_FRAMES (TOTAL_MEMORY_SIZE / PAGE_SIZE)
#define MAX_PAGES_PER_PROCESS 10 // Max logical pages a process can have

typedef struct {
    int frame_number;
    int valid; // 1 if in physical memory, 0 otherwise
} PageTableEntry;

typedef struct {
    int pid;
    PageTableEntry page_table[MAX_PAGES_PER_PROCESS];
    int num_pages_requested; // How many pages this process needs
} ProcessMemoryInfo;

void init_memory_management();
void request_memory(ProcessMemoryInfo* p_info, int pid, int num_pages);
void access_memory(ProcessMemoryInfo* p_info, int pid, int page_num);
void display_memory_status(ProcessMemoryInfo p_infos[], int num_processes); // Modified to take array
int get_page_fault_count();

#endif // MEMORY_H
