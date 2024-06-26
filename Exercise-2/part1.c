#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

void write_to_file(const char *message, int count) {
    int fd = open("output.txt", O_CREAT | O_WRONLY | O_APPEND, 0644);
    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < count; i++) {
        if (write(fd, message, strlen(message)) == -1) {
            perror("write");
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <parent_message> <child1_message> <child2_message> <count>\n", argv[0]);
        return 1;
    }

    const char *parent_message = argv[1];
    const char *child1_message = argv[2];
    const char *child2_message = argv[3];
    int count = atoi(argv[4]);

    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) {
        // First child process
        write_to_file(child1_message, count);
        exit(0);
    }
    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid2 == 0) {
        // Second child process
        // Wait for the first child process to complete
        wait(NULL);
        write_to_file(child2_message, count);
        exit(0);
    }

    // Parent process
    // Wait for both child processes to complete
    int status;
    while (wait(&status) > 0);

    // After both children have finished, parent writes to file
    write_to_file(parent_message, count);

    return 0;
}
