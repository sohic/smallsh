/*****************************************************************************************************
 * Author:  Chandan Sohi
 * Date:    10/26/2022
 * Class:   CS344
 * Project: smallsh
 *
 *
 *****************************************************************************************************/
#include "smallsh.h"

/* Declare global variables*/
bool allowBack;  //switch for whether to allow background processes or not
bool childAlert; //switch to determine if child process is running used in ctrl-z signal
bool background; //true if process is to be run in background, false otherwise
int childStatus; //stores the status of the child process
pid_t spawnPid;  //stores the PID of the child process

// initialize sigaction structs SIGINT_action and SIGTSTP_action
struct sigaction SIGINT_action = {0}, SIGTSTP_action = {0};


/*****************************************************************************************************
 * void smallsh() - function is the top level function that runs the shell. It takes no parameters and 
 *  returns nothing.
 *
 * **************************************************************************************************/
void smallsh(){
  // Initialize background processes to true
  allowBack = true;


  //initialize some variables
  int foregroundStatus = -1; //status of last non built in foreground command
  int cmd = 3;      //command initialized to other
    

  /* allocate memory to store child processes that are currently running and initialize them to 0*/
  int* cProcessArr = (int*) malloc((INPUT_BUFF + 1) * sizeof(char));
  for (int i = 0; i < 500; i++) cProcessArr[i] = 0;



  /* loop to keep the shell running - asking for the next command - until exit is entered*/
  while (cmd != -1){
    //Set SIGINT to ignore and SIGTSTP to be handled by handle_SIGTSTP
    SIGINTsetup(0);
    SIGTSTPsetup(1);
    //initialize childAlert to false
    childAlert = false;
 

    /* Initialize structure to store user input (command) and allocate memory for it. */
    struct userInput* inputPtr;
    inputPtr = (struct userInput*) malloc(sizeof(struct userInput));

    /* allocate memory for termInput and args*/
    inputPtr -> termInput = (char*) malloc((INPUT_BUFF + 1)* sizeof(char));
    inputPtr -> args = malloc((MAX_ARG) * sizeof(char*));
    
    background = false;
    /* Get command via user input through the utility function getUserInput. */
    cmd = getUserInput(inputPtr);


    /* switch structure to run commands, similar to those shown in course modules. */
    switch(cmd){
      case 1:
        //Case 1 is for cd command -> it runs the cdCommand function
        cdCommand(inputPtr);
        break;
      case -1:
        //Case -1 is the exit command;
        exitCommand(cProcessArr);
        break;
      case 2:
        statusCommand(foregroundStatus);
        //Case 2 is the status command for the last foreground command
        break;
      case 3:
        //Case 3 is for all commands other than those that are built in (cd, status, exit)
        otherCommand(inputPtr, cProcessArr);
        //if command was run in foreground, set foregroundStatus to status
        if (!background) foregroundStatus = childStatus;
        break;
      default:
        //Default case handles empty lines and comments (starting w/ #)
        break;
    }

    // at end of each command (excluding exit) check array containing child processes to see if they have finished
    // executing and print out message as needed.
    if (cmd != -1){
        for (int i = 0; i < 500; i++){
          if (cProcessArr[i] != 0){
              spawnPid = waitpid(cProcessArr[i], &childStatus, WNOHANG);
              if (spawnPid == cProcessArr[i]){
                printf("background pid %d is done: ", cProcessArr[i]);
                fflush(stdout);
                statusCommand(childStatus);
                cProcessArr[i] = 0;
              }
          }
        }

    }
    // free allocated memory for struct inputPtr
    free(inputPtr -> termInput);
    free(inputPtr -> args);
    free(inputPtr);
  }
  // free allocated memory for array that stores child processes
  free(cProcessArr);
}


/**************************************************************************************************
 * Function exitCommand handles the "exit" command.  It checks for all background processes currently
 * in progress and terminates them.  An array storing all the pids of the background processes is
 * passed as a parameter (cProcessArr).
 *
 * Parameter:  int * cProcessArr (pointer array storing all active pids)
 *
 * ************************************************************************************************/
void exitCommand(int *cProcessArr){
  //search and kill active processes
  for (int i = 0; i < 500; i++){
    if (cProcessArr[i] != 0){
      kill(cProcessArr[i], SIGKILL);
      cProcessArr[i] = 0;
    }
  }
}

/**************************************************************************************************
 * Function cdCommand handles the built in "cd" - change directory - command for the shell
 * The function takes struct userInput as a parameter and returns nothing.
 *
 * Parameter:  struct userInput inputPtr
 *
 * ***********************************************************************************************/

void cdCommand(struct userInput* inputPtr){  
  
  char cwd[256];
  char** args = inputPtr -> args;
  int ampIndex = inputPtr -> total_args;
 
  char *tempCommand = malloc((INPUT_BUFF + 1) * sizeof(char));
  bool noFile = false;
  
  //if there is only one argument - just command. Skip logic regarding handling "&"
  if (ampIndex == 1){
    noFile = true;
    goto skip;
  }
 
  //create pointer for last argument of user input and allocate memory for it
  char *lastArg = malloc((INPUT_BUFF + 1) * sizeof(char));
  strcpy(lastArg, args[ampIndex - 1]);

  //condition when there are only 2 arguments and the 2nd one is "&"
  if (strcmp(lastArg, "&") == 0 && ampIndex == 2){
    noFile = true;  
    goto skip;
  }

  // for all other conditions
  else{
    //get current working directory and store path to cwd
    getcwd(cwd, sizeof(cwd));
    char *absolute;
    //check if args[1] contains current working directory
    absolute = strstr(args[1], cwd);
    
    //if absolute set tempcommand to second argument (args[1]) of user input
    if (absolute) strcpy(tempCommand, args[1]);

    // else use makeAbsolute to convert user entered path to an absolute path
    if (!absolute){
      strcpy(tempCommand, args[1]);
      tempCommand = makeAbsolute(tempCommand);
    }
  }

skip:
  // if no file/path was etered change directory to home directory
  if (noFile == true) chdir(getenv("HOME"));
  //else change dir to path stored in tempCommand, print error if no such file or directory
  else if (chdir(tempCommand) != 0){
    fprintf(stderr, "-smallsh: cd: %s: No such file or directory\n", args[1]);
    fflush(stderr);
  }
 
  free(tempCommand);
  if (ampIndex > 1) free(lastArg);
}

/************************************************************************************************
 * Function statusCommand prints the exits status or the termination signal of a child process.
 * The function takes status (int) of the child and returns nothing.
 *
 * **********************************************************************************************/
void statusCommand(int status){
  // if no command has been executed
  if (status == -1) printf("exit value 0\n");
  //normal exit
  else if (WIFEXITED(status)) printf("exit value %d\n", WEXITSTATUS(status));
  //terminated by signal
  else printf("terminated by signal %d\n", WTERMSIG(status));
  fflush(stdout);
}

/************************************************************************************************
 *  Function otherCommand handles all non built-in (cd, status, exit) commands.  The function
 *  takes the following parameters:
 *  Parameters:  struct userInput* inputPtr - pointer to the userInput struct
 *               int *cProcessArr - a pointer to the cProcessArr that stores the child processes 
 *               currently running
 *
 * **********************************************************************************************/
void otherCommand(struct userInput* inputPtr, int *cProcessArr){ 
    // check to look for "&"
    background = resolveAmp(background, inputPtr);
 
    //int childStatus;
    int resExecvp = 0;
    bool result = false;

    //set childAlert to true, used for SIGTSTP signal
    childAlert = true;

    //similar to course module
    spawnPid = fork();

    switch(spawnPid){
      case -1:
        perror("fork()\n");
        fflush(stderr);
        exit(1);
        break;
      case 0:
        //ignore SIGTSTP signal
        SIGTSTPsetup(0);
        //print background message
        if (background){
          printf("background pid is %d\n", getpid());
          fflush(stdout);
        }
        if (!background) {
          //activate SIGINT signal
          SIGINTsetup(1);
        }
        //deal with "<" and ">" redirection, commands w/ redirection are executed in ioRedirection()
        result = ioRedirection(inputPtr, background, cProcessArr);
        if (!result){
          // execute commands with no redirection
          resExecvp = execvp(inputPtr -> command, inputPtr -> args);
          //error handling
          if (resExecvp < 0){
            fprintf(stderr, "%s: no such file or directory\n", inputPtr -> command);
            fflush(stderr);
            free(inputPtr -> termInput);
            free(inputPtr -> args);
            free(inputPtr);
            free(cProcessArr);
            exit(1);
          }
        }
        break;
      default:
        if (background){
          // store child pid in open slot in cProcessArr
          for (int i = 0; i < 500; i++){
              if (cProcessArr[i] == 0){
                cProcessArr[i] = spawnPid;
                break;
                }
           }
          //use WNOHANG for background process, immediately makes prompt available for user 
          spawnPid = waitpid(spawnPid, &childStatus, WNOHANG);
          sleep(1);
        }
        else{
          //waits for process to finish before prompt is made available to user
          spawnPid = waitpid(spawnPid, &childStatus, 0);
          //check if child process is running and SIGINT handling
          if (WIFEXITED(childStatus) == 0){
            signal(SIGINT, &handle_SIGINT);
            raise(SIGINT);
          }
        }
        break;
    }
 
 }

