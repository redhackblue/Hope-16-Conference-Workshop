#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <errno.h>

// A global buffer makes memory reading simple and reliable
char buffer[16] = "Hello, World!";

int main() {
    printf("Exercise 4: Reading Child Process Memory\n");
    sleep(1); // Give students time to read the intro

    pid_t pid = fork(); // Create a child process
    if (pid == -1) {   // Error check for fork
        perror("fork failed");
        return 1;
    }

    if (pid == 0) { // CHILD PROCESS
        // Print the PID and buffer info so parent knows the address
        printf("[Child] PID = %d, buffer at %p contains: '%s'\n", getpid(), (void*)buffer, buffer);

        // Stop itself so parent can attach safely
        raise(SIGSTOP);

        // Continue running after parent detaches
        printf("[Child] Resuming execution...\n");
        sleep(3); // Simulate some work
        printf("[Child] Exiting.\n");
        exit(0);
    } else { // PARENT PROCESS
        // Small pause to ensure child has started and stopped
        usleep(100000); // 0.1 seconds

        printf("[Parent] Attaching to child PID %d...\n", pid);

        // Attach to the child using ptrace
        if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
            perror("ptrace attach failed");
            return 1;
        }

        // Wait for child to stop after SIGSTOP
        waitpid(pid, NULL, 0);

        printf("[Parent] Attached! Attempting to read child buffer...\n");
        sleep(1); // Pause for demonstration

        errno = 0; // Reset errno before peeking
        // Read the first 8 bytes of the child buffer
        long data = ptrace(PTRACE_PEEKDATA, pid, buffer, NULL);
        if (errno != 0) {
            perror("ptrace peekdata failed");
        } else {
            printf("[Parent] Read data from child buffer (first 8 bytes): 0x%lx\n", data);
        }

        sleep(1); // Pause to let students observe

        // Detach from child and let it continue
        if (ptrace(PTRACE_DETACH, pid, NULL, 0) == -1) { // 0 ensures child resumes
            perror("ptrace detach failed");
            return 1;
        }

        printf("[Parent] Detached from child.\n");

        // Wait for child to finish
        int status;
        waitpid(pid, &status, 0);
        printf("[Parent] Child has exited.\n");
    }

    return 0; // Program ends
}

