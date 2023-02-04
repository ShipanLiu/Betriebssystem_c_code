#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "sem.h"
#include "list.h"

static const size_t MAX_LINE = 1024;
static const size_t CALC_THREADS = 5;

static void die(const char* msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

static void err(const char* msg) {
  fprintf(stderr, msg);
  exit(EXIT_FAILURE);
}

static void usage(void) {
  fprintf(stderr, "Usage: ./parrots <file\n>");
  exit(EXIT_FAILURE);
}

typedef struct {
  char *search;
  char* dir;
  bool hunt;
} search_t;


// funtion declare + global

search_t* header = NULL;
static int suchThreadNr = 0;
static int listEleNr = 0;

static void* thread_start(void*);
static void search_dir(char*, char*, bool);

// Klausur！！  Semaphore 在这里定义
SEM* sem_mutex;
SEM* sem_notify;




int main(int argc, char** argv) {

  // Deklaration
  sem_mutex = semCreate(1);
  if(!sem_mutex) die("semCreate");

  sem_notify = semCreate(0);
  if(!sem_mutex) die("semCreate");

  // Befehlzeilenargument prüfen
  if(argc < 2 || argc >3 ) usage();

  if(argc == 3) {
    if(strcmp(argv[1], "--hunt") == 0) {
      header->hunt = true;
      header->search = argv[2];
    } else {usage();}
  }

  if(argc == 2) {
    header->hunt = false;
    header->search = argv[2];
  }


  //Initialisierung und PATH auslesen
  char* path = getenv("PATH");
  if(!path) die("getenv");

  //verzeichniss aus PATH extrahieren
  char dirArr[strlen(path)];
  int index = 0;
  dirArr[index] = strtok(path, ":");
  index++;

  while((dirArr[index] = strtok(NULL, ":")) != NULL) {
    index++;
  }

  // the size of dirArr is index+1. but the last element is NULL, so the size is  = index

  //Such-threads starten und auf Beendigung warten
  pthread_t pid[index];
  for(int i = 0; i < index; i++) {
    errno = pthread_create(&pid[i], NULL, thread_start, dirArr[i]);
    if(errno) {
      die("pthread_create");
    } else {
      suchThreadNr++;
    }

    // detach
    errno = pthread_detach(pid[i]);
    if(errno) die("pthread_detach");
  }


  //check if all thread dead
  P(sem_notify);

  P(sem_mutex);
    int num = suchThreadNr;
  V(sem_mutex);

  if(num == 0) {
    // start ausgabe; // Ausgabe der gefundenen Datein
    for(int i = 0; i < listEleNr; i++) {
      char str = removeElement();
      printf("%s\n", str);
      free(str);
    }
  }

  // destroy SEM

  semDestroy(sem_mutex);
  semDestroy(sem_notify);

  // return //Aufraumen und Beenden
  if(listEleNr) {
    exit(EXIT_SUCCESS);
  } else {
    die("not found anyone");
  }


}


// function thread_start
static void* thread_start(void* arg) {

}



// funktion search_dir

static void search_dir(char* search, char* dir, bool hunt) {

}
