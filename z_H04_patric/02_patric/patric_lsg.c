/*

Answer from the tutorial

 */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include <sem.h>
#include <triangle.h>

struct state {
  int boundary;
  int interior;
  int active;
  int finished;
};

// create semaphore
static SEM* semLimit;
static SEM* semMutex;
static SEM* semNotify;

// 创建一个 outputState，相当于 一个容器， 盛放 各种 global variable。
static struct state outputState;
static volatile bool shutdown = false;

//for creating sem
static SEM* semOrDie(int initVal) {
  SEM *sem = semCreate(initVal);
  if(!sem) die("semCreate");
  return sem;
}

static void incVal(int* variable) {
  P(semMutex);
  *variable += 1;
  V(semMutex);
}

static void decVal(int* variable) {
  P(semMutex);
  *variable -= 1;
  V(semMutex);
}

static void die(char* msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

static void warn(char* msg) {
  fprintf(stderr, msg);
  exit(EXIT_FAILURE);
}

static SEM* semOrDie(int initVal) {
  SEM* sem = semCreate(initVal);
  if(!sem) die("semCreate");
  return sem;
}

static void outputCallback(int boundary, int interior) {
  outputState.boundary += boundary;
  outputState.interior += interior;
}

static void* workerThread(void* param) {
  struct triangle* w = (struct triangle*) param;
  errno = pthread_detach(pthread_self());
  if(errno != 0) die("pthread_create");

  countPoints(w, outputCallback);

  // 这个 output thread 的终结： synchronization
  decVal(&outputState.active);
  incVal(&outputState.finished);

  free(w);

  // main 主 thread 被允许 创建新的 thread。
  V(semLimit);

  return NULL;
}


// output thread function
static void* outputThread(void* param) {
  // we do not use param, so there is no cast
  errno = pthread_detach(pthread_self());
  if(errno != 0) die("pthread_detach");

  // 使用一个 while schleife, 从而不让 这个 thread 提前结束生命。
  // 这个while loop 是在所有的 thread 都 dead 之后 才 继续的。 由 shutdown 负责。 shutdown 的 值就是 在main 中设置的。
  // 注意 outputState.active 是不包括 这一个 ausgangthread 的。
  while(!shutdown) {
    // 因为 printf 是 langsam function， 所以用一下copy
    struct state copy = outputState;
    printf("\rFound %d boundary, %d interior points, %d active threads, %d finished threads",
    copy.boundary, copy.interior, copy.active, copy.finished);
    if(fflush(stdout) != EOF) die("fflush");
  }

  // last time print into the terminal and keep the information stay there
  printf("\rFound %d boundary, %d interior points, %d active threads, %d finished threads",
  outputState.boundary, outputState.interior, outputState.active, outputState.finished);

  return NULL;

}


static int parse_positive_int_or_die(char *str) {
    errno = 0;
    char *endptr;


    long x = strtol(str, &endptr, 10);
    if (errno != 0) {
        die("invalid number");
    }
    // Non empty string was fully parsed(就是 没有数字 or 有字符， 两种情况 都不符合要求， 我们要求给个 纯数字)
    if (str == endptr || *endptr != '\0') {
        fprintf(stderr, "invalid number\n");
        exit(EXIT_FAILURE);
    }
    if (x <= 0) {
        fprintf(stderr, "number not positive\n");
        exit(EXIT_FAILURE);
    }
    if (x > INT_MAX) { // INT_MAX is a macro which represents the maximum integer value
        fprintf(stderr, "number too large\n");
        exit(EXIT_FAILURE);
    }
    return (int)x;
}


int main(int argc, char** argv) {
  // 只允许 有两个 argument
  if(argc != 2) warn("argc is not 2");

  //use strtol to get the number!(useful for the klausur)
  int limit = parse_positive_int_or_die(argv[1]);

  // create one output thread
  pthread_t tid;
  errno = pthread_create(&tid, NULL, outputThread, NULL);
  if(errno != 0) die("pthread_create");


  int spawned = 0;

  while(1) {
    struct triangle *w = malloc(sizeof(struct triangle));
    if(!w) die("malloc");

    int matches = scanf("(%d,%d),(%d,%d),(%d,%d)",
            &w->point[0].x, &w->point[0].y,
            &w->point[1].x, &w->point[1].y,
            &w->point[2].x, &w->point[2].y);

    if(matches == EOF) break;

    // check if you give more than 3 pairs of points like: (1,2),(3,4)(5,6),(7,)
    int error = 0;
    if(matches != 6) {error = 1;}

    //get the left chars in the stdin
    int c = getchar();
    if(c != '\n' && c != '\0') {
      error = 1;
    }

    if(error) {
      fprintf(stderr, "does not match (%d,%d),(%d,%d),(%d,%d)\n");
      free(w);
      continue;
    }

    // create worker thread
    pthread_t workerTid;
    errno = pthread_create(&workerThread, NULL, workerThread, w);
    if(errno != 0) die("pthread_create");

    // active++
    outputState.active++;

    spawned++;
  }

  if(ferror(stdin)) die("fgets() error");

  // final cleanup
  shutdown = true;
  return EXIT_SUCCESS;
}