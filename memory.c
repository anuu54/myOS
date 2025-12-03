#include <stdio.h>
#include <stdlib.h>
#include "memory.h"

int physical_frames[NUM_FRAMES]; // Stores PID of process using the frame, or -1 if free.
int frame_to_pid_map[NUM_FRAMES]; // Stores PID for frame
int frame_to_page_num_map[NUM_FRAMES]; // Stores which page of that PID is in this frame
ProcessMemoryInfo* frame_to_process_info_map[NUM_FRAMES]; // Pointer to the ProcessMemoryInfo of the process owning the frame


int page_fault_count = 0;
int next_frame_to_replace_fifo = 0;

void init_memory_management() {
    printf("\n-- Paging Memory Management Simulation --\n");
    printf("Total Memory: %dKB, Page Size: %dKB, Num Frames: %d\n", TOTAL_MEMORY_SIZE, PAGE_SIZE, NUM_FRAMES);
    for (int i = 0; i < NUM_FRAMES; i++) {
        physical_frames[i] = -1; // -1 indicates frame is free
        frame_to_pid_map[i] = -1;
        frame_to_page_num_map[i] = -1;
        frame_to_process_info_map[i] = NULL;
    }
    page_fault_count = 0;
    next_frame_to_replace_fifo = 0;
}

void request_memory(ProcessMemoryInfo* p_info, int pid, int num_pages_needed) {
    if (num_pages_needed <= 0 || num_pages_needed > MAX_PAGES_PER_PROCESS) {
        printf("Process %d: Invalid number of pages requested (%d). Max is %d, Min is 1.\n",
               pid, num_pages_needed, MAX_PAGES_PER_PROCESS);
        p_info->num_pages_requested = 0;
        p_info->pid = pid; // Still set pid for identification
        return;
    }
    p_info->pid = pid;
    p_info->num_pages_requested = num_pages_needed;
    for (int i = 0; i < MAX_PAGES_PER_PROCESS; i++) {
        p_info->page_table[i].valid = 0;
        p_info->page_table[i].frame_number = -1;
    }
    printf("Process %d initialized, requires %d pages.\n", pid, num_pages_needed);
}

void access_memory(ProcessMemoryInfo* p_info, int pid, int page_num) {
    if (p_info == NULL || p_info->pid != pid) {
        printf("Error: ProcessMemoryInfo is NULL or does not match PID %d for access.\n", pid);
        return;
    }
    if (p_info->num_pages_requested == 0) {
        printf("Process %d: No pages were successfully requested. Cannot access memory.\n", pid);
        return;
    }
    if (page_num < 0 || page_num >= p_info->num_pages_requested) {
        printf("Process %d: Invalid page access %d (Requested pages for proc: %d).\n",
               pid, page_num, p_info->num_pages_requested);
        return;
    }

    printf("Process %d accessing page %d: ", pid, page_num);
    if (p_info->page_table[page_num].valid == 1) {
        printf("Page HIT. In Frame %d.\n", p_info->page_table[page_num].frame_number);
    } else {
        printf("Page FAULT. ");
        page_fault_count++;
        
        int free_frame_idx = -1;
        for (int i = 0; i < NUM_FRAMES; i++) {
            if (physical_frames[i] == -1) {
                free_frame_idx = i;
                break;
            }
        }

        if (free_frame_idx != -1) {
            physical_frames[free_frame_idx] = pid; // Mark frame with PID
            frame_to_pid_map[free_frame_idx] = pid;
            frame_to_page_num_map[free_frame_idx] = page_num;
            frame_to_process_info_map[free_frame_idx] = p_info;

            p_info->page_table[page_num].frame_number = free_frame_idx;
            p_info->page_table[page_num].valid = 1;
            printf("Allocated to Frame %d.\n", free_frame_idx);
        } else {
            // FIFO Page Replacement
            printf("No free frames. Replacing Frame %d (FIFO). ", next_frame_to_replace_fifo);
            
            // Invalidate the page table entry of the victim process
            int victim_pid = frame_to_pid_map[next_frame_to_replace_fifo];
            int victim_page_num = frame_to_page_num_map[next_frame_to_replace_fifo];
            ProcessMemoryInfo* victim_p_info = frame_to_process_info_map[next_frame_to_replace_fifo];

            if (victim_p_info != NULL && victim_p_info->pid == victim_pid) {
                 if(victim_page_num >= 0 && victim_page_num < victim_p_info->num_pages_requested) {
                    victim_p_info->page_table[victim_page_num].valid = 0;
                    victim_p_info->page_table[victim_page_num].frame_number = -1;
                    printf("Evicted P%d Page %d from Frame %d. ", victim_pid, victim_page_num, next_frame_to_replace_fifo);
                 } else {
                    printf("Warning: Inconsistent victim page data for P%d. ", victim_pid);
                 }
            } else {
                 printf("Warning: Could not find victim process info for P%d or PID mismatch. ", victim_pid);
            }
            
            physical_frames[next_frame_to_replace_fifo] = pid; // Current process takes over the frame
            frame_to_pid_map[next_frame_to_replace_fifo] = pid;
            frame_to_page_num_map[next_frame_to_replace_fifo] = page_num;
            frame_to_process_info_map[next_frame_to_replace_fifo] = p_info;

            p_info->page_table[page_num].frame_number = next_frame_to_replace_fifo;
            p_info->page_table[page_num].valid = 1;
            printf("Allocated to Frame %d.\n", next_frame_to_replace_fifo);

            next_frame_to_replace_fifo = (next_frame_to_replace_fifo + 1) % NUM_FRAMES;
        }
    }
}

void display_memory_status(ProcessMemoryInfo p_infos[], int num_processes_active) {
    printf("\n--- Memory Status ---\n");
    printf("Physical Frames Status (Frame: PID | Page of PID):\n");
    for (int i = 0; i < NUM_FRAMES; i++) {
        if (physical_frames[i] != -1) {
            printf("Frame %d: P%d | Page %d\n", i, frame_to_pid_map[i], frame_to_page_num_map[i]);
        } else {
            printf("Frame %d: Free\n", i);
        }
    }

    for (int p = 0; p < num_processes_active; p++) {
        if (p_infos[p].pid == 0 && p_infos[p].num_pages_requested == 0) continue; // Skip uninitialized/invalid
        printf("\nProcess %d (PID) Page Table (Requested: %d pages):\n", p_infos[p].pid, p_infos[p].num_pages_requested);
        printf("Log.Page | Valid | Phys.Frame\n");
        printf("------------------------------\n");
        for (int i = 0; i < p_infos[p].num_pages_requested; i++) {
            printf("%-8d | %-5d | %-10d\n", i, p_infos[p].page_table[i].valid, p_infos[p].page_table[i].frame_number);
        }
    }
    printf("\nTotal Page Faults: %d\n", page_fault_count);
}

int get_page_fault_count() {
    return page_fault_count;
}
