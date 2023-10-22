#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

#define BUFFER_SIZE 1024
#define PATH_MAX 1024

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
    char *buffer = (char*) malloc(BUFFER_SIZE); // uh how many bytes do I malloc?
    memset(buffer, 0, BUFFER_SIZE);

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
    int read_pipes[10]; // Assume total # files <= 10
    int num_pipes = 0;

    while((entry = readdir(dir)) != NULL) {
        char path[strlen(file_path) + strlen(entry->d_name) + 2];
        sprintf(path, "%s/%s", file_path, entry->d_name);

        memset(fd, 0, sizeof(fd));
        if(pipe(fd) == -1){
            perror("Error constructing pipe");
            exit(1);
        }

        read_pipes[num_pipes] = fd[0]; // <-- no idea if this works
        num_pipes++;

        // if (entry->d_type == DT_DIR) { // directory
        //     pid_t child_pid = fork();

        //     if (child_pid == 0) { // child
        //         close(fd[0]); // Close the read end of the pipe
        //         char* write_end = "";
        //         sprintf(write_end, "%d", fd[1]);
        //         execl("./nonleaf_process", "./nonleaf_process", path, write_end, (char *)NULL);
        //         close(fd[1]); // Close the write end of the pipe  <-- this stuff should never run right?
        //         exit(0);
        //     } 
        //     else if (child_pid > 0) { // parent
        //         close(fd[1]); // Close the write end of the pipe
        //     } 
        //     else {
        //         perror("Fork failed");
        //         exit(1);
        //     }
        // }
        // else if (entry->d_type == DT_REG) { // regular file
        //     pid_t child_pid = fork();

        //     if (child_pid == 0) { // child
        //         close(fd[0]); // Close the read end of the pipe
        //         char* write_end = "";
        //         sprintf(write_end, "%d", fd[1]);
        //         execl("./leaf_process", "./leaf_process", path, write_end, (char *)NULL);
        //         close(fd[1]); // Close the write end of the pipe
        //         exit(0);
        //     }
        //     else if (child_pid > 0) { // parent
        //         close(fd[1]); // Close the write end of the pipe
        //     } 
        //     else {
        //         perror("Fork failed");
        //         exit(1);
        //     }

        // }

        // better code condenstation? -robe
        pid_t child_pid = fork();
        char process[18] = "";
        if (entry->d_type == DT_REG)
            strcpy(process, "./leaf_process");
        else if (entry->d_type = DT_DIR)
            strcpy(process, "./nonleaf_process");

        if (child_pid == 0) { // child
            close(fd[0]); // Close the read end of the pipe
            char* write_end = "";
            sprintf(write_end, "%d", fd[1]);
            execl(process, process, path, write_end, (char *)NULL);
            close(fd[1]); // Close the write end of the pipe
            exit(0);
        }
        else if (child_pid > 0) { // parent
            close(fd[1]); // Close the write end of the pipe
        } 
        else {
            perror("Fork failed");
            exit(1);
        }
    }

    //TODO(step5): read from pipe constructed for child process and write to pipe constructed for parent process

    // gather all information from children pipes
    for (int i = 0; i <= num_pipes; i++) {
        char buffer1[BUFFER_SIZE];
        memset(buffer1, 0, BUFFER_SIZE);
        while (read(read_pipes[i], buffer, BUFFER_SIZE));
        strcat(buffer, buffer1);
    }

    // send gathered info to parent
    write(fd1, buffer, strlen(buffer));

    free(buffer);
    closedir(dir);
    return 0;
}
