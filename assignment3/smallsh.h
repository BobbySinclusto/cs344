#ifndef SMALLSH_H
#define SMALLSH_H

#include <stdbool.h>

// struct to hold command information once it has been parsed
struct command {
    int argc;
    char *argv[512];
    char *input_filename;
    char *output_filename;
    bool background;
};

// Function to initialize command struct
void init_command(struct command *cmd_buf);

// Function to perform expansion in the command string. Returns a malloc'd string that should be freed
char* expand_cmd(char *input);

// Function to create command struct from user input string
void parse_command(struct command *cmd_buf, char *input, bool is_foreground_only);

// Function to execute command, returns exit status if it is a foreground process, -1 if background
int execute_command(struct command *cmd_buf);

// reset the command buffer
void clear_command(struct command *cmd_buf);

// Checks for builtin commands and comments/blank lines. Returns a number
// corresponding to the command that was run
int check_builtin_commands(char *input, int last_status);

// A utility function for debugging
void print_command(struct command *cmd_buf);

#endif