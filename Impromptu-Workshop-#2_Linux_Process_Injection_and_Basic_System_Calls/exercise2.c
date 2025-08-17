#include <stdio.h>      // For printf
#include <stdlib.h>     // For exit
#include <unistd.h>     // For fork, execl
#include <sys/types.h>  // For pid_t
#include <sys/wait.h>   // For wait

int main() {
    printf("Exercise 2: Executing a Program in Child\n"); // Intro message
    sleep(1); // Pause

    pid_t pid = fork(); // Fork new process
    if (pid == -1) { // Error check
        perror("fork failed"); // Print error
        return 1; // Exit failure
    }

    if (pid == 0) { // Child
        printf("[Child] PID = %d, going to exec /bin/ls in 2 seconds...\n", getpid()); // Info
        sleep(2); // Delay to see message
        execl("/bin/ls", "ls", NULL); // Replace process with ls
        perror("execl failed"); // Only runs if exec fails
        exit(1); // End child
    } else { // Parent
        printf("[Parent] Waiting for child to exec and finish...\n"); // Info
        wait(NULL); // Wait for child
        printf("[Parent] Child execution complete.\n"); // Confirmation
    }
    return 0; // End program
}

