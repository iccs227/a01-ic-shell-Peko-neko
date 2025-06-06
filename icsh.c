/*  ICCS227: Project 1:icsh
*   Name: Phiraphat Wattanaprasit
*   StudentID: 6481333
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAX_CMD_BUFFER 255
#define MAX_JOBS 100

typedef struct {
    int job_id;
    pid_t pid;
    char command[MAX_CMD_BUFFER];
    bool running;
} Job;

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

void handle_command(char* buffer, char* last_command, int* exit_code) {
    buffer[strcspn(buffer, "\n")] = 0;

    if (strlen(buffer) == 0) return;

    if (strcmp(buffer, "!!") == 0) {
        if (strlen(last_command) == 0) {
            return;
        }
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
        for (int i = 0; i < job_count; i++) {
            if (job_list[i].pid > 0) {
                printf("[%d]%c  %s\t\t%s &\n",
                    job_list[i].job_id,
                    (i == job_count - 1) ? '+' : '-',
                    job_list[i].running ? "Running" : "Stopped",
                    job_list[i].command);
            }
        }
        *exit_code = 0;
        return;
    }

    if (strcmp(cmd, "fg") == 0) {
        char* arg = strtok(NULL, " ");
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

        return;
    }

    if (strcmp(cmd, "bg") == 0) {
        char* arg = strtok(NULL, " ");
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
        return;
    }

    if (strcmp(cmd, "exit") == 0) {
        char* arg = strtok(NULL, " ");
        if (arg) {
            int code = atoi(arg);
            *exit_code = code & 0xFF;
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
            waitpid(pid, &status, 0);
            current_pid = -1;
            if (WIFEXITED(status)) {
                *exit_code = WEXITSTATUS(status);
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

int main(int argc, char* argv[]) {
    char buffer[MAX_CMD_BUFFER];
    char last_command[MAX_CMD_BUFFER] = "";
    int exit_code = 0;

    FILE* input = stdin;

    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    signal(SIGCHLD, sigchld_handler);

    if (argc > 1) {
        input = fopen(argv[1], "r");
        if (!input) {
            perror("Error opening script file");
            return 1;
        }
    }
    else {
        printf("Starting IC shell\n");
    }

    while (1) {
        if (input == stdin) {
            printf("icsh $ ");
        }

        if (!fgets(buffer, MAX_CMD_BUFFER, input)) {
            if (input != stdin) fclose(input);
            printf("bye\n");
            break;
        }

        handle_command(buffer, last_command, &exit_code);
    }

    return exit_code;
}
