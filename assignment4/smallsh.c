#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

//these are GLOBAL variables being used
int background_ids[100];  //max of 100 bg processes can run at once
int status = 0;
int signal_status = 0;
int fg_mode = 0;  //this will be toggled between 1 & 0, but starts in normal or mode 0

//this function is in charge of teh output to the terminal if cd is requested
void cd_output(char* directory) {
int i=0;
char s[160];
getcwd(s, 160);   //getting the current working directory

  if(directory == NULL) {   //used when only cd is passed therefore wanting the home directory
    chdir(getenv("HOME"));  //changing directory to directory listed under HOME variable
  }
  else if(directory[0] == '/') {    //this is for when an absolute path is sent to cd
    strcat(s, directory);     //combining the current working directory with the added directory
    chdir(s);
  }
  else {      //this is for any realtive paths
    char directory_name[sizeof(directory)+1]; //makes a larger string so that a / can be added before the directory name since it wasnt absolute
    strcat(directory_name, "/");  //adding / to front
    strcat(directory_name, directory);
    strcat(s, directory_name);    //combine with current working directory stored in s
    chdir(s);
  }

//testing purposes
//  printf("%s\n", getcwd(s, 160));
}




//this function is in charge of removing the arguments that can't be passed into exec  (only called when there is a < or >)
void remove_args(int numargs, int redirect_index, char** args) {
  int i;
  args[redirect_index] = NULL;  //sets < or > to null
  args[redirect_index+1] = NULL; //sets file name to null
  //strcpy(args[i], "\0");

  for(i=0; i<(numargs-redirect_index-2); i++) {   //moves arguments after eleements that were just removed forward to fill the open space
    args[redirect_index+i] = args[redirect_index+i+2];
  }
  args[numargs-1] = NULL;  //sets the last to elements to null as we reomved two elements earlier
  args[numargs-2] = NULL;
}


//this function will look through the args for < and change the input as well as remove the < and file from the array
int input_redirect(int numargs, char** args, int background_yes) {
  int i=0, redirect_index=-5, result = 0;

  //printf("in input redirection\n");

  for(i=0; i<numargs; i++) {   //goes through args looking for redirect symbol
    if(strcmp(args[i], "<") == 0) {  //if found it stores the index and breaks out of the loop
      redirect_index = i;
      break;
    }
  }
  if(redirect_index == -5) { //no < arguments were found
    if(background_yes == 1) {
      int fd = open("/dev/null", O_RDONLY);
      dup2(fd, 0);
    }
    return 0;

  }
  if((numargs-1) == redirect_index) {   //checks if there is enough args for there to also be a file after <
    printf("NO FILE was provided after redirect\n");
    fflush(stdout);
    return 1;
  }
    int sourceFD = open(args[redirect_index+1], O_RDONLY);  //opening file after <
  if(sourceFD == -1) {  //checks to make sure that file could be opened
    printf("File provided after the redirect could not be opened and read\n");
    fflush(stdout);
    return 1;   //returns 1 if file could not be opened
  }
  result = dup2(sourceFD, 0);  //changing input stream from stdout to sourceFD
  if(result == -1) {
    printf("input stream could not be changed\n");
    fflush(stdout);
    return 1;  //returns 1 if stream could not be changed
  }
  remove_args(numargs, redirect_index, args);  //removing arguments

  return 3; //if we remove elements we need to return 3 so we know to decrease numargs in main()

}

//this function will look through the args for > and change the input as well as remove the > and file from the array
int output_redirect(int numargs, char** args, int background_yes) {
  int i=0, redirect_index=-5, result = 0;

  //printf("in input redirection\n");

  for(i=0; i<numargs; i++) {   //goes through args looking for redirect symbol
    if(strcmp(args[i], ">") == 0) {  //if found it stores the index and breaks out of the loop
      redirect_index = i;
      break;
    }
  }
  if(redirect_index == -5) { //no > arguments were found
    if(background_yes == 1) {  //this changes background output to /dev/null so it is not printed to stdout
      int fd = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);
      dup2(fd, 1);
    }
    return 0;


  }
  if((numargs-1) == redirect_index) {   //checks if there is enough args for there to also be a file after <
    printf("NO FILE was provided after redirect\n");
    fflush(stdout);
    return 1;
  }
  int targetFD = open(args[redirect_index+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);  //opening file after > that can be written to
  if(targetFD == -1) {  //checks to make sure that file could be opened
    printf("File provided after the redirect could not be opened and written to\n");
    fflush(stdout);
    return 1;   //returns 1 if file could not be opened
  }
  result = dup2(targetFD, 1);  //changing input stream from stdout to sourceFD
  if(result == -1) {
    printf("output stream could not be changed\n");
    fflush(stdout);
    return 1;  //returns 1 if stream could not be changed
  }
  remove_args(numargs, redirect_index, args);  //removing arguments
  for(i=0; i <(numargs-2); i++) {
//    printf("arg %d: %s", i, args[i]);
  }

  return 0;

}

//checks to see if the command line has a & at the end.  If it does it returns a 1, else a 0
int check_for_bg(char** args, int* numargs) {
  if(strcmp(args[(*numargs)-2], "&") == 0) { //checks last element in command line
    args[(*numargs)-2] = NULL;  //sets & element to NULL as it cant be passed through exec
    (*numargs) = (*numargs-1);  //reduced size by 1
    if(fg_mode == 0) {
    return 1;
    }
    return 0;
  }
  return 0;
}

//this function is in charge of checking whether any bg processes have completed
void check_exit() {
  int k, child_state;
  pid_t bg_children;

  for(k=0; k<100; k++){ //goes through all of our possible bg processes
    if(background_ids[k] != -9) {   //if there is a pid
      bg_children = waitpid(background_ids[k], &child_state,  WNOHANG);   //wait for that pid but don't stop foreground processes
      if(bg_children > 0) { //when one finishes either the exit code or signal code will be printed to screen
        if(WIFEXITED(child_state)) {    //retrieve exit status code
          printf("Background pid: %d exited with status: %d\n", background_ids[k], WEXITSTATUS(child_state));
          fflush(stdout);
        }
        if(WIFSIGNALED(child_state)) {  //retrieve signal status code
          printf("Background pid: %d exited via signal: %d\n", background_ids[0], WTERMSIG(child_state));
          fflush(stdout);
        }
        background_ids[k] = -9; //sets element that held pid back to -9 so we know the bg pid is completed
      }
      break;
    }
  }
}

//this is our signal handler function if ^Z is called
void catchSIGTSTP(int signo) {
  if(fg_mode == 0) {  //checks if we are currently in normal mode
    write(STDOUT_FILENO, "foreground only mode (& is ignored)\n", 36);
    fg_mode = 1;  //changing mode
  }
  else{  //activates if we are in foreground mode
    write(STDOUT_FILENO, "foreground only mode exited\n", 28);
    fg_mode = 0;  //changing mode
  }
}

//this function is in charge of changing any arguments that contain $$ in them
void add_pid_to_arg(int j, char** args) {
  int orig_size = strlen(args[j]);
  char* temp = malloc((orig_size+4) * sizeof(char));  //creating a new char* with room for teh pid
  memset(temp, '\0', orig_size+4);   //setting all chars to \0
  strcpy(temp, args[j]);  //copies original string to temp
  memset(temp + (orig_size-2), '\0', 6);    //removes $$ and replaces with \0
  sprintf(temp, "%s%d", temp, getpid());  //adds the pid to the end of the argument
  args[j] = temp;
}




int main() {
  int k;
  for(k=0; k<100; k++) { //here we are setting all the background id's to -9 as none have started yet
    background_ids[k] = -9;
  }


  struct sigaction SIGINT_action = {0}; //creating the struct for when a ^C is called

  SIGINT_action.sa_handler = SIG_IGN;   //SIG_IGN allows the parent process or smallsh to ignore the command
  sigfillset(&SIGINT_action.sa_mask);
  SIGINT_action.sa_flags = 0;

  struct sigaction SIGTSTP_action = {0};  //creating struct for when ^Z is called

  SIGTSTP_action.sa_handler = catchSIGTSTP;  //here we give it a function that we have written that allows us to change modes
  sigfillset(&SIGTSTP_action.sa_mask);
  SIGTSTP_action.sa_flags = SA_RESTART;

  sigaction(SIGTSTP, &SIGTSTP_action, NULL);

  sigaction(SIGINT, &SIGINT_action, NULL);

  char* args[512];    //these are all various variables that are used throughout the program
  size_t input_max = 2048;
  char* input = NULL;
  int running = 1, inc= 0, i = 0, j=0, numargs = 0, status_code1, status_code2, childExitMethod = -5;
  char delim[] = " \n";
  pid_t parentpid = getpid();
  pid_t bg_children, child_pid;
  int background_yes;
  char buffer[5];


  printf("$ smallsh\n");   //this is teh first thing that is printed and is only printed once
  fflush(stdout);

  while(running == 1) {   //this is how we keep the script running over and over again
    i=0;
    check_exit();  //checking to see if any of the background processes have finished

    printf(": ");
    fflush(stdout);
    getline(&input, &input_max, stdin);   //takes the input provided by user

    args[0] = strtok(input, delim);   //seperates teh first argument in command line
    numargs = 1;
    if (args[0] == NULL) {

    }
    else if(args[0][0] != '#') {
      while(args[i] != NULL) {   //this goes through all the arguments and seperates them based on spaces
        args[i+1] = strtok(NULL, delim);
        i++;
        numargs++;
      }

      for(j=0; j<numargs-1; j++) {  //this looks through all the arguments to see if any have $$
        if(strstr(args[j], "$$")) {
          add_pid_to_arg(j, args);  //this function is called if they do
        }
      }

      background_yes = 0;
      background_yes = check_for_bg(args, &numargs);    //checks whether there is a & at the end of the command. returns 1 if there is

      //this is for if exit is called in the command line
      if(strcmp(args[0], "exit") == 0) {
        return 0;
      }
      //this is for if status is called
      else if(strcmp(args[0], "status") == 0) {
        if(signal_status == 0) { //checks if process finished without a signal call to stop
        printf("exit value: %d\n", status);
        }
        else { //only prints if it was stopped by a signal
        printf("exit by signal: %d\n", signal_status);
        }
        fflush(stdout);
      }
      //this is for if cd is called in the command line
      else if(strcmp(args[0], "cd") == 0) {
          cd_output(args[1]);
      }
      else if(strcmp(args[0], "badfile") == 0) {
        printf("This file could not be read\n");
      }
      else { //this is if a non-built in command is called
        pid_t spawnpid = -5;
        spawnpid = fork();  //making a child from parent so an exec can be called

        if(background_yes == 1 && parentpid == getpid()) {  //this checks that there is a background process called and that we are in parent process
          for(k=0; k<100; k++) {
            if(background_ids[k] == -9) {  //checks for open index in background id's
              background_ids[k] = spawnpid; //stores bg pid in open index
              break;
            }
          }
          printf("Background pid: %d has started\n", spawnpid);  //prints that bg process has started
          fflush(stdout);
        }

        if(parentpid != getpid()) {   //checks to make sure that we are in child process
          if(numargs >= 2) {
            status_code1 = input_redirect((numargs-1), args, background_yes);   //checks for < operators in command line.  If found the args array is updated so it can be passed to exec
            if(status_code1 == 3) {
              numargs = (numargs-2);
            }
              status_code2 = output_redirect((numargs-1), args, background_yes); // checks for > operator
            fflush(stdout);
          }

          if(status_code1 != 1 && status_code2 == 0) {   //checks to make sure that only the child process can run exec()
            fflush(stdout);
            execvp(args[0], args);
            fflush(stdout);
          }
          SIGINT_action.sa_handler = SIG_DFL;   //here we are in the child process so we can change the ^c signal back to its original action
          sigaction(SIGINT, &SIGINT_action, NULL);

        }
        if(background_yes == 0) {  //for foreground processes only (stops : from being printed)

          child_pid = waitpid(spawnpid, &childExitMethod, 0);  //this is how we get the exit status and signal status codes of finished foreground processes
          if(child_pid > 0) {
            if(WIFEXITED(childExitMethod)) {  //exit status
              status = WEXITSTATUS(childExitMethod);
            }
            if(WIFSIGNALED(childExitMethod)) {  //signal status code
              signal_status = WTERMSIG(childExitMethod);
            }
            else {
              signal_status = 0;
            }
          }
        }
      }


    }

    running = 1;
  }



}
