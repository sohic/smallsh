#include "smallsh.h"

//signal setup per course modules with slight alterations


// SIGINT handler
void handle_SIGINT(__attribute__((unused)) int signo){
  char * message;
  message = "terminated by signal 2\n";
  write(STDOUT_FILENO, message, 23);
  fflush(stdout);
}

//SIGTSTP handler
void handle_SIGTSTP(__attribute__((unused)) int signo){
   char * message;
   char * message2;

   //wait for foreground process to finish before printing message
   if (!background) spawnPid = waitpid(spawnPid, &childStatus, 0);

   message2 = ": ";
   //check to see if background processes are allowed
   if (!allowBack) {
     message = "\nExiting foreground-only mode\n";
     write(STDOUT_FILENO, message, 30);
     allowBack = true;
   }
   else {
     message = "\nEntering foreground-only mode (& is now ignored)\n";
     write(STDOUT_FILENO, message, 50);
     allowBack = false;
   }
   //check if signal interupted a child process
   if (!childAlert) write(STDOUT_FILENO, message2, 2);
   childAlert = false;
   fflush(stdout);
  }

//setup function for SIGINT
void SIGINTsetup(int activate){
  if (activate == 0) SIGINT_action.sa_handler = SIG_IGN;
  else SIGINT_action.sa_handler = handle_SIGINT;
  sigfillset(&SIGINT_action.sa_mask);
  SIGINT_action.sa_flags = 0;
  sigaction(SIGINT, &SIGINT_action, NULL);
}

//setup function for SIGTSTP
void SIGTSTPsetup(int activate){
  if (activate == 0) SIGTSTP_action.sa_handler = SIG_IGN;
  else SIGTSTP_action.sa_handler = handle_SIGTSTP;
  sigfillset(&SIGTSTP_action.sa_mask);
  if (activate != 0) SIGTSTP_action.sa_flags = SA_RESTART;
  sigaction(SIGTSTP, &SIGTSTP_action, NULL);
}
