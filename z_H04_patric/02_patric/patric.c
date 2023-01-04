/*
T00.25

Shipan Liu  108019201553
Minhua Liu  108020210282

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>


#include "triangle.h"
#include "sem.h"

#define MAX_LINE_SIZE 100

// for the general atomic operation
static SEM *sem0;
static SEM *sem1;
// controll if the max number of thread exceeds
static SEM *sem2;
// for the atomic operation of the threadNumber
static SEM *sem3;
// for max_num threads
static SEM *sem4;
//maxmal thread
static int max_num;

// boundary and interior
static int bnodeSum;
static int inodeSum;
//the sum of nodes in a triangle
static int nodeSum;
//the thread number
static int threadNumber = 0;
//the closed thread number
static int closedThreadNumber = 0;

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
  bnodeSum += boundary;
  inodeSum += interior;
  nodeSum += (boundary + interior);
}


static void *outputThreadFunction(void *a) {
  errno = pthread_detach(pthread_self());
  if(errno != 0) {
    die("pthread_detach");
  }

  P(sem2);
  fprintf(stdout, "the boundary node number: %d\n", bnodeSum);
  fprintf(stdout, "the interior node number: %d\n", inodeSum);
  fprintf(stdout, "the thread number is: %d\n", threadNumber);
  fprintf(stdout, "the closed thread number is %d\n", closedThreadNumber);
  fprintf(stdout, "================================================\n");
  bnodeSum = 0;
  inodeSum = 0;
  nodeSum = 0;

  max_num++;
  V(sem4);
  threadNumber--;
  closedThreadNumber++;

  V(sem0);


  // the main function can continue
  if(threadNumber == 0) {
    V(sem3);
  }
  return NULL;

}








// the thread function in pthread_create()
static void *threadFunction(void *a) {

  // passive wait
  errno = pthread_detach(pthread_self());
  if(errno != 0) {
    die("pthread_detach");
  }

  threadFunction_arg* arg = (threadFunction_arg*)a;


  P(sem1);
  P(sem0);

  countPoints(arg->tri, arg->callback);
  threadNumber--;
  closedThreadNumber++;
  // the output thread can start
  V(sem2);
  max_num++;
  V(sem4);

  V(sem1);
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
  sem0 = semCreate(1);
  if(sem0 == NULL) {
    die("semCreate");
  }

  sem1 = semCreate(1);
  if(sem1 == NULL) {
    die("semCreate");
  }

  sem2 = semCreate(0);
  if(sem2 == NULL) {
    die("semCreate");
  }

  sem3 = semCreate(0);
  if(sem3 == NULL) {
    die("semCreate");
  }

  sem4 = semCreate(1);
  if(sem4 == NULL) {
    die("semCreate");
  }

  // open file "16boundary-13interior"
  FILE* file = fopen("578boundary-643interior", "r");
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

    // get rid of the \n at the end
    lineBuffer[strlen(lineBuffer)-1] = '\0';

    //!!!test
    // printf("line: %s\n", lineBuffer);

    // check the form
    int arr[6];
    int result = sscanf(lineBuffer, "(%d,%d),(%d,%d),(%d,%d)", &arr[0],&arr[1],&arr[2],&arr[3],&arr[4],&arr[5]);
    // printf("result: %d\n", result);


    if(6 != result) {
      warn("bad line\n");
      continue;
    }



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
    P(sem4);
    pthread_t tid;
    errno = pthread_create(&tid, NULL, &threadFunction, &arg);
    if(errno != 0) {
      die("pthread_create");
    }
    threadNumber++;
    max_num--;

    if(max_num > 0) {
        V(sem4);
    }

    P(sem4);
    pthread_t tid_output;
    errno = pthread_create(&tid_output, NULL, outputThreadFunction, NULL);
    if(errno != 0) {
      die("pthread_create");
    }
    threadNumber++;
    max_num--;

    if(max_num > 0) {
        V(sem4);
    }
  }


  // now the main function can continue
  P(sem3);
  printf("finished!\n");

  // close file
  if(EOF == fclose(file)) {
    die("fclose");
  }

  semDestroy(sem0);
  semDestroy(sem1);
  semDestroy(sem2);
  semDestroy(sem3);
  semDestroy(sem4);

  exit(EXIT_SUCCESS);
}