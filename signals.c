#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include "icsh.h"

void sigint_handler(int sig) {
    if (current_pid > 0) {
        kill(current_pid, SIGINT);
    }
}

void sigtstp_handler(int sig) {
    if (current_pid > 0) {
        kill(current_pid, SIGTSTP);
    }
}

void sigchld_handler(int sig) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < job_count; i++) {
            if (job_list[i].pid == pid) {
                job_list[i].running = false;
                printf("\n[%d]+  Done                    %s\n", job_list[i].job_id, job_list[i].command);
                fflush(stdout);
                break;
            }
        }
    }
}
