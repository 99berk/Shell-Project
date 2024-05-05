# A Communicating Multi-shell

## Prerequisites
- C system calls
- Exec family functions
- Fork
- File I/O

## Learning Objectives
- Practice fork/exec relationships
- Understand the multi-process environment and how they can communicate through a shared object
- Understand the details of a shell program

## Introduction
In this project, you are required to implement a multi-shell program that opens multiple shells which share a file object with each other. For this, you are given singleshell.c and multishell.c files. Below are the details and todos for these files:

## Details
-singleshell.c-

It is a partially implemented shell application. Here, it will write all the stdout messages we see in the shell to a shared file object. The memory shared in the file is created as follows:

***
fd = shm_open(MY_SHARED_FILE_NAME, O_RDWR, 0); 
/* Map shared memory */
addr = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
***

You can think of it similar to malloc with more options. Therefore, you can treat the variable addr as an array. To copy a string to this address, you can use functions like strncpy or memcpy.

In singleshell.c, you need to complete the following:
1. Do not use system() anywhere in your program. Implement it by using exec and fork.
2. Program should copy/write all the stdout and stderr messages to addr as well, including process IDs and newlines.
3. The initial message is written when the program starts to run, containing its own process id, parent process id, and start date, month, day, and time.
4. Exit when the shell is given an exit command, writing its end date, month, day, and time before termination.
5. Implement functionality to run system programs (ls, etc.) as in a normal shell.
6. The shell should be able to run programs located in the current directory or another directory, along with appropriate guidance for given arguments.
7. Close all file descriptors and check for errors in all system calls.


-multishell.c-

It creates a given number of singleshell program instances.

1. Given a number, it creates that many singleshell processes using xterm to display them in separate windows.
2. After all singleshell instances finish, the data is saved from the shared memory (addr) to a newly opened file with the name shelllog-[datetime].txt.
3. The program ends by creating the log file and closing all file descriptors.
