/**
 * Simple Virtual File System Simulation
 * Purpose: To learn how kernels manage file metadata and allocation 
 * without relying on a real physical disk.
 */
#include <stdio.h>
#include <string.h>
#include "filesystem.h"

File file_system[MAX_FILES];
int num_active_files = 0;

void init_filesystem() {
    printf("\n-- Basic File System Simulation ##\n");
    for (int i = 0; i < MAX_FILES; i++) {
        file_system[i].allocated = 0;
        strcpy(file_system[i].name, "");
        file_system[i].size = 0;
    }
    num_active_files = 0;
    printf("File system initialized. Max files: %d\n", MAX_FILES);
}

void create_file_sim(const char* filename, int size) {
    if (strlen(filename) >= MAX_FILENAME_LEN) {
        printf("Filename '%s' is too long. Max length is %d.\n", filename, MAX_FILENAME_LEN -1);
        return;
    }
    if (num_active_files >= MAX_FILES) {
        printf("File system full. Cannot create '%s'.\n", filename);
        return;
    }
    // Check if file already exists
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_system[i].allocated && strcmp(file_system[i].name, filename) == 0) {
            printf("File '%s' already exists.\n", filename);
            return;
        }
    }

    for (int i = 0; i < MAX_FILES; i++) {
        if (!file_system[i].allocated) {
            //strcpy(file_system[i].name, filename);
            // Using strncpy prevents buffer overflows
            strncpy(file_system[i].name, filename, MAX_FILENAME_LEN - 1);
            file_system[i].name[MAX_FILENAME_LEN - 1] = '\0'; // Manual null-termination for safety
            file_system[i].size = size;
            file_system[i].allocated = 1;
            num_active_files++;
            printf("File '%s' (size %d) created.\n", filename, size);
            return;
        }
    }
}

void delete_file_sim(const char* filename) {
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_system[i].allocated && strcmp(file_system[i].name, filename) == 0) {
            file_system[i].allocated = 0;
            memset(file_system[i].name, 0, MAX_FILENAME_LEN); // Wipe the name
            file_system[i].size = 0;                          // Reset the size
            num_active_files--;
            printf("File '%s' deleted successfully.\n", filename);
        return;
}
        
    }
    printf("File '%s' not found for deletion.\n", filename);
}

void list_files_sim() {
    printf("\n--- Files in System (%d active) ---\n", num_active_files);
    int found = 0;
    for (int i = 0; i < MAX_FILES; i++) {
        if (file_system[i].allocated) {
            printf("- Name: %s, Size: %d\n", file_system[i].name, file_system[i].size);
            found = 1;
        }
    }
    if (!found) {
        printf("No files in the system.\n");
    }
}
