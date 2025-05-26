#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_CMD_BUFFER 255

void handle_command(char* buffer, char* last_command, int* exit_code) {
    // Remove newline if present
    buffer[strcspn(buffer, "\n")] = 0;

    // Ignore empty line
    if (strlen(buffer) == 0) return;

    // Handle '!!'
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

    // Tokenize and process
    char* cmd = strtok(buffer, " ");
    if (!cmd) return;

    if (strcmp(cmd, "echo") == 0) {
        char* arg = strtok(NULL, "");
        if (arg) printf("%s\n", arg);
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
        printf("bad command\n");
    }
}

int main(int argc, char* argv[]) {
    char buffer[MAX_CMD_BUFFER];
    char last_command[MAX_CMD_BUFFER] = "";
    int exit_code = 0;

    FILE* input = stdin;

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
