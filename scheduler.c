#include <stdio.h>
#include <stdlib.h>
#include "scheduler.h"

void simulate_round_robin(Process processes[], int n, int time_quantum) {
    printf("\n--  Round Robin Scheduling Simulation --\n");
    printf("Time Quantum: %d\n", time_quantum);
    printf("PID\tArrival\tBurst\tCompletion\tTurnaround\tWaiting\n");

    int current_time = 0;
    int completed_processes = 0;
    int queue[MAX_PROCESSES];
    int front = -1, rear = -1;
    int i;

    for (i = 0; i < n; i++) {
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].in_queue = 0; // Process not in queue initially
        processes[i].completion_time = 0;
        processes[i].turnaround_time = 0;
        processes[i].waiting_time = 0;
    }

    int processes_added_to_initial_queue = 0;
    while(completed_processes < n) {
        // Add newly arrived processes to the queue
        for (i = 0; i < n; i++) {
            if (processes[i].arrival_time <= current_time && processes[i].remaining_time > 0 && !processes[i].in_queue) {
                if (rear == MAX_PROCESSES - 1 && front == 0 || rear + 1 == front) { // Queue full check for circular array
                    // This shouldn't happen if MAX_PROCESSES is sufficient
                    printf("Error: Ready queue is full!\n");
                } else {
                    if (front == -1) front = 0; // First element
                    rear = (rear + 1) % MAX_PROCESSES;
                    queue[rear] = i;
                    processes[i].in_queue = 1;
                    processes_added_to_initial_queue = 1; // Flag that at least one process made it to queue
                }
            }
        }

        if (front == -1) { // If queue is empty
            if(processes_added_to_initial_queue == 0 && completed_processes < n){
                // If no process has even made it to the queue yet, but there are processes to run, advance time
                current_time++;
                continue;
            } else if (completed_processes < n) {
                // If queue became empty but not all processes are done, it means there's a gap
                // Find the next arrival time and jump if necessary (or just increment)
                int min_next_arrival = -1;
                for(i=0; i<n; ++i) {
                    if(processes[i].remaining_time > 0) {
                        if(min_next_arrival == -1 || processes[i].arrival_time < min_next_arrival) {
                            min_next_arrival = processes[i].arrival_time;
                        }
                    }
                }
                if(min_next_arrival != -1 && min_next_arrival > current_time) {
                    printf("CPU Idle. Advancing time from %d to %d\n", current_time, min_next_arrival);
                    current_time = min_next_arrival;
                } else {
                     current_time++; // Default advance if no specific next arrival found or it's in past/present
                }
                continue;
            } else {
                break; // All processes done
            }
        }

        int current_process_idx = queue[front];
        
        if (front == rear) front = rear = -1; // Queue becomes empty
        else front = (front + 1) % MAX_PROCESSES;
        
        processes[current_process_idx].in_queue = 0; // Mark as dequeued for execution

        printf("Time %d: Executing Process PID %d (Burst left: %d)\n", current_time, processes[current_process_idx].pid, processes[current_process_idx].remaining_time);

        if (processes[current_process_idx].remaining_time <= time_quantum) {
            current_time += processes[current_process_idx].remaining_time;
            processes[current_process_idx].remaining_time = 0;
            processes[current_process_idx].completion_time = current_time;
            processes[current_process_idx].turnaround_time = processes[current_process_idx].completion_time - processes[current_process_idx].arrival_time;
            processes[current_process_idx].waiting_time = processes[current_process_idx].turnaround_time - processes[current_process_idx].burst_time;
            completed_processes++;

            printf("Time %d: Process PID %d FINISHED. CT=%d, TAT=%d, WT=%d\n",
                   current_time,
                   processes[current_process_idx].pid,
                   processes[current_process_idx].completion_time,
                   processes[current_process_idx].turnaround_time,
                   processes[current_process_idx].waiting_time);
        } else {
            current_time += time_quantum;
            processes[current_process_idx].remaining_time -= time_quantum;
            printf("Time %d: Process PID %d ran for quantum. Remaining: %d\n", current_time, processes[current_process_idx].pid, processes[current_process_idx].remaining_time);
            
            // Add processes that might have arrived during this quantum's execution before adding current one back
             for (i = 0; i < n; i++) {
                if (processes[i].arrival_time <= current_time && processes[i].remaining_time > 0 && !processes[i].in_queue && i != current_process_idx) {
                     if (rear == MAX_PROCESSES - 1 && front == 0 || rear + 1 == front) { /* Queue full */ }
                     else {
                        if (front == -1) front = 0;
                        rear = (rear + 1) % MAX_PROCESSES;
                        queue[rear] = i;
                        processes[i].in_queue = 1;
                    }
                }
            }
            // Add current process back to queue if not finished
            if (processes[current_process_idx].remaining_time > 0) {
                 if (rear == MAX_PROCESSES - 1 && front == 0 || rear + 1 == front) { /* Queue full */ }
                 else {
                    if (front == -1) front = 0;
                    rear = (rear + 1) % MAX_PROCESSES;
                    queue[rear] = current_process_idx;
                    processes[current_process_idx].in_queue = 1;
                }
            }
        }
    }

    printf("\nFinal Process States:\n");
    printf("PID\tArrival\tBurst\tCompletion\tTurnaround\tWaiting\n");
    float avg_turnaround_time = 0, avg_waiting_time = 0;
    for (i = 0; i < n; i++) {
        printf("%d\t%d\t%d\t%d\t\t%d\t\t%d\n",
               processes[i].pid, processes[i].arrival_time, processes[i].burst_time,
               processes[i].completion_time, processes[i].turnaround_time, processes[i].waiting_time);
        avg_turnaround_time += processes[i].turnaround_time;
        avg_waiting_time += processes[i].waiting_time;
    }
    if (n > 0) {
        avg_turnaround_time /= n;
        avg_waiting_time /= n;
    }
    printf("\nAverage Turnaround Time: %.2f\n", avg_turnaround_time);
    printf("Average Waiting Time: %.2f\n", avg_waiting_time);
}
