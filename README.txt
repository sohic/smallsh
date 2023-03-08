SMALLSH - Write your own shell in C

smallsh will implement a subset of features of well-known shells, such as bash. This program will

    Provide a prompt for running commands
    Handle blank lines and comments, which are lines beginning with the # character
    Provide expansion for the variable $$
    Execute 3 commands exit, cd, and status via code built into the shell
    Execute other commands by creating new processes using a function from the exec family of functions
    Support input and output redirection
    Support running commands in foreground and background processes
    Implement custom handlers for 2 signals, SIGINT and SIGTSTP

+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
HOW TO RUN

The require files to run the smallsh program are listed below. It uses a makefile to compile.
Simply type "make" at the prompt where the working directory contains the below listed files.
The executable is "smallsh".

Files: 1.  main.c 2. smallsh.c 3. utilities.c 4. signals.c 5. smallsh.h 6. Makefile


++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
