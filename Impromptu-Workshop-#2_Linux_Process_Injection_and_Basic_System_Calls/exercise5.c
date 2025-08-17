#include <stdio.h>      // For printf
#include <stdlib.h>     // For exit
#include <unistd.h>     // For fork, sleep
#include <sys/types.h>  // For pid_t
#include <sys/wait.h>   // For waitpid
#include <sys/ptrace.h> // For ptrace functions
#include <signal.h>     // For raise(SIGSTOP)
#include <errno.h>      // For error checking

// Global buffer in child process
char buffer[16] = "Original";

int main() {
    // Print exercise intro
    printf("Exercise 5: Writing to Child Process Memory\n");
    fflush(stdout);  // Flush stdout to make sure message appears immediately
    sleep(1);        // Small pause for clarity

    // Fork the process
    pid_t pid = fork();
    if (pid == -1) { 
        perror("fork failed"); 
        return 1;
    }

    if (pid == 0) { 
        // -------- CHILD PROCESS --------
        // Print the original buffer and its address
        printf("[Child] PID = %d, buffer at %p contains: '%s'\n", getpid(), (void*)buffer, buffer);
        fflush(stdout);

        sleep(1);           // Give parent time to prepare
        raise(SIGSTOP);     // Stop child so parent can attach with ptrace

        // Child resumes automatically after PTRACE_DETACH
        printf("[Child] Buffer after modification: '%s'\n", buffer);
        fflush(stdout);

        printf("[Child] Exiting.\n");
        fflush(stdout);
        exit(0);

    } else { 
        // -------- PARENT PROCESS --------
        sleep(1); // Ensure child has printed its initial buffer

        printf("[Parent] Attaching to child PID %d...\n", pid);
        fflush(stdout);

        // Attach to child so we can modify its memory
        if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
            perror("ptrace attach failed - ensure ptrace_scope=0 or run as root");
            return 1;
        }

        // Wait for child to stop after SIGSTOP
        waitpid(pid, NULL, 0);

        printf("[Parent] Attached! Writing 'Modified' to child's buffer...\n");
        fflush(stdout);

        // Prepare new data to write
        char new_data[] = "Modified";

        // Write new_data into child's buffer in sizeof(long) chunks
        for (size_t i = 0; i < sizeof(new_data); i += sizeof(long)) {
            long data = 0;
            for (size_t j = 0; j < sizeof(long) && (i + j) < sizeof(new_data); j++) {
                ((char*)&data)[j] = new_data[i + j]; // Copy byte-by-byte into long
            }
            if (ptrace(PTRACE_POKEDATA, pid, buffer + i, data) == -1) {
                perror("ptrace pokedata failed");
            }
        }

        // Read back the full buffer from the child to verify modification
        char verified[sizeof(buffer)] = {0};
        for (size_t i = 0; i < sizeof(buffer); i += sizeof(long)) {
            errno = 0;
            long data = ptrace(PTRACE_PEEKDATA, pid, buffer + i, NULL);
            if (errno != 0) {
                perror("ptrace peekdata failed");
                break;
            }
            for (size_t j = 0; j < sizeof(long) && (i + j) < sizeof(buffer); j++) {
                verified[i + j] = ((char*)&data)[j]; // Reconstruct buffer
            }
        }

        // Print the verified child buffer
        printf("[Parent] Verified child buffer: '%s'\n", verified);
        fflush(stdout);

        // Detach from child; this automatically resumes the child
        if (ptrace(PTRACE_DETACH, pid, NULL, 0) == -1) {
            perror("ptrace detach failed");
            return 1;
        }

        printf("[Parent] Detached from child.\n");
        fflush(stdout);

        // Wait for child to finish execution
        int status;
        waitpid(pid, &status, 0);
        printf("[Parent] Child has exited.\n");
        fflush(stdout);
    }

    return 0;
}

