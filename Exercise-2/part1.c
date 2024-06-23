#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    /*
        check if the number of arguments is correct, if not print an error message
        *the number of correct arguments is 5
        *the first argument is the name of the program
        *the second argument is the message that the parent will write
        *the third argument is the message that the first child will write
        *the fourth argument is the message that the second child will write
        *the fifth argument is the number of times that the parent will write the message
    */
    if (argc != 5){
        fprintf(stderr, "Usage: %s <parent_message> <child1_message> <child2_message> <count>\n", argv[0]);
        return 1;
    }
    int numOfTime = atoi(argv[4]);
    int fdin = open("output.txt", O_CREAT | O_WRONLY | O_APPEND , 0644);

    // if the file didn't open print an error message
    if (fdin == -1){
        perror("open");
        return 1;
    }

    // create a child process (child)
    pid_t pid = fork();

    // if the child process didn't create print an error message
    if (pid == -1){
        perror("fork");
        return 1;
    }

    // what the child process will do (child)
    if (pid == 0){

        // create a second child process (grandchild)
        pid_t pid2 = fork();

        // if the second child process didn't create print an error message
        if (pid2 == -1){
            perror("fork");
            return 1;
        }
        // what the second grandchild process will do (grandchild)
        if (pid2 == 0){
            // write the message of the second child to the file
            for (int i = 0; i < numOfTime ; i++){
                write(fdin, argv[3], sizeof(argv[3]));
            }
        }
        // what the first child process will do (child)
        else{
            // wait for the second child to finish
            wait(NULL);
            // write the message of the first child to the file
            for (int i = 0; i < numOfTime ; i++){
                write(fdin, argv[2], sizeof(argv[2]));
            }
        }  
    }
    // what the parent process will do (parent)
    else{
        wait(NULL);
        // write the message of the parent to the file
        for (int i = 0; i < numOfTime ; i++){
            write(fdin, argv[1], sizeof(argv[1]));
        }
    }
    return 1;
}