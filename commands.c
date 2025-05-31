#include "commands.h"
#include "jobs.h"
#include "signals.h"
#include "icsh.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>

void handle_command(char* buffer, char* last_command, int* exit_code) {
    buffer[strcspn(buffer, "\n")] = 0;

    if (strlen(buffer) == 0) return;

    if (strcmp(buffer, "!!") == 0) {
        if (strlen(last_command) == 0) return;
        printf("%s\n", last_command);
        strcpy(buffer, last_command);
    }
    else {
        strcpy(last_command, buffer);
    }

    bool is_background = false;
    char* ampersand = strrchr(buffer, '&');
    if (ampersand && *(ampersand + 1) == '\0') {
        is_background = true;
        *ampersand = '\0';
        while (ampersand > buffer && *(ampersand - 1) == ' ') {
            *(--ampersand) = '\0';
        }
    }

    char* input_file = NULL;
    char* output_file = NULL;

    char* redirect_in = strchr(buffer, '<');
    char* redirect_out = strchr(buffer, '>');

    if (redirect_in) {
        *redirect_in = '\0';
        input_file = strtok(redirect_in + 1, " ");
    }
    if (redirect_out) {
        *redirect_out = '\0';
        output_file = strtok(redirect_out + 1, " ");
    }

    char* cmd = strtok(buffer, " ");
    if (!cmd) return;

    if (strcmp(cmd, "echo") == 0) {
        char* arg = strtok(NULL, "");
        if (arg && strcmp(arg, "$?") == 0) {
            printf("%d\n", *exit_code);
        }
        else if (arg) {
            printf("%s\n", arg);
        }
        *exit_code = 0;
        return;
    }

    if (strcmp(cmd, "jobs") == 0) {
        print_jobs();
        *exit_code = 0;
        return;
    }

    if (strcmp(cmd, "fg") == 0) {
        char* arg = strtok(NULL, " ");
        handle_fg(arg, exit_code);
        return;
    }

    if (strcmp(cmd, "bg") == 0) {
        char* arg = strtok(NULL, " ");
        handle_bg(arg, exit_code);
        return;
    }

    if (strcmp(cmd, "exit") == 0) {
        char* arg = strtok(NULL, " ");
        if (arg) {
            *exit_code = atoi(arg) & 0xFF;
        }
        printf("bye\n");
        exit(*exit_code);
    }

    char* args[64];
    int i = 0;
    args[i++] = cmd;
    char* token;
    while ((token = strtok(NULL, " ")) != NULL && i < 63) {
        args[i++] = token;
    }
    args[i] = NULL;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
    }
    else if (pid == 0) {
        if (input_file) {
            int fd = open(input_file, O_RDONLY);
            if (fd < 0) {
                perror("input redirection failed");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }

        if (output_file) {
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) {
                perror("output redirection failed");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        execvp(args[0], args);
        perror("exec failed");
        exit(1);
    }
    else {
        if (!is_background) {
            current_pid = pid;
            int status;
            waitpid(pid, &status, WUNTRACED);  // Wait even if stopped
            current_pid = -1;

            if (WIFEXITED(status)) {
                *exit_code = WEXITSTATUS(status);
            }
            else if (WIFSTOPPED(status)) {
                if (job_count < MAX_JOBS) {
                    job_list[job_count].job_id = job_count + 1;
                    job_list[job_count].pid = pid;
                    job_list[job_count].running = false;
                    strncpy(job_list[job_count].command, last_command, MAX_CMD_BUFFER - 1);
                    job_list[job_count].command[MAX_CMD_BUFFER - 1] = '\0';
                    printf("\n[%d]+  Stopped                 %s\n", job_list[job_count].job_id, job_list[job_count].command);
                    job_count++;
                }
            }
        }
        else {
            if (job_count < MAX_JOBS) {
                job_list[job_count].job_id = job_count + 1;
                job_list[job_count].pid = pid;
                job_list[job_count].running = true;
                strncpy(job_list[job_count].command, last_command, MAX_CMD_BUFFER - 1);
                job_list[job_count].command[MAX_CMD_BUFFER - 1] = '\0';
                printf("[%d] %d\n", job_list[job_count].job_id, pid);
                job_count++;
            }
            else {
                fprintf(stderr, "Job table full\n");
            }
        }
    }
}
