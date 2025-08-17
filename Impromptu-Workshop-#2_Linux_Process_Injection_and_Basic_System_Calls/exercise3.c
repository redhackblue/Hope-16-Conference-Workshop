#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <signal.h> // For raise()
#include <errno.h>

int main() {
    // Intro message
    printf("Exercise 3: Attaching to a Process with ptrace\n");
    sleep(1); // Give students time to read

    // Fork a new child process
    pid_t pid = fork();
    if (pid == -1) { // Check for fork failure
        perror("fork failed");
        return 1;
    }

    if (pid == 0) { // CHILD PROCESS
        // Immediately stop itself so the parent can attach
        raise(SIGSTOP);

        // After being attached and detached by the parent, continue execution
        printf("[Child] PID = %d, starting 5-second loop...\n", getpid());

        // Loop printing PID and PPID every second
        for (int i = 0; i < 5; i++) {
            printf("[Child] Alive: PID = %d, PPID = %d\n", getpid(), getppid());
            sleep(1);
        }

        printf("[Child] Exiting.\n"); // Child finishes
        exit(0);
    } else { // PARENT PROCESS
        // Small pause to ensure child has started and is stopped
        usleep(100000); // 0.1 seconds

        printf("[Parent] Attaching to child PID %d...\n", pid);

        // Attach to the child using ptrace
        if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1) {
            perror("ptrace attach failed");
            return 1;
        }

        // Wait for child to stop after SIGSTOP
        waitpid(pid, NULL, 0);

        printf("[Parent] Attached! Sleeping 2 seconds before detaching...\n");
        sleep(2); // Pause so students can observe the attached state

        // Detach from the child and continue its execution
        if (ptrace(PTRACE_DETACH, pid, NULL, 0) == -1) { // The 0 ensures child resumes
            perror("ptrace detach failed");
            return 1;
        }

        printf("[Parent] Detached from child.\n");

        // Wait for the child to finish its loop and exit
        int status;
        waitpid(pid, &status, 0); // Wait for actual child termination
        printf("[Parent] Child has exited.\n");
    }

    return 0; // Program ends
}

