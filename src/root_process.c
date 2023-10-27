#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "../include/utils.h"

#define WRITE (O_WRONLY | O_CREAT | O_TRUNC)
#define PERM (S_IRUSR | S_IWUSR)
char *output_file_folder = "output/final_submission/";

#define BUFFER_SIZE 4096

// Done by: Stephen and RobertW, Checked by:
void redirection(char **dup_list, int size, char* root_dir) {
    // TODO(overview): redirect standard output to an output file in output_file_folder("output/final_submission/")
    // TODO(step1): determine the filename based on root_dir. e.g. if root_dir is "./root_directories/root1", the output file's name should be "root1.txt"
    char file_name[BUFFER_SIZE];
    memset(file_name, 0, sizeof(file_name));
    sprintf(file_name, "%s", extract_filename(root_dir));
    strcat(file_name, ".txt");

    //TODO(step2): redirect standard output to output file (output/final_submission/root*.txt)
    char output_path[BUFFER_SIZE];
    memset(output_path, 0, sizeof(output_path));
    sprintf(output_path, "output/final_submission/");
    strcat(output_path, file_name);
    
    int fd = open(output_path, WRITE, PERM);
    if(fd == -1){
        perror("Failed to open file\n");
        exit(-1);
    }

    // create a duplicate of STDOUT_FILENO and use dup2 to redirect standard output to output file
    int TEMP_STDOUT_FILENO = dup(STDOUT_FILENO);
    if(dup2(fd, STDOUT_FILENO) == -1){
        perror("Failed to redirect output\n");
        exit(-1);
    }
    
    //TODO(step3): read the content each symbolic link in dup_list, write the path as well as the content of symbolic link to output file(as shown in expected)
    for (int i = 0; i < size; i++) {
        char link_content[BUFFER_SIZE];
        memset(link_content, 0, sizeof(link_content));
        ssize_t link_len = readlink(dup_list[i], link_content, sizeof(link_content) - 1);

        if (link_len == -1) {
            perror("Failed to read symbolic link");
            exit(-1);
        } else {
            printf("[<path of symbolic link> --> <path of retained file>] : [%s --> %s]\n", dup_list[i], link_content);
        }
    }

    fflush(stdout);
    // Restore the original standard output
    dup2(TEMP_STDOUT_FILENO, STDOUT_FILENO);
    close(TEMP_STDOUT_FILENO);
    close(fd);
}

void create_symlinks(char **dup_list, char **retain_list, int size) {
    //TODO(): create symbolic link at the location of deleted duplicate file
    //TODO(): dup_list[i] will be the symbolic link for retain_list[i]
    for(int i = 0; i < size; i++) {
        if (symlink(retain_list[i], dup_list[i]) != 0) {
            printf("Error creating symbolic link");
            exit(-1); 
        }
    }
}

void delete_duplicate_files(char **dup_list, int size) {
    //TODO(): delete duplicate files, each element in dup_list is the path of the duplicate file
    for(int i = 0; i < size; i++) {
        if(remove(dup_list[i]) != 0) {
            printf("Error deleting file");
            exit(-1); 
        }
    }
}

// Done by: RobertW, Checked by:
// ./root_directories <directory>
int main(int argc, char* argv[]) {
    if (argc != 2) {
        // dir is the root_directories directory to start with
        // e.g. ./root_directories/root1
        printf("Usage: ./root_process <dir> \n");
        return 1;
    }

    //TODO(overview): fork the first non_leaf process associated with root directory("./root_directories/root*")
    char* root_directory = argv[1];
    char all_filepath_hashvalue[BUFFER_SIZE * 8]; //buffer for gathering all data transferred from child process
    memset(all_filepath_hashvalue, 0, sizeof(all_filepath_hashvalue)); //initialize all_filepath_hashvalue array to zeros
    
    //TODO(step1): construct pipe
    int fd[2];
    if(pipe(fd) == -1) {
        printf("error constructing pipe");
        return 1;
    }

    //TODO(step2): fork() child process & read data from pipe to all_filepath_hashvalue
    pid_t first_proc;
    first_proc = fork();

    if (first_proc < 0) {
        printf("error forking");
        return 1;
    }
    else if (first_proc == 0) {   // Child process
        close(fd[0]); // Close read end
        char write_end[2] = ""; // room for pipe and null term
        sprintf(write_end, "%d", fd[1]);
        execl("./nonleaf_process", "./nonleaf_process", root_directory, write_end, NULL);
        perror("Exec failed");
        exit(1);
    }
    else { // Parent process
        close(fd[1]);  // Close write end
        printf("bruh\n");
        wait(NULL); // wait for all children to finish
        printf("FFFFF\n");
        // Read in file hashes and aggregate the file hashes
        read(fd[0], all_filepath_hashvalue, BUFFER_SIZE);

        close(fd[0]);
    }
    
    //TODO(step3): malloc dup_list and retain list & use parse_hash() in utils.c to parse all_filepath_hashvalue
    // dup_list: list of paths of duplicate files. We need to delete the files and create symbolic links at the location
    // retain_list: list of paths of unique files. We will create symbolic links for those files
    char** dup_list = (char**)malloc(BUFFER_SIZE * 8);
    char** retain_list = (char**)malloc(BUFFER_SIZE * 8);
    
    int size = parse_hash(all_filepath_hashvalue, dup_list, retain_list);

    // printf("all-file\n");
    // char *ptr = all_filepath_hashvalue;
    // while (*ptr != '\0' && ptr < all_filepath_hashvalue + sizeof(all_filepath_hashvalue)) {
    //     printf("%s\n", ptr);
    //     ptr += strlen(ptr) + 1;
    // }
        
    // printf("%ld\n", sizeof(dup_list));
    // char** temp_dup = dup_list;
    // printf("dup-list\n");
    // while (*temp_dup) {
    //     printf("%s\n", *temp_dup);
    //     temp_dup++;
    // }
    // char** temp_retain = retain_list;
    // printf("retain-list\n");
    // while (*temp_retain) {
    //     printf("%s\n", *temp_retain);
    //     temp_retain++;
    // }

    //TODO(step4): implement the functions

    // printf("%d\n", size);
    // printf("_____________________________________________\n");
    delete_duplicate_files(dup_list, size);
    create_symlinks(dup_list, retain_list, size);
    redirection(dup_list, size, argv[1]);

    //TODO(step5): free any arrays that are allocated using malloc!!

    for (int i = 0; i < size; i++) {
        free(dup_list[i]);
        if (strcmp(retain_list[i], "ToFree")) // check if buffer already marked to free
            strcpy(retain_list[i], "ToFree");
        else
            retain_list[i] = NULL;
    }

    for (int i = 0; i < size; i++) {
        if (retain_list[i] != NULL) {
            free(retain_list[i]);
        }
    }

    free(dup_list);
    free(retain_list);
    dup_list = NULL;
    retain_list = NULL;
}
