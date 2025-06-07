#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "signals.h"
#include <sys/wait.h>
#include "jobs.h"

Job job_list[MAX_JOBS];
int job_count = 0;
pid_t current_pid = -1;

int find_job_by_id(int id) {
    for (int i = 0; i < job_count; i++) {
        if (job_list[i].job_id == id) {
            return i;
        }
    }
    return -1;
}

void print_jobs() {
    for (int i = 0; i < job_count; i++) {
        if (job_list[i].pid > 0) {
            printf("[%d]%c  %s\t\t%s &\n",
                   job_list[i].job_id,
                   (i == job_count - 1) ? '+' : '-',
                   job_list[i].running ? "Running" : "Stopped",
                   job_list[i].command);
        }
    }
}

void handle_fg(char* arg, int* exit_code) {
    if (!arg || arg[0] != '%') {
        printf("Usage: fg %%<job_id>\n");
        return;
    }

    int id = atoi(arg + 1);
    int idx = find_job_by_id(id);
    if (idx == -1) {
        printf("No such job\n");
        return;
    }

    printf("%s\n", job_list[idx].command);
    current_pid = job_list[idx].pid;
    kill(current_pid, SIGCONT);
    int status;
    waitpid(current_pid, &status, 0);
    current_pid = -1;
    job_list[idx].running = false;

    if (WIFEXITED(status)) {
        *exit_code = WEXITSTATUS(status);
    }
}

void handle_bg(char* arg, int* exit_code) {
    if (!arg || arg[0] != '%') {
        printf("Usage: bg %%<job_id>\n");
        return;
    }

    int id = atoi(arg + 1);
    int idx = find_job_by_id(id);
    if (idx == -1) {
        printf("No such job\n");
        return;
    }

    job_list[idx].running = true;
    kill(job_list[idx].pid, SIGCONT);
    printf("[%d]+ %s &\n", job_list[idx].job_id, job_list[idx].command);
    *exit_code = 0;
}
