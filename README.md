**Project group number:** PA 27

**Group member names and x500s:**
1. Stephen Ma, ma000094
2. Robert Wang, wan00379
3. Robert Tan, tan00222

**The name of the CSELabs computer that you tested your code on:**
csel-kh1250-21.cselabs.umn.edu

**Members’ individual contributions:**  
Stephen Ma:
* Work on root_process.c, nonleaf_process.c, leaf_process.c
* Write pseudocode for README

Robert Wang:
* Work on root_process.c, nonleaf_process.c
* Check over leaf_process.c

Robert Tan:
* Work on nonleaf_process.c, leaf_process.c, root_process.c

**How you designed your program for creating the process tree**  
***Pseudocode:***  
root_process.c:  
create pipe 
call fork to spawn first child  
if current process is child process:  
  close read end of pipe  
  call execl to run ./nonleaf_process  
else if current process is parent process:    
  close write end of pipe  
  wait for child process
  read in file hashes and aggregate them  
  close read end of pipe
call parse_hash to populate dup_list and retain_list using what we read from the child process
call delete_duplicate_files
call create_symlinks
call redirection

nonleaf_process.c:  
iterate through each entry in the current directory
  skip "." and ".."
  create pipe    
  add read end of pipe to an array we initialized before the loop
  call fork
  if current process is child process:
    close read end of pipe
    if entry is a file:
      call execl to run ./leaf_process
    else if entry is a folder:
      call execl to run ./nonoleaf_process
  else if current process is parent process:
    close write end of pipe
wait for all children
loop through the array that contains all the read ends of the pipes:
  read from the pipe and concatenate to an empty string initialized before the loop
  close the read end of the pipe
write the aggregated information to parent process
  
leaf_process.c:  
compute hash_value of file using hash_data_block() 
Send this hash value, along with the file's full path, to the parent process through the given pipe write end  

***Differences from Intermediate Submission***
The main difference we had was in nonleaf_process.c. Here, we now use an array to store all read ends of the pipes, and after the while loop, we iterate through the array in order to read from each read end of the pipe and concatenate the string to buffer.
