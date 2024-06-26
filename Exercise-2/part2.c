#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>

void write_message(const char *message, int count){
    for (int i = 0; i < count; i++){
        printf("%s\n", message);
        usleep((rand() % 100) * 1000); // Random delay between 0 and 99 milliseconds
    }
}

int main(int argc, char *argv[]){

    // Check if the number of arguments is not under 4
    if (argc < 4){
        fprintf(stderr, "Usage: %s <message1> <message2> ... <count>\n", argv[0]);
        return 1;
    }
    // Get the number of time eche message will be printed
    int count = atoi(argv[argc - 1]);
    
    // Create a child process for each message
    for (int i = 1; i < argc - 1; i++){
        pid_t pid = fork();
        if (pid < 0){
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0){
            // Child process
            int fd;
            while (fd = open("lockfile.lock", O_CREAT | O_EXCL, 0666) == -1){
                if (errno == EEXIST){
                    usleep((rand() % 100) * 1000); // Random delay between 0 and 99 milliseconds
                    continue;
                } else {
                    perror("open");
                    exit(EXIT_FAILURE);
                }
                usleep((rand() % 100) * 1000); // Random delay between 0 and 99 milliseconds
            }
            write_message(argv[i], count);
            // Delete the lockfile
            if (unlink("lockfile.lock") == -1) {
                perror("unlink");
                exit(EXIT_FAILURE);
            }
            exit(0);
        }
    }
    // Parent process
    // Wait for all child processes to complete
    while (wait(NULL) > 0);
    return 1;
}