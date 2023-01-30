#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <inttypes.h>

#include "sem.h"
#include "queue.h" //qCreate, qPut, qGet, qDestroy
#include "triangle.h"

static const size_t MAX_LINE = 1024;
static const size_t CALC_THREADS = 5;

static void die(const char* msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

static void usage(void) {
  fprintf(stderr, "Usage: ./parrots <file\n>");
  exit(EXIT_FAILURE);
}

// global variable and declariation
static void* print_thread(void* a);
static void* calc_thread(void* a);

static SEM* sem_mutex = NULL;
static SEM* sem_notify= NULL;

static volatile bool finished = false;
static QUEUE* work_queue = NULL;


// start main
int main(int argc, char** argv) {

  if(argc != 2) usage();


  // initialisierung
  work_queue = qCreate();
  if(work_queue == NULL) die("qCreate");

  sem_mutex = semCreate(1);
  if(sem_mutex == NULL) die("semCreate");

  sem_notify = semCreate(0);
  if(sem_notify == NULL) die("semCreate");


  // 下面的思路是： 先把 所有的thread 先 start

  // Thread starten
  // tid of printThread and calcThread
  pthread_t ts[CALC_THREADS + 1];
  errno = pthread_create(&ts[0], NULL, print_thread, NULL);
  if(errno != 0) die("pthread_create");

  // create the Rechnenthreads
  for(size_t i = 1; i < CALC_THREADS; i++) {
    errno = pthread_create(&ts[i], NULL, calc_thread, NULL);
    if(errno != 0) die("pthread_create");
  }

  // Arbritspaket aus der Datei auslesen
  FILE* fh = fopen(argv[1], "r");
  if(!fh) die("fopen");


  // haupt Schleife
  while(1) {
    char line[MAX_LINE+2];
    if(fgets(line, sizeof(line), fh) == NULL) {
      if(ferror(fh)) die("fgets");
      break;
    }

    // if too long(老套路)
    size_t len = strlen(line);
    if(len == MAX_LINE+1 && line[MAX_LINE] != '\n') {
      fprintf(stderr, "line too long\n");

      int c;
      do{
        c = fgetc(fh);
      }while(c != EOF && c != '\0');
      if(ferror(fh)) die("fgetc");

      continue;
    }

    //ceeate a new tri instance for later usage
    struct triangle tri;
  bool success = parseTriangle(line, &tri);
  if(!success) {
    fprintf(stderr, "bad parse");
    continue;
  }

  // create new node and insert it into the queue
  struct triangle* node = malloc(sizeof(struct triangle));
  if(node == NULL) die("malloc");

  *node = tri;

  qPut(work_queue, node);

  }


  // Threads + Ressourcen aufraumen
  fclose(fh);
  // insert null for each cal thread
  for(size_t i = 0; i<CALC_THREADS; i++) {
    qPut(work_queue, NULL);
  }

  for(size_t i=1; i<CALC_THREADS+1; i++) {
    errno = pthread_join(ts[i], NULL);
    if(errno != 0) die("pthread_join");
  }

  V(sem_notify); // notify the ausgabethread and give out put the data

  errno = pthread_join(ts[0], NULL);
  if(errno != 0) die("pthread_join");

  qDestroy(work_queue);
  semDestroy(sem_mutex);
  semDestroy(sem_notify);

  exit(EXIT_SUCCESS);
}
//end main


// Funktion Rechenthread function（这就需要在前面 declare了）
static void* calc_thread(void* a) {
  // time 1:29: 19
}



// Ende Rechenthread






// Ausgabethread function



// Ende Ausgabethread
