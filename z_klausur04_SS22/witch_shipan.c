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

static void warn(const char* msg) {
  fprintf(stderr, msg);
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
      // 因为你操作了suchThreadNr 在thread function， 所以也要保护起来。
      P(sem_mutex);
      suchThreadNr++;
      V(sem_mutex);
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
    err("not found anyone");
  }


}


// function thread_start
static void* thread_start(void* arg) {
  char* dir = (char*)arg;

  search_dir(header->search, dir, header->hunt);


  P(sem_mutex);
  suchThreadNr--;
  V(sem_mutex);

  V(sem_notify);

  return NULL;
}



// funktion search_dir

static void search_dir(char* search, char* dir, bool hunt) {

  char newPath[strlen(dir) + strlen(search) + 1];

  DIR* dirPointer = opendir(dir);
  if(dirPointer == NULL) warn("opendir");

  struct dirent* entry = NULL;
  struct stat buf;

  errno = 0;
  entry = readdir(dirPointer);

  while(entry != NULL) {
    // process entry
    if(lstat(entry->d_name, &buf) == -1) {
      errno = 0;
      entry = readdir(dirPointer);
      warn("lstat");
      continue;
    };

    // if it is a regular file
    if(S_ISREG(buf.st_mode)) {
      if(strcmp(entry->d_name, header->search) == 0) {
        //create new path;
        strcat(newPath, dir);
        strcat(newPath, "/");
        strcat(newPath, search);


        P(sem_mutex);
          // update listEleNr
          listEleNr++;

          // insert the "new path" into list
          insertElement(newPath);
        V(sem_mutex);

        // if delete this file
        if(header->hunt) {
          if(unlink(newPath) != 0) warn("unlink");
        }
      }
    }

    // if it is a verzeichniss
    if(S_ISDIR(buf.st_mode)) {
      search_dir(header->search, newPath, header->hunt);
    }

    errno = 0;
    entry = readdir(dirPointer);
  }
  if(errno) warn("readdir");

  if(closedir(dirPointer) == -1) warn("closedir");

  return NULL;
}
