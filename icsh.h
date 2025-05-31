#ifndef ICSH_H
#define ICSH_H

#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>


#define MAX_CMD_BUFFER 255
#define MAX_JOBS 100

typedef struct {
    int job_id;
    pid_t pid;
    char command[MAX_CMD_BUFFER];
    bool running;
} Job;

extern pid_t current_pid;
extern Job job_list[MAX_JOBS];
extern int job_count;

#endif
