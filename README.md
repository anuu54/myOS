My OS Simulaton 
I built this project to demystify how Operating Systems work under the hood. While textbooks explain the theory, I wanted to see how is it actually working turn by coding the core logic in C.

Core Functionality
Round Robin Scheduler: Manages process execution via a circular queue to ensure fair CPU time distribution.
Memory Manager: Simulates RAM allocation and tracking to prevent overlaps and leaks.
Virtual File System: Handles file metadata and persistent storage on a simulated disk.
Modular Architecture: Each component is isolated into its own .c and .h files for clean, defensive programming.

Project Structure
scheduler.c – simulates CPU context switching and queue management.
memory.c – simulates RAM allocation and block tracking.
disk.c - Simulates memory switching concepts.
filesystem.c –simulates file metadata and storage logic (virtual).
main.c – The test bench that runs the full simulation.
