#include <stdio.h>
#include <stdlib.h> // For abs()
#include "disk.h"

void simulate_fcfs_disk_scheduling(int requests[], int num_requests, int initial_head_pos, int total_cylinders) {
    printf("\n## FCFS Disk Scheduling Simulation ##\n");
    if (num_requests <= 0) {
        printf("No disk requests to process.\n");
        return;
    }
    if (initial_head_pos < 0 || initial_head_pos >= total_cylinders) {
        printf("Invalid initial head position %d. Must be between 0 and %d.\n", initial_head_pos, total_cylinders -1);
        return;
    }
     if (total_cylinders <= 0) {
        printf("Invalid total cylinders %d. Must be positive.\n", total_cylinders);
        return;
    }
    
    printf("Total Cylinders: 0 to %d\n", total_cylinders - 1);
    printf("Initial Head Position: %d\n", initial_head_pos);
    printf("Request Queue: ");
    for(int i=0; i < num_requests; i++) {
        printf("%d ", requests[i]);
    }
    printf("\n");

    int total_head_movement = 0;
    int current_head_pos = initial_head_pos;

    printf("Head Movement Sequence: %d", current_head_pos);
    for (int i = 0; i < num_requests; i++) {
        if (requests[i] < 0 || requests[i] >= total_cylinders) {
            printf("\nSkipping invalid request: %d (out of bounds 0-%d). ", requests[i], total_cylinders-1);
            continue;
        }
        total_head_movement += abs(requests[i] - current_head_pos);
        current_head_pos = requests[i];
        printf(" -> %d", current_head_pos);
    }
    printf("\nTotal Head Movement: %d cylinders.\n", total_head_movement);
}
