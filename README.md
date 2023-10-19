**Project group number:** PA 27

**Group member names and x500s:**
1. Stephen Ma, ma000094
2. Robert Wang, wan00379
3. Robert Tan, tan00222

**The name of the CSELabs computer that you tested your code on:**
csel-kh1250-21.cselabs.umn.edu

**Members’ individual contributions plan:**  
Stephen Ma:
* Work on root_process.c, nonleaf_process.c, leaf_process.c
* Write pseudocode for README

Robert Wang:
* Work on root_process.c, nonleaf_process.c
* Check over leaf_process.c

Robert Tan:
* Work on nonleaf_process.c, leaf_process.c
* Check over root_process.c

**Plan on how you are going to construct the pipes and inter-process communication**  
***Pseudocode:***  
root_process.c:  
create pipe 
call fork to spawn first child  
if current process is child process:  
  close read end of pipe  
  run ./nonleaf_process  
  close write end of pipe  
else if current process is parent process:    
  close write end of pipe  
  read in file hashes and aggregate them  
  close read end of pipe

nonleaf_process.c:  
iterate through given directory
  create pipe    
  call fork to spawn child process for the following if else statement
  if current entry is a directory: create another non_leaf process using execl and give it the pipe's write end 
  else if current entry is a file: create a leaf process using execl and give it the pipe's write end
collect all children’s pipe messages using an array  
write the aggregated information to parent process
  
leaf_process.c:  
compute hash_value of file using hash_data_block() 
Send this hash value, along with the file's full path, to the parent process through the given pipe write end  


