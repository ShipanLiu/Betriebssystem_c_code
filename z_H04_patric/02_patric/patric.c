#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>


#include "triangle.h"
#include "sem.h"

#define MAX_LINE_SIZE 20

// for the general atomic operation
static SEM *sem1;
// controll if the max number of thread exceeds
static SEM *sem2;
//maxmal thread
static int max_num;

// define a function pointer for using in the following struct
typedef void (*callbackFunction)(int a, int b);

typedef struct threadFunction_arg {
  struct triangle* tri;
  callbackFunction callback;
}threadFunction_arg;

static void die(char* msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

static void warn(char* msg) {
  fputs(msg, stderr);
}


// callback function
static void callback(int boundary, int interior) {
  printf("callback is called\n");
  // printf("sum: %d\n", boundary + interior);
  //avoid printf functon
  int sum = boundary + interior;
  fprintf(stdout, "%d", sum);
}

// static void *fun(void *a) {
//   threadFunction_arg* arg = (threadFunction_arg*)a;
//   printf("x0: %d\n", arg->tri->point[0].x);
//   printf("y0: %d\n", arg->tri->point[0].y);

//   return NULL;
// }


// the thread function in pthread_create()
static void *threadFunction(void *a) {

  //!!!!!!!!!!test
  printf("in threadFunction: one thread created\n");

  // passive wait
  errno = pthread_detach(pthread_self);
  if(errno != 0) {
    die("pthread_detach");
  }

  threadFunction_arg* arg = (threadFunction_arg*)a;

  countPoints(arg->tri, arg->callback);
  printf("countPoints executed in threadFunction\n");

  return NULL;
}



int main(int argc, char* argv[]) {
  // check argument number
  if(argc != 2) {
    fputs("parameter number wrong", stderr);
    exit(EXIT_FAILURE);
  }

// get the maxmal number of threads
  sscanf(argv[1], "%d", &max_num);

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
    // errno = 0;

    // get rid of the \n at the end
    lineBuffer[strlen(lineBuffer)-1] = '\0';

    //!!!!!!!!!!test
    printf("the length of the line is: %d\n", (int)strlen(lineBuffer));
    printf("line: %s\n", lineBuffer);

    // check the form
    int arr[6];
    if(6 != sscanf(lineBuffer, "(%d,%d),(%d,%d),(%d,%d)", &arr[0],&arr[1],&arr[2],&arr[3],&arr[4],&arr[5])) {
      warn("bad line");
      break;
    }

    //!!!!!!!!!!test
    for(int i=0; i<6; i++) {
      printf("%d,", arr[i]);
    }
    printf("\n");

    struct triangle tri;

    for(int i=0; i<3; i++) {
      struct coordinate cordi;
      cordi.x = arr[2*i];
      cordi.y = arr[2*i+1];
      tri.point[i] = cordi;
    }

    //create the parameter for the thread function
    threadFunction_arg arg;
    arg.tri = &tri;
    // callback is a function pointer
    arg.callback = callback;

    //create the thread for each line
    pthread_t tid;
    errno = pthread_create(&tid, NULL, &threadFunction, &arg);
    printf("errno: %d\n", errno);
    printf("pthread_create should execute\n");
    pthread_join(tid, NULL);

  }










  // close file
  if(EOF == fclose(file)) {
    die("fclose");
  }





  // read file in while-loop + checkLine + checkTriangle + create Thread.



  // exit(EXIT_SUCCESS);
}