#include <stdio.h>      // For printf
#include <stdlib.h>     // For exit
#include <unistd.h>     // For fork, sleep
#include <sys/types.h>  // For pid_t
#include <sys/wait.h>   // For waitpid
#include <sys/ptrace.h> // For ptrace
#include <signal.h>     // For raise(SIGSTOP)
#include <string.h>     // For memcpy
#include <errno.h>      // For error handling

// Global buffer in child process
static char buffer[32] = "Original data";

int main() {
    printf("Exercise 6: Simplified Process Injection using PTRACE_TRACEME\n");
    fflush(stdout);
    sleep(1); // short pause for readability

    pid_t pid = fork(); // Fork a new process
    if (pid == -1) {
        perror("fork failed");
        return 1;
    }

    if (pid == 0) {
        // -------- CHILD PROCESS --------
        ptrace(PTRACE_TRACEME, 0, NULL, NULL); // Allow parent to trace

        // Print child PID and buffer address
        printf("[Child] PID = %d, buffer at %p contains: '%s'\n",
               getpid(), (void*)buffer, buffer);
        fflush(stdout);
        
        //sleep(15);

        // Stop self to give parent or monitor time to attach
        raise(SIGSTOP);

        // Optional extra sleep to ensure you have enough time in another terminal
        //sleep(60);

        // After injection, print buffer content
        printf("[Child] Buffer after injection: '%s'\n", buffer);
        fflush(stdout);

        printf("[Child] Exiting.\n");
        fflush(stdout);
        exit(0);

    } else {
        // -------- PARENT PROCESS --------
        int status;

        // Wait for child to stop itself (SIGSTOP)
        waitpid(pid, &status, 0);

        printf("[Parent] Attached! Injecting 'Injected data' into child's buffer...\n");
        fflush(stdout);

        char injected_code[] = "Injected data";

        // Write injected data in sizeof(long) chunks
        for (size_t i = 0; i < sizeof(injected_code); i += sizeof(long)) {
            long data = 0;
            size_t chunk_size = sizeof(long) > (sizeof(injected_code) - i) ? 
                                (sizeof(injected_code) - i) : sizeof(long);
            memcpy(&data, injected_code + i, chunk_size);

            if (ptrace(PTRACE_POKEDATA, pid, (void*)((long)buffer + i), data) == -1) {
                perror("ptrace pokedata failed");
            } else {
                // Print each chunk written for live monitoring in this terminal
                printf("[Parent] Injected chunk at offset %zu: 0x%lx\n", i, data);
                fflush(stdout);
            }

           // usleep(7000000); // Optional small delay to make injection visible gradually
        }

        // Detach so child continues
        ptrace(PTRACE_DETACH, pid, NULL, 0);
        printf("[Parent] Injection complete and detached.\n");
        fflush(stdout);

        // Wait for child to finish
        waitpid(pid, &status, 0);
        printf("[Parent] Child has exited.\n");
        fflush(stdout);
    }

    return 0;
}

