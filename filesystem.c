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
            strcpy(file_system[i].name, filename);
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
            //strcpy(file_system[i].name, ""); // Optional: clear name
            //file_system[i].size = 0;       // Optional: clear size
            num_active_files--;
            printf("File '%s' deleted.\n", filename);
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
