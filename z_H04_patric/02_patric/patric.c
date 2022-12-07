#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>


#include "triangle.h"
#include "sem.h"

#define MAX_LINE_SIZE 20

// for the general atomic operation
static SEM* sem1;
// controll if the max number of thread exceeds
static SEM* sem2;
//maxmal thread
static unsigned int max_num;

typedef struct threadFunction_arg {
  struct triangle* tri;
  void *callback;
}threadFunction_arg;

static void die(char* msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

static void warn(char* msg) {
  fputs(msg, stderr);
}


static int checkLine(char* str) {
  // copy the str for later compare
  char* strCpy = strdup(str);
  if(strCpy == NULL) {
    die("strdup");
  }
  //check if space exists or not
  char *afterToken = strtok(str, " \t");
  //if there exists space, then return 0
  return strcmp(afterToken, strCpy) == 0 ? 1 : 0;
}


// convert a line to the format of argument for function threadFunction()
static struct triangle convertLineToStructTriangle(char* str) {
    int arr[6];
    int index = 0;
    sscanf(&(strtok(str, ",")[1]), "%d", &arr[index]);
    printf("%c\n", arr[index]);
    index++;
    for(int i=1; i<=5; i++) {
        if(i%2 != 0) {
            sscanf(&(strtok(NULL, ",")[0]), "%d", &arr[index]) ;
            // printf("%d\n", arr[index]);
        } else {
            sscanf(&(strtok(NULL, ",")[1]), "%d", &arr[index]) ;
            // printf("%d\n", arr[index]);
        }
        index++;
    }

    struct triangle tri;

    for(int i=0; i<3; i++) {
      struct coordinate cordi;
      cordi.x = arr[2*i];
      cordi.y = arr[2*i+1];
      tri.point[i] = cordi;
    }

    // return a struct
    return tri;

}


// callback function
static void callback(int boundary, int interior) {

}


// the thread function in pthread_create()
static void *threadFunction(void *a) {

  // passive wait
  errno = pthread_detach(pthread_self);
  if(errno != 0) {
    die("pthread_detach");
  }

  threadFunction_arg* arg = (threadFunction_arg*)a;

  countPoints(arg->tri, arg->callback);

}



int main(int argc, char* argv[]) {
  // check argument number
  if(argc != 2) {
    fputs("parameter number wrong", stderr);
    exit(EXIT_FAILURE);
  }

  max_num = argv[1];

  // create SEM
  sem1 = semCreate(1);
  if(sem1 == NULL) {
    die("semCreate");
  }

  sem2 = semCreate(0);
  if(sem2 == NULL) {
    die("semCreate");
  }

  // open file "16boundary-13interior"
  FILE* file = fopen("16boundary-13interior", "r");
  if(file == NULL) {
    die("fopen");
  }

  // read each line
  while(1) {
    char lineBuffer[MAX_LINE_SIZE];
    if(NULL == fgets(lineBuffer, sizeof(lineBuffer), file)) {
      if(ferror(file)) {
        die("fgets");
      } else {
        // read EOF
        break;
      }
    }
    // reset the errno before creating the thread
    errno = 0;

    // get rid of the \n at the end
    lineBuffer[sizeof(lineBuffer)-1] = '\0';

    //create the thread for each line
    pthread_t tid;

    // create args for the threadFunction
    struct triangle tri = convertLineToStructTriangle(lineBuffer);

    //create
    threadFunction_arg arg;
    arg.tri = &tri;
    arg.callback = callback;


    errno = pthread_create(&tid, NULL, &threadFunction, &arg);

  }










  // close file
  if(EOF == fclose(file)) {
    die("fclose");
  }





  // read file in while-loop + checkLine + checkTriangle + create Thread.



  exit(EXIT_SUCCESS);
}