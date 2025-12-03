#include <stdio.h>
#include <string.h> 
#include <stdlib.h> 

#include "scheduler.h"
#include "memory.h"
#include "filesystem.h"
#include "disk.h"

#define MAX_MEM_PROCESSES_MAIN 5
ProcessMemoryInfo mem_proc_infos[MAX_MEM_PROCESSES_MAIN];
int current_mem_processes_count = 0;

int memory_initialized_flag = 0;
int fs_initialized_flag = 0;

typedef struct {
    char name[50];
    int pages_needed;
    char file_to_access[50];
} ProgramProfile;

#define NUM_KNOWN_PROGRAMS 3
ProgramProfile known_programs[NUM_KNOWN_PROGRAMS] = {
    {"editor", 4, "mydoc.txt"},
    {"compiler", 6, "source.c"},
    {"player", 2, "song.mp3"}
};


#ifndef FILESYSTEM_C_INCLUDED // Guard to prevent redefinition if filesystem.c includes this main.h in a complex setup
extern File file_system[MAX_FILES]; // Assuming MAX_FILES is defined in filesystem.h
#endif


void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Function to tokenize input string
#define MAX_ARGS 10
#define MAX_CMD_LEN 100
int parse_command(char* input, char** command, char* args[]) {
    char* token;
    int argc = 0;

    char input_copy[MAX_CMD_LEN];
    strncpy(input_copy, input, MAX_CMD_LEN -1);
    input_copy[MAX_CMD_LEN -1] = '\0';

    token = strtok(input_copy, " \t\n"); 
    if (token == NULL) {
        return 0; 
    }
    *command = strdup(token); 

    while ((token = strtok(NULL, " \t\n")) != NULL && argc < MAX_ARGS) {
        args[argc++] = strdup(token);
    }
    return argc + 1; 
}

void free_parsed_command(char* command, char* args[], int arg_count) {
    if (command) free(command);
    // arg_count includes the command itself. So if arg_count is 1, args array is empty.
    // If arg_count is 2, args[0] is valid. Loop goes from i=0 to (arg_count-1 -1).
    for (int i = 0; i < arg_count - 1; ++i) { 
        if (args[i]) free(args[i]);
    }
}

int main() {
    char input_line[MAX_CMD_LEN];
    char* command = NULL;
    char* args[MAX_ARGS];
    int arg_count;

    for(int i=0; i<MAX_MEM_PROCESSES_MAIN; ++i) {
        mem_proc_infos[i].pid = 0; 
        mem_proc_infos[i].num_pages_requested = 0;
    }

    printf("Welcome to MyOS Simulator Shell!\n");
    printf("Type 'help' for a list of commands.\n");

    while (1) {
        printf("myos> ");
        fflush(stdout); 

        if (fgets(input_line, sizeof(input_line), stdin) == NULL) {
            printf("\nExiting MyOS shell (EOF).\n");
            break; 
        }

        input_line[strcspn(input_line, "\n")] = 0;

        if (strlen(input_line) == 0) {
            continue; 
        }
        
        for(int i=0; i < MAX_ARGS; ++i) args[i] = NULL;

        arg_count = parse_command(input_line, &command, args);

        if (arg_count == 0 || command == NULL) {
            if(command) free(command); 
            command = NULL;
            continue; 
        }

        if (strcmp(command, "exit") == 0) {
            printf("Exiting MyOS shell.\n");
            free_parsed_command(command, args, arg_count);
            break;
        } else if (strcmp(command, "help") == 0) {
            printf("Available commands:\n");
            printf("  help                            - Show this help message\n");
            printf("  exit                            - Exit the MyOS shell\n");
            printf("  rr <time_quantum>               - Simulate Round Robin (e.g., rr 4)\n");
            printf("  mem_init                        - Initialize Memory Management\n");
            printf("  mem_req <pid> <num_pages>       - Request memory (e.g., mem_req 101 3)\n");
            printf("  mem_access <pid> <page_num>     - Access memory (e.g., mem_access 101 0)\n");
            printf("  mem_status                      - Display Memory Status\n");
            printf("  fs_init                         - Initialize File System\n");
            printf("  fs_create <name> <size>         - Create file (e.g., fs_create doc.txt 100)\n");
            printf("  fs_delete <name>                - Delete file (e.g., fs_delete doc.txt)\n");
            printf("  fs_list                         - List files\n");
            printf("  disk_fcfs <head> <cyl> <r1> ... - FCFS Disk (e.g., disk_fcfs 50 200 98 183)\n");
            printf("  exec_process <program_name>     - Simulate full lifecycle (e.g., exec_process editor)\n");
            printf("                                    Known programs: editor, compiler, player\n");

        } else if (strcmp(command, "exec_process") == 0) {
            if (arg_count < 2 || args[0] == NULL) {
                printf("Usage: exec_process <program_name>\n");
                printf("Known programs: editor, compiler, player. Others use defaults.\n");
            } else {
                char* program_name_arg = args[0];
                int process_id = 1000 + current_mem_processes_count + 1; // Semi-unique PID
                
                int required_pages = 3; // Default
                char file_to_access[50] = "default_data.txt"; // Default
                int profile_found = 0;

                for (int i = 0; i < NUM_KNOWN_PROGRAMS; i++) {
                    if (strcmp(known_programs[i].name, program_name_arg) == 0) {
                        required_pages = known_programs[i].pages_needed;
                        strncpy(file_to_access, known_programs[i].file_to_access, sizeof(file_to_access) - 1);
                        file_to_access[sizeof(file_to_access) - 1] = '\0';
                        profile_found = 1;
                        printf("[OS_SIM_INFO] Using profile for known program: '%s'\n", program_name_arg);
                        break;
                    }
                }
                if (!profile_found) {
                    printf("[OS_SIM_INFO] Program '%s' not in known profiles. Using default settings (Pages: %d, File: %s).\n", 
                           program_name_arg, required_pages, file_to_access);
                }

                printf("\n--- Simulating Lifecycle for Program: %s (PID: %d) ---\n", program_name_arg, process_id);

                printf("[OS_SIM | PID: %d | State: NEW] Process '%s' created.\n", process_id, program_name_arg);
                printf("[OS_SIM | PID: %d | Action] System identifies '%s' for execution (located on simulated secondary storage).\n", process_id, program_name_arg);
                printf("[OS_SIM | PID: %d | Action] Disk I/O: Fetching executable for '%s'...\n", process_id, program_name_arg);
                printf("[OS_SIM | PID: %d | Action] Disk I/O: '%s' loaded from secondary storage into a temporary staging area.\n", process_id, program_name_arg);
                printf("[OS_SIM | PID: %d | Action] Memory Manager: Requesting %d pages for '%s'...\n", process_id, required_pages, program_name_arg);
                
                if (!memory_initialized_flag) {
                    init_memory_management(); 
                    memory_initialized_flag = 1;
                    current_mem_processes_count = 0; // Reset count as mem_init was called
                     for(int i=0; i<MAX_MEM_PROCESSES_MAIN; ++i) {mem_proc_infos[i].pid = 0; mem_proc_infos[i].num_pages_requested = 0;}
                }
                
                int mem_idx = -1;
                // Find an unused slot OR the next available if array is not full
                for(int i=0; i < MAX_MEM_PROCESSES_MAIN; ++i) {
                    if(mem_proc_infos[i].pid == 0) { // Found an unused slot (e.g. after a process finished and was cleaned up - not fully implemented cleanup)
                        mem_idx = i;
                        break;
                    }
                }
                if (mem_idx == -1) { // No empty slot found, try to use next if available
                    if (current_mem_processes_count < MAX_MEM_PROCESSES_MAIN) {
                        mem_idx = current_mem_processes_count;
                    } else {
                        printf("[OS_SIM_ERROR | PID: %d] No space in mem_proc_infos array to track new process memory. Increase MAX_MEM_PROCESSES_MAIN.\n", process_id);
                        free_parsed_command(command, args, arg_count); command = NULL;
                        continue;
                    }
                }
                
                request_memory(&mem_proc_infos[mem_idx], process_id, required_pages);
                
                if(mem_proc_infos[mem_idx].num_pages_requested > 0) { // If request_memory was successful
                    if(mem_proc_infos[mem_idx].pid == process_id) { // Ensure this slot is now for our current process_id
                        // Only increment current_mem_processes_count if this is a truly new slot being used,
                        // or if request_memory itself should return whether it's a new process.
                        // For simplicity, if mem_idx was an empty slot (pid 0) or points to a new process slot, count it.
                        // This logic can be tricky if processes are "removed" and slots are reused.
                        // Let's assume request_memory sets the pid and we can check if it was a new assignment.
                        // A simpler way for this demo: if we got a valid mem_idx and request_memory succeeded, we assume it's managed.
                        // The current_mem_processes_count tracks how many active processes we *think* we have.
                        int is_new_active_process_slot = 1; // Assume true for now for demo simplicity
                        if(is_new_active_process_slot && mem_idx >= current_mem_processes_count) {
                            // This implies we're using a slot at the end of the currently "filled" portion
                             current_mem_processes_count = mem_idx + 1;
                        } else if (is_new_active_process_slot && mem_proc_infos[mem_idx].pid == process_id) {
                            // This could be a reused slot, current_mem_processes_count might not need to change if it reflects "highest index used + 1"
                            // This part needs more robust tracking if processes are properly deallocated
                        }
                        // For this demo, let's simplify: if a slot is used for this new process, and it's beyond current_mem_processes_count, update.
                        // A better approach: init_memory_management resets current_mem_processes_count.
                        // Each successful unique PID request increments it.
                        // To ensure it's truly unique for this session if previous runs filled array:
                        int pid_already_exists = 0;
                        for(int k=0; k<current_mem_processes_count; ++k) {
                            if (k != mem_idx && mem_proc_infos[k].pid == process_id) {
                                pid_already_exists = 1; break;
                            }
                        }
                        if (!pid_already_exists && mem_idx == current_mem_processes_count && current_mem_processes_count < MAX_MEM_PROCESSES_MAIN) {
                            current_mem_processes_count++;
                        }


                        for (int i = 0; i < required_pages; i++) {
                            printf("[OS_SIM | PID: %d | Action] Memory Manager: Loading page %d into main memory...\n", process_id, i);
                            access_memory(&mem_proc_infos[mem_idx], process_id, i); 
                        }
                        printf("[OS_SIM | PID: %d | Action] Memory Manager: PID %d successfully loaded into main memory.\n", process_id, process_id);
                        printf("[OS_SIM | PID: %d | State: READY] Process '%s' is in main memory, waiting for CPU.\n", process_id, program_name_arg);
                    } else {
                         printf("[OS_SIM_ERROR | PID: %d] Memory slot confusion after request_memory for '%s'.\n", process_id, program_name_arg);
                         free_parsed_command(command, args, arg_count); command = NULL;
                         continue;
                    }
                } else {
                    printf("[OS_SIM_ERROR | PID: %d] Memory request failed for '%s'. Cannot proceed to READY state.\n", process_id, program_name_arg);
                    free_parsed_command(command, args, arg_count); command = NULL;
                    continue;
                }
        
                printf("[OS_SIM | PID: %d | Action] Scheduler: Dispatching process '%s' to CPU...\n", process_id, program_name_arg);
                printf("[OS_SIM | PID: %d | State: RUNNING] Process '%s' is now executing instructions on the CPU.\n", process_id, program_name_arg);
                printf("[OS_SIM | PID: %d | Action] '%s' is performing its computation...\n", process_id, program_name_arg);
                printf("[OS_SIM | PID: %d | Action] Process '%s' requests to open file '%s'.\n", process_id, program_name_arg, file_to_access);
                printf("[OS_SIM | PID: %d | State: WAITING] Process '%s' blocked, waiting for file '%s' operation.\n", process_id, program_name_arg, file_to_access);
                
                if (!fs_initialized_flag) {
                    init_filesystem(); 
                    fs_initialized_flag = 1;
                }
                printf("[OS_SIM | PID: %d | Action] File System: Servicing I/O request for '%s'...\n", process_id, file_to_access);
                
                int file_exists_flag = 0;
                // Accessing global 'file_system' array - ensure filesystem.h defines File and MAX_FILES
                // And that file_system is declared in filesystem.c without static, or provide a helper.
                // For this demo, assuming direct access is possible via extern.
                for(int f_idx=0; f_idx < MAX_FILES; ++f_idx) { // MAX_FILES should be from filesystem.h
                    if(file_system[f_idx].allocated && strcmp(file_system[f_idx].name, file_to_access) == 0) {
                        file_exists_flag = 1;
                        break;
                    }
                }

                if (file_exists_flag) {
                    printf("[OS_SIM | PID: %d | Action] File System: File '%s' found and opened.\n", process_id, file_to_access);
                } else {
                    printf("[OS_SIM | PID: %d | Action] File System: File '%s' not found. Creating it...\n", process_id, file_to_access);
                    create_file_sim(file_to_access, 20); 
                    printf("[OS_SIM | PID: %d | Action] File System: File '%s' created and opened.\n", process_id, file_to_access);
                }
                printf("[OS_SIM | PID: %d | Action] File System: I/O operation for '%s' completed.\n", process_id, file_to_access);
                printf("[OS_SIM | PID: %d | State: READY] Process '%s' moved back to Ready Queue after I/O.\n", process_id, program_name_arg);
                printf("[OS_SIM | PID: %d | Action] Scheduler: Dispatching process '%s' to CPU again...\n", process_id, program_name_arg);
                printf("[OS_SIM | PID: %d | State: RUNNING] Process '%s' continues execution...\n", process_id, program_name_arg);
                printf("[OS_SIM | PID: %d | Action] '%s' performing final computations...\n", process_id, program_name_arg);
                printf("[OS_SIM | PID: %d | State: TERMINATED] Process '%s' has completed its execution.\n", process_id, program_name_arg);
                printf("[OS_SIM | PID: %d | Action] OS is deallocating memory and removing PID %d from process table (conceptually).\n", process_id, process_id);
                // To truly deallocate:
                // 1. Mark mem_proc_infos[mem_idx].pid = 0; (or some other unused indicator)
                // 2. Invalidate its page table entries.
                // 3. Free its frames in physical_frames[] and associated maps in memory.c
                // This would make current_mem_processes_count management more complex (it would decrease).
                // For now, FIFO replacement handles frame reuse eventually.
                if(mem_idx != -1 && mem_proc_infos[mem_idx].pid == process_id) { // Basic cleanup for next exec_process
                    // This doesn't free the frames yet, but makes the slot available
                   // mem_proc_infos[mem_idx].pid = 0; 
                   // mem_proc_infos[mem_idx].num_pages_requested = 0;
                   // A more robust cleanup in memory.c would be needed to free frames.
                   // For now, if mem_init is called, it clears everything.
                }


                printf("--- Simulation Lifecycle for PID %d (%s) Ended ---\n", process_id, program_name_arg);
            } // end of arg_count check for exec_process
        } // end of exec_process command
        
        // ... other commands like rr, mem_init, etc. from previous main.c version
        else if (strcmp(command, "rr") == 0) {
            if (arg_count < 2 || args[0] == NULL) {
                printf("Usage: rr <time_quantum>\n");
            } else {
                int tq = atoi(args[0]);
                if (tq <= 0) tq = 4; 
                 Process processes_rr[] = {{1,0,10},{2,1,5},{3,2,8}}; 
                int num_rr = sizeof(processes_rr)/sizeof(Process);
                simulate_round_robin(processes_rr, num_rr, tq);
            }
        } else if (strcmp(command, "mem_init") == 0) {
            init_memory_management();
            current_mem_processes_count = 0;
            memory_initialized_flag = 1;
             for(int i=0; i<MAX_MEM_PROCESSES_MAIN; ++i) {
                mem_proc_infos[i].pid = 0;
                mem_proc_infos[i].num_pages_requested = 0;
            }
            printf("Memory management initialized.\n");
        } else if (strcmp(command, "mem_req") == 0) {
            if (!memory_initialized_flag) printf("Initialize memory first (mem_init).\n");
            else if (arg_count < 3 || args[0] == NULL || args[1] == NULL) printf("Usage: mem_req <pid> <num_pages>\n");
            else if (current_mem_processes_count >= MAX_MEM_PROCESSES_MAIN) printf("Max memory processes (%d) reached.\n", MAX_MEM_PROCESSES_MAIN);
            else {
                int pid = atoi(args[0]);
                int pages = atoi(args[1]);
                if (pid > 0) {
                    int existing_idx = -1;
                    for(int i=0; i < current_mem_processes_count; ++i) if(mem_proc_infos[i].pid == pid) existing_idx = i;
                    
                    if(existing_idx != -1) printf("PID %d already exists for memory tracking.\n", pid);
                    else {
                        int next_slot = -1; // Find truly empty slot or next available
                        for(int i=0; i < MAX_MEM_PROCESSES_MAIN; ++i) if(mem_proc_infos[i].pid == 0) {next_slot = i; break;}
                        if (next_slot == -1 && current_mem_processes_count < MAX_MEM_PROCESSES_MAIN) next_slot = current_mem_processes_count;

                        if (next_slot != -1) {
                            request_memory(&mem_proc_infos[next_slot], pid, pages);
                            if(mem_proc_infos[next_slot].num_pages_requested > 0 && mem_proc_infos[next_slot].pid == pid) {
                                if (next_slot >= current_mem_processes_count) current_mem_processes_count = next_slot + 1;
                            }
                        } else {
                             printf("Could not find slot for new PID %d in mem_proc_infos.\n", pid);
                        }
                    }
                } else printf("Invalid PID.\n");
            }
        } else if (strcmp(command, "mem_access") == 0) {
            if (!memory_initialized_flag) printf("Initialize memory first (mem_init).\n");
            else if (current_mem_processes_count == 0 && MAX_MEM_PROCESSES_MAIN > 0 && mem_proc_infos[0].pid == 0 ) printf("No processes requested memory yet (mem_req).\n"); // Check if any process is active
            else if (arg_count < 3 || args[0] == NULL || args[1] == NULL) printf("Usage: mem_access <pid> <page_num>\n");
            else {
                int pid = atoi(args[0]);
                int page = atoi(args[1]);
                int idx = -1;
                for(int i=0; i<current_mem_processes_count; ++i) if(mem_proc_infos[i].pid == pid) idx = i; // Search up to current_mem_processes_count
                if(idx != -1) access_memory(&mem_proc_infos[idx], pid, page);
                else printf("PID %d not found in active memory processes.\n", pid);
            }
        } else if (strcmp(command, "mem_status") == 0) {
            if(!memory_initialized_flag) printf("Initialize memory first (mem_init).\n");
            else display_memory_status(mem_proc_infos, current_mem_processes_count);
        } else if (strcmp(command, "fs_init") == 0) {
            init_filesystem();
            fs_initialized_flag = 1;
        } else if (strcmp(command, "fs_create") == 0) {
            if(!fs_initialized_flag) printf("Initialize filesystem first (fs_init).\n");
            else if(arg_count < 3 || args[0] == NULL || args[1] == NULL) printf("Usage: fs_create <name> <size>\n");
            else create_file_sim(args[0], atoi(args[1]));
        } else if (strcmp(command, "fs_delete") == 0) {
            if(!fs_initialized_flag) printf("Initialize filesystem first (fs_init).\n");
            else if(arg_count < 2 || args[0] == NULL) printf("Usage: fs_delete <name>\n");
            else delete_file_sim(args[0]);
        } else if (strcmp(command, "fs_list") == 0) {
            if(!fs_initialized_flag) printf("Initialize filesystem first (fs_init).\n");
            else list_files_sim();
        } else if (strcmp(command, "disk_fcfs") == 0) {
            if (arg_count < 4) { 
                printf("Usage: disk_fcfs <head_pos> <total_cylinders> <req1> [req2 ...]\n");
            } else {
                int head = atoi(args[0]);
                int cylinders = atoi(args[1]);
                int requests[MAX_DISK_REQUESTS];
                int num_reqs = 0;
                for (int i = 2; i < arg_count -1 && num_reqs < MAX_DISK_REQUESTS; ++i) { 
                     if(args[i] != NULL) {
                        requests[num_reqs++] = atoi(args[i]);
                     }
                }
                simulate_fcfs_disk_scheduling(requests, num_reqs, head, cylinders);
            }
        }
        // End of other commands
        else {
            printf("Unknown command: '%s'. Type 'help' for available commands.\n", command);
        }
        free_parsed_command(command, args, arg_count); 
        command = NULL; 
    } // end while(1)
    return 0;
}
