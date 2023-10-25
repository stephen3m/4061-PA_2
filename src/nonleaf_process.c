#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <dirent.h>

#define BUFFER_SIZE 1024

// Done by: RoberT and Stephen, Checked by:
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
    char *buffer = (char*) malloc(BUFFER_SIZE); 
    memset(buffer, 0, BUFFER_SIZE);

    //TODO(step3): open directory
    DIR *dir = opendir(file_path);
    struct dirent *entry;
    if (dir == NULL) {
        perror("Error opening directory");
        exit(1);
    }

    int read_pipes[10]; // stores all the read ends of the pipes we create in while loop; we are given that 1 <= Total number of files <= 10
    int num_pipes = 0; // index used to traverse through read_pipes

    //TODO(step4): traverse directory and fork child process
    // Hints: Maintain an array to keep track of each read end pipe of child process
    while ((entry = readdir(dir)) != NULL) {
        char path[strlen(file_path) + strlen(entry->d_name) + 2];
        sprintf(path, "%s/%s", file_path, entry->d_name);

        // Create pipe
        int fd[2];
        if (pipe(fd) == -1) {
            perror("Error constructing pipe");
            exit(1);
        }

        // Add read end of pipe to read_pipes array 
        read_pipes[num_pipes] = fd[0];
        num_pipes++;

        pid_t child_pid = fork();

        if (child_pid == 0) { // Child process
            close(fd[0]); // Close the read end of the pipe

            char process[1024] = "";
            if (entry->d_type == DT_REG)
                strcpy(process, "./leaf_process");
            else if (entry->d_type == DT_DIR)
                strcpy(process, "./nonleaf_process");

            char write_end[1024];
            sprintf(write_end, "%d", fd[1]);

            // Execute the child process
            execl(process, process, path, write_end, (char *)NULL);
            perror("Exec failed");
            exit(1);
        }
        else if (child_pid > 0) { // Parent process
            close(fd[1]); // Close the write end of the pipe
        }
        else {
            perror("Fork failed");
            exit(1);
        }
    }
    
    while(wait(NULL) > 0); // wait for all children to finish

    //TODO(step5): read from pipe constructed for child process and write to pipe constructed for parent process

    // gather all information from children pipes and close read ends of the pipes
    for (int i = 0; i < num_pipes; i++) {
        char buffer1[BUFFER_SIZE];
        memset(buffer1, 0, BUFFER_SIZE);
        while (read(read_pipes[i], buffer1, BUFFER_SIZE) > 0) {
            strcat(buffer, buffer1);
            memset(buffer1, 0, BUFFER_SIZE);
        }
        close(read_pipes[i]);
    }

    // send gathered info to parent
    write(fd1, buffer, strlen(buffer));
    close(fd1);

    free(buffer);
    closedir(dir);
    return 0;
}
