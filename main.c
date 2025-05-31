/*  ICCS227: Project 1:icsh
*   Name: Phiraphat Wattanaprasit
*   StudentID: 6481333
*/

#include <stdio.h>
#include <string.h>
#include "icsh.h"
#include "signals.h"
#include "jobs.h"

void handle_command(char* buffer, char* last_command, int* exit_code); // from commands.c

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
    } else {
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
