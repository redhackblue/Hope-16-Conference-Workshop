#include <stdio.h>      // Standard I/O for printf
#include <stdlib.h>     // Standard library for exit
#include <unistd.h>     // For fork, sleep, getpid
#include <sys/types.h>  // For pid_t type
#include <sys/wait.h>   // For wait function

int main() {
    printf("Exercise 1: Forking a Child Process\n"); // Intro message
    sleep(7); // Pause to give time to read

    pid_t pid = fork(); // Create child process
    if (pid == -1) { // Error check
        perror("fork failed"); // Print error
        return 1; // Exit with failure
    }

    if (pid == 0) { // Child branch
        printf("[Child] Started. PID = %d, PPID = %d\n", getpid(), getppid()); // Show child info
        
        // Loop to show child PID and PPID every second
        for (int i = 0; i < 7; i++) {
            printf("[Child] Alive: PID = %d, PPID = %d\n", getpid(), getppid());
            sleep(1);
        }
        
        printf("[Child] Doing some work...\n"); // Simulated work
        sleep(7); // More delay
        
        printf("[Child] Exiting now.\n"); // Exit message
        exit(0); // End child
        
    } else { // Parent branch
        printf("[Parent] Created child with PID = %d\n", pid); // Report child PID
        
        // Loop to show parent PID and child PID every second while waiting
        for (int i = 0; i < 7; i++) {
            printf("[Parent] Alive: PID = %d, Child PID = %d\n", getpid(), pid);
            sleep(1);
        }
        
        
        printf("[Parent] Waiting for child to finish...\n"); // Waiting message
        wait(NULL); // Wait for child
        
        printf("[Parent] Child has exited.\n"); // Confirmation
    }
    return 0; // End program
}

