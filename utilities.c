#include "smallsh.h"

/**************************************************************************************************
 * Function getUserInput() uses fgets() to read user input from the terminal.  The function also is
 * in charge of variable expansion by calling (replaceVar()) and parsing user input using strtok.
 *
 * Parameters:  strucu userInput* inputPtr
 * Return:  int (0 for comments and empty lines, 1 if command is "cd", 2 if command is "status", -1
 *          if command is "exit", for all other cases, 3 is returned).
 *
 * * ************************************************************************************************/
int getUserInput(struct userInput* inputPtr){

  int i = 0;

  //print ": " at beginning of user input line
  printf(": ");
  fflush(stdout);

  //use fgets to read in reader data
  fgets(inputPtr -> termInput, INPUT_BUFF + 1, stdin);
  int lenInput = strlen(inputPtr -> termInput);
  int returnValue;

  // check for comments "#" and empty lines
  if (inputPtr -> termInput[0] == '#' || inputPtr -> termInput[0] == '\n') return 0;

  //replace '\n' with '\0'
  if (inputPtr -> termInput[lenInput - 1] == '\n') inputPtr -> termInput[lenInput - 1] = '\0';

  //perform variable expansion
  replaceVar(inputPtr);

  //use strtok to parse user input for command and other arguments
  // command stored in inputPtr -> command
  // command + all other arguments stored in inputPtr -> args
  char *token = strtok(inputPtr -> termInput, " ");
  if (token == NULL){
    returnValue = 0;
    goto skip;
  }
  inputPtr -> command = token;
  inputPtr -> args[0] = token;
  i++;

  // keep track of number of arguments
  while (token != NULL){
    token = strtok(NULL, " ");
    inputPtr -> args[i] = token;
    i++;
  }

  //end args array with NULL
  inputPtr -> args[i] = NULL;

  //store args count in inputPtr -> total_args
  inputPtr -> total_args = i - 1;

  free(token);

  // returnValue logic
  if (strcmp(inputPtr -> command, "cd") == 0) returnValue = 1;
  else if (strcmp(inputPtr -> command, "status") == 0) returnValue = 2;
  else if (strcmp(inputPtr -> command, "exit") == 0) returnValue = -1;
  else returnValue = 3;

skip:

  return returnValue;
}


/**************************************************************************************************
 * Function replaceVar() examines the user input for "$$" for variable expansion.  It replace each
 * instance of "$$" with the pid.  The function takes a pointer to the struct userInput as a 
 * parameter and returns nothing.
 *
 * Parameter:  struct userInput* inputPtr - pointer to struct userInput
 *
 * ************************************************************************************************/

void replaceVar(struct userInput* inputPtr){
   
  int countTemp = 0;

  //Get pid and convert it to string
  char* pidStr = malloc(11 * sizeof(char));
  sprintf(pidStr, "%d", getpid());

  //allocate newStr to temporarily store converted command
  char *newStr = malloc((INPUT_BUFF + 1) * sizeof(char));
 
  //for loop to check user input character by character looking for "$$"

  for (int i = 0; i < (int)strlen(inputPtr -> termInput); i++){
    //logic to check for $$ and replace by pid
    if (inputPtr -> termInput[i] == '$' && inputPtr -> termInput[i + 1] == '$'){
      for (int j = 0; j < (int)strlen(pidStr); j++){
        newStr[countTemp] = pidStr[j];
        countTemp++;
      }
      //increment +1 for the second $
      i += 1;
    }
    //if not $$
    else{
      newStr[countTemp] = inputPtr -> termInput[i];
      countTemp++;

    }
  }
  //copy newStr to inputPtr -> termInput using memcpy
  memcpy(inputPtr -> termInput, &newStr[0], countTemp);
  //add '\0'
  inputPtr -> termInput[countTemp] = '\0';
  //free allocated memory
  free(pidStr);
  free(newStr); 
}

/************************************************************************************************
 * Function:  makeAbsolute
 * Parameters:  *path (pointer to a char pointer)
 * Return:  *path (pointer to a char pointer)
 *
 * This function converts a directory path from relative path to an absolute path.  This path is
 * used by the chdir() function to change the working directory.
 *
 *
 * **********************************************************************************************/
char* makeAbsolute(char *path){
  /* Create path to store path to current working directory and get current working director by  
   * calling getcwd()*/
  char cwd[2048];
  getcwd(cwd, sizeof(cwd));

  /*check to see if the path provided by the user starts with a '/'.  If so concactenate the user
   * input to the back of the current working directory.  Copy this new path to *path */
  if (path[0] =='/'){
    strcat(cwd, path);
    strcpy(path, cwd);
  }
  /* If the user provided path doesn't start with '/' concactenate '/' to current working directory
   * and then concactenate the user input to the current working directory.  Finally, copy this 
   * complete path to *path.*/
  else{
    strcat(cwd, "/");
    strcat(cwd, path);
    strcpy(path, cwd);
  }

  return path;
}


/*********************************************************************************************************
 * Function resolveAmp() returns a boolean indicating if "&" was the last argument of the user input.
 * The function sets the index following the last argument to NULL.
 * Parameters:  bool background - bool indicating if "&" - passed as false
 *              struct userInput* inputPtr - pointer to struct userInput
 *
 *  Return:  bool background
 *
 ********************************************************************************************************/
bool resolveAmp(bool background, struct userInput* inputPtr){

  background = false;
  //ampIndex is the number of arguments in the user input
  int ampIndex = inputPtr -> total_args;
  
  //when there is only one argument (command)
  if (ampIndex == 1){
    inputPtr -> args[1] = NULL;
  }
  // when there are only two arguments & the last argument is "&"
  else if (ampIndex == 2 && strcmp(inputPtr -> args[1], "&") == 0){
    inputPtr -> args[1] = NULL;
    if (allowBack) background = true;
    else background = false;
  }
  // for all other cases where the last argument is "&" 
  else if (strcmp(inputPtr -> args[ampIndex- 1], "&") == 0){
    inputPtr -> args[ampIndex - 1] = NULL;
    if (allowBack) background = true;
    else background = false;
    }
  else inputPtr -> args[ampIndex] = NULL;

  return background;

}

/*********************************************************************************************************
 * Desc: ioRedirection function searches for ">" and "<" within the args and ensures that they are followed
 * by a filename.  The function then routs the stdin to and the stdout from the applicable files.  This
 * function is also responsible, for routing the stdin/stdout for background cases.  In case there is a 
 * redirection, execvp is used to execute the command within this function.
 *
 *  Parameters:  inputPtr : pointer to command structure containing user input and parsed data from that input
 *               background:  boolean for background process, true menaing background process
 *               cProcessArr: pointer to int array containg the runnig child processes
 *  Returns:  boolean, true if execp was already executed ( meaning either redirection was done or it was a 
 *            background process) false otherwise.
 *
 *
 * NOTE:  If background is true (meaning the command has "&" at the end, the input / output is redirected to
 * "/dev/null".
 * NOTE:  cProcessArr only passed so it can be freed during a file opening (source or target) error or
 * during execvp execution error.
 * *******************************************************************************************************/

bool ioRedirection(struct userInput* inputPtr, bool background, int* cProcessArr){
  bool target = false;
  bool source = false;
  bool returnValue;
  int targetFD;
  int sourceFD;
  int targetIndex = -1;
  int sourceIndex = -1;
  int i = 0;
  char* targetFile = malloc(100 * sizeof(char));
  char* sourceFile = malloc(100 * sizeof(char));
  char** args = inputPtr -> args;

  // while loop to search throught the args to locate ">" and "<" and see if input and
  // output files exist.
  while(args[i] != NULL){
    if (strcmp(args[i], ">") == 0 && args[i + 1] != NULL){
      target = true;
      targetIndex = i;
      strcpy(targetFile, args[i + 1]);
    }
    if (strcmp(args[i], "<") == 0 && args[i + 1] != NULL){
      source = true;
      sourceIndex = i;
      strcpy(sourceFile, args[i + 1]);
    }
    i++;
  }

  // if no source or target and background is false, set returnValue to false
  if (targetIndex == -1 && sourceIndex == -1 && !background){
    free(targetFile);
    free(sourceFile);
    returnValue = false;
    goto skip;
  }

  // open appropriate sourceFile
  if (sourceIndex == -1 && background){
    sourceFD = open("/dev/null", O_RDONLY);
  }
  else if (source){
    sourceFD = open(sourceFile, O_RDONLY);
  }

  // error catching block for error opening source file
  if (source && sourceFD == -1){
    fprintf(stderr, "cannot open %s for input\n", sourceFile);
    fflush(stdout);
    free(sourceFile);
    free(targetFile);
    free(args);
    free(inputPtr -> termInput);
    free(inputPtr);
    free(cProcessArr);
    exit(1);
  }
  
  // open appropriate targetFile
  if (targetIndex == -1 && background){
    targetFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  }
  else if (target){
    targetFD = open(targetFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  }

  //error catching block for error opening target file
  if (target && targetFD == -1){
    fprintf(stderr, "cannot open %s for output\n", targetFile);
    fflush(stdout);
    free(targetFile);
    free(sourceFile);
    free(args);
    free(inputPtr -> termInput);
    free(inputPtr);
    free(cProcessArr);
    exit(1);
  }

  //replace stdin w/ sourceFD
  if(sourceIndex != -1 || background){
    if (!background) args[sourceIndex] = NULL;
    dup2(sourceFD, STDIN_FILENO);
  }
  //replace stdout w/ targetFD
  if (targetIndex != -1 || background){
    if (!background) args[targetIndex] = NULL;
    dup2(targetFD, STDOUT_FILENO);
  }

  // free sourceFile and targetFile
  free(sourceFile);
  free(targetFile);

  //when no target or source set returnValue to false
  if (sourceIndex == -1 && targetIndex == -1){
    returnValue = false;
    goto skip;
  }
  
  // run execvp when either target or source exists
  int resExecvp = 0;
  resExecvp = execvp(inputPtr -> command, inputPtr -> args);
   
  //free args (inputPtr -> args) after running execvp
  free(args);

  // error catching block for execvp
  if (resExecvp < 0){
    fprintf(stderr, "%s: no such file or directory\n", inputPtr -> command);
    fflush(stderr);
    free(inputPtr -> termInput);
    free(inputPtr);
    free(cProcessArr);
    exit(1);
  }
  // set returnValue to true if either source or targett
  if (target || source) returnValue = true;
  else returnValue = false;

skip:
  return returnValue;

}

