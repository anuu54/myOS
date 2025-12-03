#ifndef SCHEDULER_H
#define SCHEDULER_H

#define MAX_PROCESSES 10

typedef struct {
    int pid;
    int arrival_time;
    int burst_time;
    int remaining_time;
    int completion_time;
    int turnaround_time;
    int waiting_time;
    int in_queue; // Flag to track if process is in ready queue
} Process;

void simulate_round_robin(Process processes[], int n, int time_quantum);

#endif // SCHEDULER_H
