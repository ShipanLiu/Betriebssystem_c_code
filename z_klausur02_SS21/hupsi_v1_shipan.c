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
  char* command;
  time_t start_time;
  time_t end_time;
  struct process* next;

};

static pid_t run(char*);
static void waitProcess(void);

static struct process* process_list_header;
static long procs = 0;

int main(int argc, char** argv) {

  char* end;
  errno = 0;
  long max_proc = strtol(argv[1], &end, 10);

  if(errno != 0) die("strtol");
  if(max_proc <= 0 || argv[1] == end || *end != '\0') {
    usage();
  }


  char buf[CMD_MAX + 2];
  while(fgets(buf, sizeof(buf), stdin) != NULL) {
    // before executing the new cmd line, check if there are free process left.
    if(procs == max_proc) {
      // collect zombies and decrease process number
      waitProcess();
      // proc--;
    }

    // create a new process node for each cmd line
    struct process *proc = calloc(1, sizeof(struct process));
    if(proc == NULL) die("calloc");

    // remove \n
    buf[strlen(buf)-1] = '\0';
    proc->command = strdup(buf);
    if(proc->command == NULL) die("strdup");
    proc->start_time = time(NULL);
    // we assume run will not have any error
    proc->pid = run(buf);

    // insert into the list as the first element;
    proc->next = process_list_header;
    process_list_header = proc;

    procs++;
  }

  if(ferror(stdin)) die("fgets");

  // collect zombies
  while(0 < procs) {
    waitProcess();
    //proc--
  }

  exit(EXIT_SUCCESS);
}


static pid_t run(char* command) {

  // we assume the "command" are all stardard good, so we don't have to check if the "cmd" is too short or something...

  pid_t pid = fork();

  if(pid == -1){
    die("fork");
  }else if(pid == 0) {
    //split the "cmmand"
    int index = 0;
    char* paraArr[CMD_MAX/2];
    paraArr[index] = strtok(command, " ");
    index++;

    while((paraArr[index] = strtok(NULL, " ")) != NULL) {
      index++;
    }

    //run the cmd
    execvp(paraArr[0], paraArr);
    //if something wrong, this process will be ended, so now set end_time
    // step1, find this node in the process list
    struct process* node =process_list_header;
    while(node->pid != pid) {
      node = node->next;
    }
    // node found! now set the endtime to the node.
    node->end_time = time(NULL);
    die("execvp");
  }

  // father process is only used to return pid
  return pid;
}



static void waitProcess(void) {
  // here we wait any one zombie
  pid_t pid;
  int status;
  errno = 0;
  pid = waitpid(-1, &status, WNOHANG);

  // if pid == 0, this means this kindprocess is still alive
  if(pid == 0) {
    return;
  }

  // if pid < 0, the errno is either "ECHILD" or other errors
  if(pid < 0) {
    // if there is no kindprocess for me to terminate
    if(errno == ECHILD) {
      return;
    } else {
      die("waitpid");
    }
  }

  // find this node in the process list and update the end_time
  // step1, find this node in the process list
  struct process* node =process_list_header;
  while(node->pid != pid) {
    node = node->next;
  }
  // node found! now set the endtime to the node.
  node->end_time = time(NULL);

  // check if this process is excited or not
  if(WIFEXITED(status)) {
    // exit with success
    // now waitpid terminates a kindprocess successfully
    procs--; // the main process can create one more new process
    // output
    printf("pid:[%d], cmd:[%s], Exitstatus[%d], Dauer:[%ld]\n", (int)pid, node->command, WEXITSTATUS(status), (long)(node->end_time - node->start_time));
    return;
  } else {
    // exit without success
    return;
  }

}
