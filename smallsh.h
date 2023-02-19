#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

#ifndef __SMALLSH_H__
#define __SMALLSH_H__

// constants for input buffer and max arguments.
#define INPUT_BUFF 2048
#define MAX_ARG 512

// struct to store user input
struct userInput{
  char* termInput;
  char* command;
  char** args;
  int total_args;
};

/* structs for SIGINT and SIGTSTP signals */

extern struct sigaction SIGINT_action;

extern struct sigaction SIGTSTP_action;

/* global variables for whether to allow background process (allowBack) and 
 * whether a child process is currently active (childAlert).  These two boolean
 * variables behave like switches (true/false).*/

extern bool allowBack;

extern bool childAlert;

/* global variables that keeps track of whether the current command is to be run in
 * foreground or background (background).*/

extern bool background;

/* global variable childStatus is used to store the staus of child processes */

extern int childStatus;

/* global variable spawnPid is used to store the PID of the child process */

extern pid_t spawnPid;
/**************************************************************************************
 * ************************************************************************************
 *                                 smallsh.c functions 
 * ************************************************************************************
 * ************************************************************************************/



void smallsh();
/* ************************************************************************************
 * smallsh():
 *
 * Top level function that runs the shell
 *
 * Parameter:  N/A
 * Return:     N/A
 * ************************************************************************************/

void exitCommand(int*);
/* ************************************************************************************
 * exitCommand():
 *
 * Function for exit command.  Takes a pointer array storing the pids of currently 
 * running processes as a parameter and returns nothing.
 *
 * Parameter:  int*
 * Return:     N/A
 * ************************************************************************************/

void cdCommand(struct userInput*);
/* ************************************************************************************
 * cdCommand():
 *
 * Function for the cd command.  Takes a pointer to the struct userInput as 
 * a parameter and returns nothing.
 *
 * Parameter: struct userInput*
 * Return:    N/A
 * ************************************************************************************/


void statusCommand(int);
/* ************************************************************************************
 * statusCommand():
 *
 * Function for the status command. Takes an int (status) as parameter and
 * returns nothing.
 *
 * Parameter:  int
 * Return:     N/A
 * ************************************************************************************/

void otherCommand(struct userInput*, int*);
/* ************************************************************************************
 * otherCommand():
 *
 * Function to run all commands other than cd, status, and exit.  Takes a pointer
 * to the struct userInput and a pointer to an int array that stores the child pids of
 * the active processes.  Returns nothing.
 *
 * Parameter:  struct userInput*
 *             int* 
 * Return:     N/A
 * ***********************************************************************************/



/**************************************************************************************
 * ************************************************************************************
 *                            utilities.c functions 
 * ************************************************************************************
 * ************************************************************************************/



int getUserInput(struct userInput*);
/* *************************************************************************************
 * getUserInput():
 *
 * This function reads in user input and populates the struct userInput.  The function
 * also calls replaceVar() to perform variable expansion.  The function takes a pointer
 * to the struct userInput as a parameter and returns an integer representing the 
 * command entered by the user.
 *
 * Parameter:  struct userInput*
 * Return:     int
 * *************************************************************************************/

void replaceVar(struct userInput*);
/* ************************************************************************************ 
 *  replaceVar():
 *
 *  This function examines the user input for "$$" for variable expansion.  Each instance
 *  of "$$" is replaced by the parent pid.  The function takes a pointer to the struct
 *  userInput as a parameter and returns nothing.
 *
 *  Parameters:  struct userInput*
 *  Return:      N/A
 *  ************************************************************************************/

char* makeAbsolute(char*);
/* ************************************************************************************
 * makeAbsolute():
 *
 * This function converts a directory path into an absolute path.  The function takes
 * a pointer to a directory path as a parameter and returns the ultimate path to the 
 * directory. 
 *
 * Parameters:  char*
 *
 * Return:      char*
 * ************************************************************************************/

bool resolveAmp(bool, struct userInput*);
/***************************************************************************************
 * resolveAmp():
 *
 * This function examines the user input for '&' to determine if the command should be
 * executed in the foreground or background. The function takes a pointer to the struct
 * userInput and boolean used for storing true (if background process) and false (if a
 * foreground process).  The function returns this boolean.
 *
 * Parameters:  bool 
 *              struct userInput*
 *
 * Return:     bool
 * ************************************************************************************/

bool ioRedirection(struct userInput*, bool, int*);
/* ************************************************************************************
 * ioRedirection():
 *
 * This function searches for ">" and "<" within the arguments entered by the user and 
 * ensures that they are followed by a filename.  The function then routs the stdin (to)
 * and stdout (from) the applicable files.  This function is also responsible for routing 
 * the stdin/stdout for background cases.  In case there is a redirection, execvp() is 
 * used to execute the command within this function.  This function takes a pointer to 
 * struct userInput, a boolean that indicates if the command is to be executed in the 
 * background, and an array that has stores the pids of the processes that are currently
 * running as parameters.  The function returns a boolean indicating if redirection was 
 * required.
 *
 * Parameters:  struct userInput*
 *              bool
 *              int*
 * Return:      bool
 * ************************************************************************************/


/**************************************************************************************
 * ************************************************************************************
 *                                   signals.c functions 
 * ************************************************************************************
 * ************************************************************************************/

void handle_SIGINT(int);
/* **************************************************************************************
 * handle_SIGINT():
 *
 * Signal handler for SIGINT 
 *
 * Parameter:  int 
 * Return:     N/A
 * **************************************************************************************/

void handle_SIGTSTP(int);
/* **************************************************************************************
 * handle_SIGTSTP():
 *
 * signal handler for SIGTSTP 
 *
 * Parameter:  int
 * Return:     N/A
 * **************************************************************************************/

void SIGINTsetup(int);
/****************************************************************************************
 * SIGINTsetup():
 *
 * setup function for struct SIGINT_action.  Takes an int as a parameter and returns
 * nothing.
 *
 * Parameter:  int -> 0: ignore signal
 *                    else: use signal handler handle_SIGINT
 * Return:     N/A
 * **************************************************************************************/

void SIGTSTPsetup(int);
/* **************************************************************************************
 * SIGTSTPsetup():
 *
 * setup function for struct sigaction SIGTSTP_action.  Takes an int as a parameter
 * and returns nothing.
 *
 * Parameter:  int ->  0: ignore signal, sa_flag = 0
 *                     else: use signal handler handle_SIGTSTP, also set sa_flag = SA_RESTART
 * Return:     N/A
 *
 * ***************************************************************************************/

#endif
