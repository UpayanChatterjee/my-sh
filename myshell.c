#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_LINE 1024
#define MAX_ARGS 64
#define HISTORY_SIZE 50

// Global history array
char *history[HISTORY_SIZE];
int history_count = 0;

// Function prototypes
char *read_input();
char **parse_input(char *input);
int execute_builtin(char **args);
void execute_external(char **args, int background);
void execute_command(char **args);
void add_to_history(char *command);
int has_pipe(char **args);
void handle_piping(char **args);
int has_redirection(char **args);
void handle_redirection(char **args);
int is_background(char **args);
void run_script(char *filename);
void print_error(char *msg);

int main(int argc, char *argv[])
{
    // If a script file is provided, run it in batch mode
    if (argc > 1)
    {
        run_script(argv[1]);
        return 0;
    }

    char *input;
    char **args;

    // Main shell loop
    while (1)
    {
        input = read_input();
        if (!input)
            break; // EOF or error

        add_to_history(input);

        args = parse_input(input);
        if (args[0] != NULL)
        {
            execute_command(args);
        }

        free(input);
        free(args);
    }

    return 0;
}

// Read user input using readline for history support
char *read_input()
{
    char *input = readline("> ");
    return input;
}

// Parse input into command and arguments using strtok
char **parse_input(char *input)
{
    char **args = malloc(MAX_ARGS * sizeof(char *));
    if (!args)
    {
        print_error("Memory allocation failed");
        exit(1);
    }

    char *token = strtok(input, " \t\n");
    int i = 0;
    while (token != NULL && i < MAX_ARGS - 1)
    {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
    return args;
}

// Execute built-in commands
int execute_builtin(char **args)
{
    if (strcmp(args[0], "cd") == 0)
    {
        if (args[1] == NULL)
        {
            print_error("cd: expected argument");
        }
        else
        {
            if (chdir(args[1]) != 0)
            {
                perror("cd");
            }
        }
        return 1;
    }
    else if (strcmp(args[0], "exit") == 0)
    {
        exit(0);
    }
    else if (strcmp(args[0], "help") == 0)
    {
        printf("Built-in commands:\n");
        printf("  cd <dir>    - Change directory\n");
        printf("  exit        - Exit the shell\n");
        printf("  help        - Display this help\n");
        printf("External commands are executed using fork and execvp.\n");
        printf("Supports piping (|), redirection (>, <), and background (&).\n");
        return 1;
    }
    return 0; // Not a built-in
}

// Execute external commands using fork and execvp
void execute_external(char **args, int background)
{
    pid_t pid = fork();
    if (pid == 0)
    {
        // Child process: execute the command
        handle_redirection(args); // Handle I/O redirection if present
        execvp(args[0], args);
        // If execvp fails
        perror("execvp");
        exit(1);
    }
    else if (pid > 0)
    {
        // Parent process
        if (!background)
        {
            waitpid(pid, NULL, 0); // Wait for child to finish
        }
        else
        {
            printf("[Background process started with PID %d]\n", pid);
        }
    }
    else
    {
        perror("fork");
    }
}

// Main command execution dispatcher
void execute_command(char **args)
{
    if (execute_builtin(args))
        return;

    int background = is_background(args);
    if (background)
    {
        // Remove & from args
        int i = 0;
        while (args[i] != NULL)
            i++;
        args[i - 1] = NULL;
    }

    if (has_pipe(args))
    {
        handle_piping(args);
    }
    else
    {
        execute_external(args, background);
    }
}

// Add command to history
void add_to_history(char *command)
{
    if (history_count < HISTORY_SIZE)
    {
        history[history_count++] = strdup(command);
    }
    else
    {
        free(history[0]);
        for (int i = 0; i < HISTORY_SIZE - 1; i++)
        {
            history[i] = history[i + 1];
        }
        history[HISTORY_SIZE - 1] = strdup(command);
    }
    add_history(command); // Add to readline history
}

// Check if command has pipe
int has_pipe(char **args)
{
    int i = 0;
    while (args[i] != NULL)
    {
        if (strcmp(args[i], "|") == 0)
            return 1;
        i++;
    }
    return 0;
}

// Handle piping for two commands
void handle_piping(char **args)
{
    int pipe_pos = -1;
    int i = 0;
    while (args[i] != NULL)
    {
        if (strcmp(args[i], "|") == 0)
        {
            pipe_pos = i;
            break;
        }
        i++;
    }
    if (pipe_pos == -1)
        return;

    // Split into left and right commands
    char *left[MAX_ARGS];
    char *right[MAX_ARGS];
    for (i = 0; i < pipe_pos; i++)
        left[i] = args[i];
    left[i] = NULL;
    i = pipe_pos + 1;
    int j = 0;
    while (args[i] != NULL)
        right[j++] = args[i++];
    right[j] = NULL;

    // Create pipe
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        return;
    }

    // Fork first child for left command
    pid_t pid1 = fork();
    if (pid1 == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        handle_redirection(left); // Handle redirection in left if any
        execvp(left[0], left);
        perror("execvp");
        exit(1);
    }

    // Fork second child for right command
    pid_t pid2 = fork();
    if (pid2 == 0)
    {
        close(pipefd[1]);
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        handle_redirection(right); // Handle redirection in right if any
        execvp(right[0], right);
        perror("execvp");
        exit(1);
    }

    // Parent closes pipe and waits
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

// Check if command has redirection
int has_redirection(char **args)
{
    int i = 0;
    while (args[i] != NULL)
    {
        if (strcmp(args[i], ">") == 0 || strcmp(args[i], "<") == 0)
            return 1;
        i++;
    }
    return 0;
}

// Handle I/O redirection using dup2
void handle_redirection(char **args)
{
    int i = 0;
    while (args[i] != NULL)
    {
        if (strcmp(args[i], ">") == 0)
        {
            int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1)
            {
                perror("open");
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
            // Remove redirection from args
            args[i] = NULL;
        }
        else if (strcmp(args[i], "<") == 0)
        {
            int fd = open(args[i + 1], O_RDONLY);
            if (fd == -1)
            {
                perror("open");
                exit(1);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
            // Remove redirection from args
            args[i] = NULL;
        }
        i++;
    }
}

// Check if command should run in background
int is_background(char **args)
{
    int i = 0;
    while (args[i] != NULL)
        i++;
    if (i > 0 && strcmp(args[i - 1], "&") == 0)
        return 1;
    return 0;
}

// Run commands from a script file
void run_script(char *filename)
{
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        perror("fopen");
        return;
    }

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), file))
    {
        // Remove trailing newline
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0)
            continue; // Skip empty lines

        add_to_history(line);
        char **args = parse_input(line);
        if (args[0] != NULL)
        {
            execute_command(args);
        }
        free(args);
    }

    fclose(file);
}

// Print custom error messages
void print_error(char *msg)
{
    fprintf(stderr, "myshell: %s\n", msg);
}