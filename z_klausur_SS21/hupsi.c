#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CMD_MAX 100

static void die(const char* msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

static void usage(void) {
  fprintf(stderr, "Usage: hupsi <n>\n");
  exit(EXIT_FAILURE);
}

struct process {
  pid_t pid;


  //Eigene Mitgelieder
  char* cmd;
  time_t time;
  struct process* next;

};

// Makros, Funktionsdeklarationen globale Variablen
pid_t run(char* cmd);
void waitProcess(void);

struct process* header = NULL;
static int limit = 0;


// Function main
int main(int argc, char** argv) {

  // check the number of arguments
  if(argc != 2) {
    usage();
  }

  // get the process limit number n
  errno = 0;
  char* endStr;
  long x = strtol(argv[1], &endStr, 10);
  // errno
  if(errno != 0) die("strtol");

  //argv[1] consists of non-number   ODER    argv[1] contains any char
  if(str == endStr || *endStr != '\0') {
    fprintf(stderr, "bad argv[1]");
    exit(EXIT_FAILURE);
  }
  if(x < 0 || x > INT_MAX) {
    fprintf(stderr, "argv[1] negative or too big");
    exit(EXIT_FAILURE);
  }

  limit = (int)x;


  // read file per line via stdin
  char buf[CMD_MAX+1];

  while(fgets(buf, sizeof(buf), stdin)) {

    // before running next cmd, collect
    waitProcess();

    // check line too short
    if(strlen(buf) <= 1) {
      continue;
    }

    while(1) {
      if(limit == 0) {
        waitProcess();
      } else {
        break;
      }
    }

    // duplicate the buf
    if(char* job = strdup(buf) == NULL) die("strdup");

    // create a new process
    pid_t pid = fork();
    if(pid == -1) die("fork");
    limit--;
    if(pid == 0) {
      //child process
      run(job);

    } else {
      // parent process, insert the process into linked list
      // create a node
      struct process* x = (struct process*)malloc(sizeof(struct process));
      if(x == NULL) die("malloc");


      x->pid = pid;
      x->cmd = job;
      x->next = NULL;

      // if header is NULL
      if(header == NULL) {
        header = x;
      } else {
        // set the new node as header
        struct process* y = header;
        x->next = y;
        header = x;
      }
    }
  }

  if(ferror(stdin)) {
    die("fgets");
  }
}


// Ende Funktion main


// Funktion run
static pid_t run(char* cmdline) {
  char* cmdPara[CMD_MAX];
  int index = 0;
  cmdPara[index] = strtok(cmdline, " ");
  index++;

  while((cmdPara[index] = strtok(NULL, cmdline)) != NULL) {
    index++;
  }

  //find this node in the list and update the time.
  struct process* y = header;
  while(y->pid != getpid()) {
    y = y->next
  }

  // set time on the node.
  y->time = time(NULL);

  execvp(cmdPara[0], cmdPara);
  die("execvp");

  return getpid();

}
// Ende Funktion run


//
static void waitProcess(void) {

pid_t pid;
int status;
errno = 0;


while(1) {

//auf beliebigen Process warten
pid = waitpid(-1, &status, WNOHANG);

if(pid == 0) break;

if(pid < 0) {
  if(errno == ECHILD) {
    break;
  } else {
    die("waitpid");
  }
}





}



// struct process zu PID aus Processliste ermitteln(you know which pid is terminated and should be kicked out of the list)


// (Vorbereiten der) Ausgabe


// Process aus Liste entfernen & Speicherfreigeben




}
//Ende Funktion waitProcess
