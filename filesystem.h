#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#define MAX_FILES 20
#define MAX_FILENAME_LEN 50

typedef struct {
    char name[MAX_FILENAME_LEN];
    int size; // e.g., in blocks or KB
    int allocated; // 1 if exists, 0 if deleted/free slot
} File;

void init_filesystem();
void create_file_sim(const char* filename, int size);
void delete_file_sim(const char* filename);
void list_files_sim();

#endif // FILESYSTEM_H
