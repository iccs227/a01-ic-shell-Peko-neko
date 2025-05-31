#include "commands.h"
#include "jobs.h"
#include "signals.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>

#define MAX_CMD_BUFFER 255

void handle_command(char* buffer, char* last_command, int* exit_code) {
    buffer[strcspn(buffer, "\n")] = 0;

    if (strlen(buffer) == 0) return;

    if (strcmp(buffer, "!!") == 0) {
        if (strlen(last_command) == 0) return;
        printf("%s\n", last_command);
        strcpy(buffer, last_command);
    } else {
        strcpy(last_command, buffer);
    }

    // rest of the function can be refactored in real usage
    printf("Command handler not fully implemented in this stub.\n");
}
