#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_CMD_BUFFER 255

int main() {
    char buffer[MAX_CMD_BUFFER];
    char last_command[MAX_CMD_BUFFER] = "";  // stores last command
    int exit_code = 0;

    FILE* input = stdin;

    // Script mode: open file instead of stdin
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
        printf("icsh $ ");
        if (!fgets(buffer, MAX_CMD_BUFFER, stdin)) {
            // EOF (e.g., Ctrl+D)
            printf("\nbye\n");
            break;
        }

        // Remove trailing newline if present
        buffer[strcspn(buffer, "\n")] = 0;

        // Handle empty input
        if (strlen(buffer) == 0) {
            continue;
        }

        // Handle '!!' command
        if (strcmp(buffer, "!!") == 0) {
            if (strlen(last_command) == 0) {
                // No previous command
                continue;
            }
            printf("%s\n", last_command);
            strcpy(buffer, last_command);  // replace current buffer
        }
        else {
            strcpy(last_command, buffer);  // update last_command
        }

        // Tokenize the input to check the command
        char* cmd = strtok(buffer, " ");

        if (strcmp(cmd, "echo") == 0) {
            char* arg = strtok(NULL, "");  // Get rest of the line
            if (arg) {
                printf("%s\n", arg);
            }
            continue;
        }
        else if (strcmp(cmd, "exit") == 0) {
            char* arg = strtok(NULL, " ");
            if (arg) {
                int code = atoi(arg);
                exit_code = code & 0xFF;
            }
            printf("bye\n");
            break;
        }
        else {
            printf("bad command\n");
        }
    }

    return exit_code;
}
