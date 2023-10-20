#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

// Done by: Checked by:
int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: ./nonleaf_process <directory_path> <pipe_write_end> \n");
        return 1;
    }
    //TODO(overview): fork the child processes(non-leaf process or leaf process) each associated with items under <directory_path>
    //TODO(step1): get <file_path> <pipe_write_end> from argv[]
    char *file_path = argv[1];
    int fd1 = atoi(argv[2]);

    //TODO(step2): malloc buffer for gathering all data transferred from child process as in root_process.c
    char *buffer = (char*) malloc(); // uh how many bytes do I malloc?

    //TODO(step3): open directory
    DIR *dir = opendir(dirname);
    struct dirent *entry;
    if (dir == NULL) {
        perror("Error opening directory");
        exit(1);
    }

    //TODO(step4): traverse directory and fork child process
    // Hints: Maintain an array to keep track of each read end pipe of child process
    int fd[2];
    if(pipe(fd) == -1){
        perror("Error constructing pipe");
        exit(1);
    }
    while((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) { // directory
            pid_t child_pid = fork();
            if (child_pid == 0) { // child
                close(fd[0]); // Close the read end of the pipe
                char* write_end = "";
                sprintf(write_end, "%d", fd[1]);
                execl("./nonleaf_process", "./nonleaf_process", file_path, write_end, (char *)NULL);
                close(fd[1]); // Close the write end of the pipe
                exit(0);
            } 
            else if (child_pid > 0) { // parent
                close(fd[1]); // Close the write end of the pipe
                // Missing stuff here
                close(fd[0]); // Close the read end of the pipe in the parent process
            } 
            else {
                perror("Fork failed");
                exit(1);
            }
        }
        else if (entry->d_type == DT_REG) { // regular file
            pid_t child_pid = fork();
            if (child_pid == 0) { // child
                close(fd[0]); // Close the read end of the pipe
                char* write_end = "";
                sprintf(write_end, "%d", fd[1]);
                execl("./leaf_process", "./leaf_process", file_path, write_end, (char *)NULL);
                close(fd[1]); // Close the write end of the pipe
                exit(0);
            }
            else if (child_pid > 0) { // parent
                close(fd[1]); // Close the write end of the pipe
                // Missing stuff here
                close(fd[0]); // Close the read end of the pipe in the parent process
            } 
            else {
                perror("Fork failed");
                exit(1);
            }

        }
    }

    //TODO(step5): read from pipe constructed for child process and write to pipe constructed for parent process

    free(buffer);
    closedir(dir);
    return 0;
}
