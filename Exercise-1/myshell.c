#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_HISTORY 100
#define MAX_COMMAND_LENGTH 100

char **history;
int history_count = 0;
char **additional_paths;
int path_count = 0;

void add_to_history(char *command) {
    if (history_count < MAX_HISTORY) {
        history[history_count] = strdup(command);
        history_count++;
    }
}

void print_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%d %s\n",i, history[i]);
    }
}

void change_directory(char *path) {
    if (chdir(path) != 0) {
        perror("cd failed");
    }
}

void print_working_directory() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("getcwd failed");
    }
}

void execute_command(char **args, char **envp) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        // Try to execute the command in each of the specified directories
        for (int i = 0; i < path_count; i++) {
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", additional_paths[i], args[0]);
            execve(path, args, envp);
        }
        // If not found in specified directories, use execvp which searches in PATH
        execvp(args[0], args);
        perror("exec failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        // Remove the additional error message
    } else {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv, char **envp) {
    additional_paths = (char **)malloc(argc * sizeof(char *));
    if (additional_paths == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    for (int i = 1; i < argc; i++) {
        additional_paths[path_count] = strdup(argv[i]);
        if (additional_paths[path_count] == NULL) {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }
        path_count++;
    }

    history = (char **)malloc(MAX_HISTORY * sizeof(char *));
    if (history == NULL) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < MAX_HISTORY; i++) {
        history[i] = NULL;
    }

    char command[MAX_COMMAND_LENGTH] = {0};
    while (1) {
        printf("$ ");
        fflush(stdout);
        if (fgets(command, sizeof(command), stdin) == NULL) {
            break;
        }
        command[strcspn(command, "\n")] = 0; // Remove the newline character
        add_to_history(command);

        char **args = (char **)malloc((MAX_COMMAND_LENGTH / 2 + 1) * sizeof(char *));
        if (args == NULL) {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < MAX_COMMAND_LENGTH / 2 + 1; i++) {
            args[i] = NULL;
        }

        char *token = strtok(command, " ");
        int i = 0;
        while (token != NULL) {
            args[i] = strdup(token);
            if (args[i] == NULL) {
                perror("malloc failed");
                exit(EXIT_FAILURE);
            }
            token = strtok(NULL, " ");
            i++;
        }

        if (args[0] == NULL) {
            free(args);
            continue;
        } else if (strcmp(args[0], "cd") == 0) {
            if (args[1] != NULL) {
                change_directory(args[1]);
            } else {
                fprintf(stderr, "cd: missing argument\n");
            }
        } else if (strcmp(args[0], "pwd") == 0) {
            print_working_directory();
        } else if (strcmp(args[0], "history") == 0) {
            print_history();
        } else if (strcmp(args[0], "exit") == 0) {
            free(args);
            break;
        } else {
            execute_command(args, envp);
        }

        if(history_count == MAX_HISTORY - 1) {
            break;
        }
        // Free args
        for (int i = 0; args[i] != NULL; i++) {
            free(args[i]);
        }
        free(args);
    }

    // Free allocated history and additional paths
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }
    free(history);
    for (int i = 0; i < path_count; i++) {
        free(additional_paths[i]);
    }
    free(additional_paths);

    return 0;
}
