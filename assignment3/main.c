#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#include "smallsh.h"
#include "dynarray.h"

int main(int argc, char **argv) {
    // TODO: Detect SIGINT
    // TODO: Detect SIGTSTP

    char input[2048];
    struct command cmd_buf;
    init_command(&cmd_buf);
    int last_status = 0;
    bool is_foreground_only = false;

    while (1) {
        printf(": ");
        fflush(stdout);
        fgets(input, 2048, stdin);
        // Get rid of newline character
        input[strcspn(input, "\n")] = '\0';

        int ran_builtin = check_builtin_commands(input, last_status);
        if (ran_builtin >= 0) {
            // Ran one of the builtin commands
            if (ran_builtin == 1) {
                // Ran exit command
                // TODO: kill bg processes
                return EXIT_SUCCESS;
            }
            // go back to prompt
            continue;
        }

        parse_command(&cmd_buf, input, is_foreground_only);
        //print_command(&cmd_buf);
        int exit_status = execute_command(&cmd_buf);
        clear_command(&cmd_buf);
        
        // Update exit status if necessary
        if (exit_status != -1) {
            last_status = exit_status;
        }
    }

    return EXIT_SUCCESS;
}