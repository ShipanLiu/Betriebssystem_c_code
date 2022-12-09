/*
T00.25

Shipan Liu  108019201553
Minhua Liu  108020210282

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>



#include "plist.h"

#define MAX_COMMAND_SIZE 1337

static void myKill(char* msg) {
  perror("msg");
  exit(EXIT_FAILURE);
}

static void myWarn(char* msg) {
  fputs(msg, stderr);
}


// 01-print the current path
static void printPath() {
  // set errno to 0
  errno = 0;

  // define a buffer for saving the path
  size_t pathSize = 500;
  char* pathBuffer = (char*)malloc(pathSize*sizeof(char*));
  if(pathBuffer == NULL) {
    myKill("malloc on pathBuffer");
  }

  // now get the path
  while(!getcwd(pathBuffer, pathSize)) {
    // if the error is because of size problem
    if(errno == ERANGE) {
      pathSize *= 2;
      pathBuffer = (char*)realloc(pathBuffer, pathSize*sizeof(char*));
      if(pathBuffer == NULL) {
        myKill("realloc on pathBuffer");
      }
    } else {
      // if the error is not because of size problem
      myKill("getcwd");
    }

  }

  // put the path in stdout
  if(EOF == fputs(pathBuffer, stdout)) {
    myKill("fputs");
  }

  printf(":");

  if(EOF == fflush(stdout)) {
    myKill("fflush");
  }

  free(pathBuffer);
}

// 02-print all the jobs, work with the walklist()
static int printJobs(pid_t pid, const char* str) {
  printf("pid: %d, job: %s\n", (int)pid, str);
  return 0;
}

// 03-collect zombies
static void collectZombies() {
  // terminate the zomblies
  pid_t pid;
  int status;
  errno = 0;

  // you don't know how many zombies, so use while
  //-1: targets are all child processes
  // WNOHANG:
  while(1) {
    pid = waitpid(-1, &status, WNOHANG);
    // pid = 0 : the zombie(child) still alive
    // pid < 0 : error OR no zombies
    // pid > 0 : zombies terminated

    if(pid == 0) {
    // pid = 0 : the zombie(child) still alive, 这个 在 写的时候 忘记了
      break;
    }

    if(pid != 0) {
      if(pid < 0) {
        //The calling process has no existing unwaited-for child processes or the the all the zomblies are finished collected
        if(errno == ECHILD) {
          break;
        }else {
          myKill("waitpid");
        }
      }
    }

    // the job in the plist was already removed the '\n'
    char job[MAX_COMMAND_SIZE + 1];  // 因为有 '\0' 在最后

    if(removeElement(pid, job, sizeof(job)) < 0) {
      // pid not found or plist is empty
      continue;
    }

    //print status
    if(WIFEXITED(status)) {
      // exit with success
      printf("Exitstatus [%s] = %d\n", job, WEXITSTATUS(status));
    } else {
      printf("[%s] did not exit with success", job);
    }
  }
}


int main() {
  // input the command
  char inputBuffer[MAX_COMMAND_SIZE + 1];
  int isBackground = 0;

  while(1) {

    //reset isBackground
    isBackground = 0;

    //collect zombies before printing the path
    collectZombies();


    printPath();

    if(EOF == fflush(stdin)) {
      myKill("fflush stdin");
    }


    if(NULL == fgets(inputBuffer, MAX_COMMAND_SIZE + 1, stdin)) {
      if(ferror(stdin)) {
        myKill("stdin");
      } else {
        // no character read / input EOF, terminate clash
        break;
      }
    }

    // check if too short, only '\n'
    if(strlen(inputBuffer) <= 1) {
      continue;
    }

    // check if too long
    if(strlen(inputBuffer) == MAX_COMMAND_SIZE && inputBuffer[MAX_COMMAND_SIZE-1] != '\n') {

      myWarn("input too long");

      // clean the char left in the buffer
      int c;
      do{
        c = fgetc(stdin);
      }while(c != '\n' && c != EOF);

      // we have to guarantee that fgetc does his job well so that the buffer will be cleaned for next time input
      if(ferror(stdin)) {
        myKill("stdin");
      }
      continue;
    }

    // remove '/n'
    inputBuffer[strlen(inputBuffer)-1] = '\0';

    // check if background
    if(inputBuffer[strlen(inputBuffer)-1] == '&') {
      // remove &
      inputBuffer[strlen(inputBuffer)-1] = '\0';
      isBackground = 1;
    }

    // cpoy the command before splitting(for later inserting into the plist)
    char* job = strdup(inputBuffer);
    if(job == NULL) {
      myKill("strdup");
    }

    // now split the command
    int index = 0;
    char* paraArr[MAX_COMMAND_SIZE];
    paraArr[index] = strtok(inputBuffer, " \t");
    index++;

    // last parameter is given NULL
    while((paraArr[index] = strtok(NULL, " \t")) != NULL) {
      index++;
    }

    // check if command exists or not
    if(paraArr[0] == NULL) {
      continue;
    }

    // // !!!test
    // printf("sum of parameters: %d\n", index);
    // for(int i = 0; i <= index; i++) {
    //   printf("parameter: %s\n", paraArr[i]);
    // }

    // if the command is cd, then there should be only 2 paremeter: command + path
    if(strcmp("cd", paraArr[0]) == 0) {
      if(paraArr[1] == NULL || paraArr[2] != NULL) {
        // too less parameter OR too many parameterts
        myWarn("wrong number of parameters of cd");
      } else {
        // conduct cd command
        if(chdir(paraArr[1]) != 0) {
          myWarn("cd");
        }
      }
      // continue for new command input
      continue;
    }


    // if the command is jobs , then there should be only one parameter: jobs
    if(strcmp("jobs", paraArr[0]) == 0) {
      if(paraArr[1] != NULL) {
        myWarn("jobs");
      } else {
        // do walklist
        // void walkList(int (*callback) (pid_t, const char *))
        walkList(printJobs);
      }
      // continue for new input;
      continue;
    }

    // if the command is neither "cd" nor "jobs", we do that in child process
    pid_t pid = fork();

    if(pid < 0) {
      myKill("fork");
    } else if(pid == 0) {
      // here in child process
      execvp(paraArr[0], paraArr);
      myKill("execvp");
    }

    // here is parent process
    // if background: insert into pList + do not wait for terminating
    // because you did not wait() in the background mode, so you need to deal with the zombies.
    if(isBackground == 1) {
      if(-2 == insertElement(pid, job)) {
        // erroro in plist.c ist set but in this file(clash.c) not
        myWarn("malloc or strdup");
        exit(EXIT_FAILURE);
      }
    } else {
      // front : wait until the child terminate, there is no zombies in the front mode
      int status;
      if(waitpid(pid, &status, 0) < 0) {
        myKill("waitpid");
      }

      if(WIFEXITED(status)) {
        // exit with success
        printf("Exitstatus [%s] = %d\n", job, WEXITSTATUS(status));
      } else {
        printf("[%s] did not exit with success", job);
      }
    }
  }
  exit(EXIT_SUCCESS);

}
