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

    if (strcmp(buffer, "echo $?") == 0) {
        printf("%d\n", *exit_code);
        return;
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
        if (arg) {
            printf("%s\n", arg);
        }
        *exit_code = 0;
        return;
    }
    else if (strcmp(cmd, "exit") == 0) {
        char* arg = strtok(NULL, " ");
        if (arg) {
            int code = atoi(arg);
            *exit_code = code & 0xFF;
        }
        printf("bye\n");
        exit(*exit_code);
    }
    else {
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
                // We'll handle job storage and printing in the next commit
                printf("Running as background job (pid=%d)\n", pid);
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
