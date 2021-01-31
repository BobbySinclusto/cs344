#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

// struct to hold command information once it has been parsed
struct command {
    int argc;
    char *argv[512];
    char *input_filename;
    char *output_filename;
    bool background;
};

void init_command(struct command *cmd_buf) {
    cmd_buf->argc = 0;
    cmd_buf->argv[0] = NULL;
    cmd_buf->input_filename = NULL;
    cmd_buf->output_filename = NULL;
    cmd_buf->background = false;
}

// Function to perform expansion in the command string. Returns a malloc'd string that should be freed
char* expand_cmd(char *input) {
    // Count number of expansion operators in command
    int exp_count = 0;
    for (int i = 0; i < strlen(input) - 1; ++i) {
        if (input[i] == '$' && input[i+1] == '$') {
            exp_count += 1;
            // Skip next character
            i += 1;
        }
    }

    // buffer to store pid string, pid can never be longer than 8 characters
    char pid_string[8];

    // Store pid in pid_string
    sprintf(pid_string, "%d", getpid());
    int pid_length = strlen(pid_string);

    // Allocate memory for new command string
    char *new_command = malloc(sizeof(char) * (strlen(input) - (exp_count * 2) + pid_length * exp_count + 1));

    // Perform expansion if necessary
    int j = 0;
    for (int i = 0; i < strlen(input); ++i) {
        if (input[i] == '$' && input[i+1] == '$') {
            strncpy(new_command + j, pid_string, pid_length);
            j += pid_length;
            // Skip next character
            i += 1;
        }
        else {
            new_command[j] = input[i];
            j += 1;
        }
    }
    new_command[j] = '\0';
    return new_command;
}

// Function to create command struct from user input string
void parse_command(struct command *cmd_buf, char *input, bool is_foreground_only) {
    // Perform expansion if necessary
    char *cmd_string = expand_cmd(input);

    // Keep track of where the arguments stop and redirection/background operations start
    int last_index = strlen(cmd_string);

    // Check if this should run in background
    if (cmd_string[strlen(cmd_string) - 2] == ' ' && cmd_string[strlen(cmd_string) - 1] == '&') {
        cmd_buf->background = true;
        last_index -= 2;

        // Set input_filename and output_filename to default to /dev/null
        cmd_buf->input_filename = "/dev/null";
        cmd_buf->output_filename = "/dev/null";
    }

    // Check for redirection operators
    // input redirection
    int input_space_offset = 0;
    char *input_redir = strstr(cmd_string, " < ");
    if (input_redir != NULL) {
        // Update last_index if necessary
        if ((int)(input_redir - cmd_string) < last_index) {
            last_index = (int)(input_redir - cmd_string);
        }
        // Move past the redirection operator to the filename
        input_redir += 3;
        // We'll replace next space character with null byte after we're done checking the rest of the string
        input_space_offset = strcspn(input_redir, " ");
        cmd_buf->input_filename = input_redir;
    }

    // Output redirection
    int output_space_offset = 0;
    char *output_redir = strstr(cmd_string, " > ");
    if (output_redir != NULL) {
        // Update last_index if necessary
        if ((int)(output_redir - cmd_string) < last_index) {
            last_index = (int)(output_redir - cmd_string);
        }
        // Move past the redirection operator to the filename
        output_redir += 3;
        // We'll replace next space character with null byte after we're done checking the rest of the string
        output_space_offset = strcspn(output_redir, " ");
        cmd_buf->output_filename = output_redir;
    }

    // Actually do the replacing now that we're done checking the rest of the string
    if (input_redir != NULL) input_redir[input_space_offset] = '\0';
    if (output_redir != NULL) output_redir[output_space_offset] = '\0';

    // Split command into arguments
    int argnum = 0;
    int prev_index = 0;
    for (int i = 0; i <= last_index; ++i) {
        if (cmd_string[i] == ' ' || cmd_string[i] == '\0') {
            cmd_string[i] = '\0';
            cmd_buf->argv[argnum] = cmd_string + prev_index;
            prev_index = i + 1;
            argnum += 1;
        }
    }
    // Set last argument to NULL
    cmd_buf->argv[argnum] = NULL;
    cmd_buf->argc = argnum;
}

// Function to execute command, returns exit status if it is a foreground process, -1 if background
int execute_command(struct command *cmd_buf) {
    // Open files if necessary
    // TODO: record these in the background processes array
    int input_fd = 0;
    if (cmd_buf->input_filename != NULL) {
        input_fd = open(cmd_buf->input_filename, O_RDONLY);
        if (input_fd == -1) {
            printf("cannot open %s for input\n", cmd_buf->input_filename);
            return 1;
        }
    }

    int output_fd = 0;
    if (cmd_buf->output_filename != NULL) {
        output_fd = open(cmd_buf->output_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_fd == -1) {
            printf("cannot open %s for output\n", cmd_buf->output_filename);
            return 1;
        }
    }
    // Time to make a forkbomb
    pid_t child_pid = fork();

    if (child_pid == -1) {
        printf("Fork failed\n");
        return EXIT_FAILURE;
    }
    if (child_pid == 0) {
        // This is the child process, execute the command
        // Redirect i/o as necessary
        if (cmd_buf->input_filename != NULL) {
            if (dup2(input_fd, 0) == -1) {
                printf("Unable to get file descriptor for %s\n", cmd_buf->input_filename);
                exit(1);
            }
            close(input_fd);
        }
        if (cmd_buf->output_filename != NULL) {
            if (dup2(output_fd, 1) == -1) {
                printf("Unable to get file descriptor for %s\n", cmd_buf->output_filename);
                exit(1);
            }
            close(output_fd);
        }

        // Ready to execute!
        execvp(cmd_buf->argv[0], cmd_buf->argv);
        // If we make it this far, something went wrong
        if (errno == ENOEXEC) {
            printf("%s: command not found\n", cmd_buf->argv[0]);
        }
        else {
            printf("%s: no such file or directory\n", cmd_buf->argv[0]);
        }
        fflush(NULL);
        exit(1);
    }
    else {
        // This is the parent process, set foreground_process or update background_processes
        int child_status;
        child_pid = waitpid(child_pid, &child_status, 0);
        return WEXITSTATUS(child_status);
    }


    // If command should execute in background, print pid when it begins
    // Background commands default to redirecting stdin and stdout to /dev/null
    // When it terminates, print process id and exit status.

    // If a command terminates after being killed, print message before continuing
    return 0;
}

void clear_command(struct command *cmd_buf) {
    // Free string that was allocated to hold this command
    free(cmd_buf->argv[0]); // first argument is always the start of the string
    init_command(cmd_buf);
}

// TODO: debug this
void cd(char *input) {
    // Check if we need to cd to home directory
    if (input[2] == '\0') {
        if (chdir(getenv("HOME")) != 0) {
            printf("no such directory: %s\n", getenv("HOME"));
            fflush(NULL);
        }
    }
    else {
        if (chdir(input + 3) != 0) {
            printf("no such directory: %s\n", input + 3);
        }
    }
}

// Checks for builtin commands and comments/blank lines. Returns a number
// corresponding to the command that was run
int check_builtin_commands(char *input, int last_status) {
    // Check for comment/newline
    if (input[0] == '#' || input[0] == '\0') {
        return 0;
    }
    if (strcmp(input, "exit") == 0) {
        return 1;
    }
    if (strcmp(input, "status") == 0) {
        printf("exit value %d\n", last_status);
        fflush(NULL);
        return 2;
    }
    // Make sure that cd isn't part of the start of the name of another command
    if (strstr(input, "cd") == input && (input[2] == ' ' || input[2] == '\0')) {
        cd(input);
        return 3;
    }
    return -1;
}

static void sigint_handler() {
    // TODO: Make this do something useful
    puts("yo\n");
}

void print_command(struct command *cmd_buf) {
    printf("argc: %d\nargv: ", cmd_buf->argc);
    for (int i = 0; cmd_buf->argv[i] != NULL; ++i) {
        printf("%s | ", cmd_buf->argv[i]);
    }
    printf("\nInput file: %s\n", cmd_buf->input_filename);
    printf("Output file: %s\n", cmd_buf->output_filename);
    printf("Is background: %d\n\n", cmd_buf->background);
    fflush(NULL);
}

int main(int argc, char **argv) {
    // TODO: Detect SIGINT
    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        printf("Unable to set signal handler for SIGINT.\n");
        fflush(NULL);
        return EXIT_FAILURE;
    }
    // TODO: Detect SIGSTP

    char input[2048];
    struct command cmd_buf;
    init_command(&cmd_buf);
    int last_status = 0;
    bool is_foreground_only = false;

    while (1) {
        printf(": ");
        fflush(NULL);
        fgets(input, 2048, stdin);
        // Get rid of newline character
        input[strcspn(input, "\n")] = '\0';

        int ran_builtin = check_builtin_commands(input, last_status);
        if (ran_builtin >= 0) {
            // Ran one of the builtin commands
            if (ran_builtin == 1) {
                // Ran exit command
                return EXIT_SUCCESS;
            }
            // go back to prompt
            continue;
        }

        parse_command(&cmd_buf, input, is_foreground_only);
        print_command(&cmd_buf);
        int exit_status = execute_command(&cmd_buf);
        clear_command(&cmd_buf);
        
        // Update exit status if necessary
        if (exit_status != -1) {
            last_status = exit_status;
        }
    }

    return EXIT_SUCCESS;
}