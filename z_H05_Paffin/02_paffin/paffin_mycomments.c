/*

semLimit  用在  创建 grep thred  和  死亡 grep thread 里面

semNotify   用在 block main  和   craw thred  和  死亡 grep thread 里面



 */



#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>
#include <sys/stat.h>
#include <string.h>

#include "list.h"s
#include "sem.h"

// (module-)global variables
static int activeGrepThreads;
static int activeCrawlThreads;
// search string
static char* searchString;
// the number of items in the queue
static unsigned long hitsNr = 0;



// create semaphore
static SEM* semMutex;
static SEM* semNotify;
static SEM* semLimit;


// function declarations
static void* processTree(void* path);
static void* processDir(char* path);
static void* processEntry(char* path, struct dirent* entry);
static void* processFile(void* path);

static void usage(void) {
	fprintf(stderr, "Usage: paffin <string> <max-grep-threads> <trees...>\n");
	exit(EXIT_FAILURE);
}

static void die(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

// increase variable function
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

/**
 * \brief Initializes necessary data structures and spawns one crawl-Thread per tree.
 * Subsequently, waits passively on the termination of all threads.
 * If all threads terminated, it dequeues all list entries, prints them, and subsequently
 * frees all allocated resources and exits/returns.
 */

int main(int argc, char** argv) {
	if(argc < 4) {
		usage();
	}

	// convert argv[2] (<max-grep-threads>) into long with strtol()
	errno = 0;
	char *endptr;
	int maxGrepThreads = strtol(argv[2], &endptr, 10);

	// argv[2] can not be converted into long without error
	if(errno != 0 || endptr == argv[2] || *endptr != '\0') {
		usage();
	}

	if(maxGrepThreads <= 0) {
		fprintf(stderr, "max-grep-threads must not be negative or zero\n");
		usage();
	}

	// TODO: implement me!

  // get the searchString
  searchString = argv[1];

  // create sem
  semMutex = semCreate(1);
  if(semMutex == NULL) {
    die("semCreate");
  }
  semNotify = semCreate(0);
  if(semNotify == NULL) {
    die("semCreate");
  }
  semLimit = semCreate(maxGrepThreads);
  if(semMutex == NULL) {
    die("semCreate");
  }

  //now create craw threads
  pthread_t tids[argc-3];
  for(int i=3; i<argc; i++) {
    errno = pthread_create(&tids[i], NULL, processTree, argv[i]);
    if(errno != 0) die("pthread_create");

    // self detach
    errno = pthread_detach(tids[i]);
    if(errno != 0) die("pthread_detach");

    // increase activeCrawlThreads
    incVal(&activeCrawlThreads);
  }

  // block untill all craw and grep threads are terminated
  while(1) {

    // unblock if one thread is dead and check if it's time to dequeue
    P(semNotify);

    // get the latest "activeCrawlThreads" and "activeGrepThreads"
    P(semMutex);
    int activeCrawNr = activeCrawlThreads;
    int activeGrepNr = activeGrepThreads;
    V(semMutex);

    // check if it's time to dequeue
    if(activeCrawNr > 0 || activeGrepNr > 0) {
      continue;
    }else {
      // dequeue + free place + break <--- need the number of items!
      for(int i=0; i<hitsNr; i++) {
        char* str = dequeue();
        if(puts(str) == EOF) die("puts");
        // free
        free(str);
      }
      // fflush and break
      if(fflush(stdout) == EOF) die("fflush");
      break;
    }

  }

  // destroy the semaphore
  semDestroy(semMutex);
  semDestroy(semNotify);
  semDestroy(semLimit);
	return EXIT_SUCCESS;
}

/**
 * \brief Acts as start_routine for crawl-Threads and calls processDir().
 *
 * \param path Path to the directory to process
 *
 * \return Always returns NULL
 */
static void* processTree(void* path) {
	//cast the parameter
  char* treePath = (char*)path;
  // get dir pointer and call processDir
  processDir(treePath);

  // this thread is dead now
  decVal(&activeCrawlThreads);
  // one thread dead --> unblock tree
  V(semNotify);
	return NULL;
}



/**
 * \brief Iterates over all directory entries of path and calls processEntry()
 * on each entry (except "." and "..").
 *
 * \param path Path to directory to process
 *
 * \return Always returns NULL
 */

static void* processDir(char* path) {
	// here we will open and read dirPointer and iterate
  DIR* dirPointer = opendir(path);
  if(dirPointer == NULL) die("opendir");

  //now get the entries
  struct dirent* entry = readdir(dirPointer);
  errno = 0;
  while(entry != NULL) {
    // ignore if the entry is "." or ".."
    if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      processEntry(path, entry);
    }

    // repeat to find other entries
    errno = 0;
    entry = readdir(dirPointer);
  }

  // now entry is NULL, check if it is beecause of EOF or errno
  if(errno) die("readdir");
  //close the dirPointer
  if(closedir(dirPointer) == -1) die("closedir");
	return NULL;
}



/**
 * \brief Spawns a new grep-Thread if the entry is a regular file and calls processDir() if the entry
 * is a directory.
 *
 * It updates activeGrepThreads if necessary. If the maximum number of active grep-Threads is
 * reached the functions waits passively until another grep-Threads can be spawned.
 *
 * \param path Path to the directory of the entry
 * \param entry Pointer to struct dirent as returned by readdir() of the entry
 *
 * \return Always return NULL
 */
static void* processEntry(char* path, struct dirent* entry) {
	//concat the path and entryName --> create a new path called pathPlusEntry to open.
  int pathPlusEntryLength = strlen(path) + strlen(entry->d_name) + 2; // 2 means '/' + '\0'
  char* pathPlusEntry = (char*)malloc(pathPlusEntryLength*sizeof(char));

  if(pathPlusEntry == NULL) die("malloc");

  // concat to a new string
  strcat(pathPlusEntry, path);
  strcat(pathPlusEntry, "/");
  strcat(pathPlusEntry, entry->d_name);

  // now check file type

  struct stat buf;
  if(lstat(entry->d_name, &buf) == -1){
    free(pathPlusEntry);
    die("lstat");
  }

  if(S_ISDIR(buf.st_mode)) {
    //back to processDir and pass the newst path concat
    processDir(pathPlusEntry);
  }

  if(S_ISREG(buf.st_mode)) {

    // due to the limited number of grep threads
    P(semLimit);


    // here you create a new grep thread
    pthread_t tid;
    errno = pthread_create(&tid, NULL, processFile, pathPlusEntry);
    if(errno != 0) die("pthread_create");

    // self detach
    errno = pthread_detach(tid);
    if(errno != 0) die("pthread_datach");

    // update activeCrawlThreads
    incVal(&activeGrepThreads);

  }


  free(pathPlusEntry);
	return NULL;
}












/**
 * \brief Acts as start_routine for grep-Threads and searches all lines of the file for the
 * search pattern.
 *
 * It adds a line, the corresponding line number and the path to the file to the list if the
 * search pattern is found.
 *
 * \param path Path to the file to process
 *
 * \return Always returns NULL
 */
static void* processFile(void* path) {

  // this is a thread function, so first cast
  char* filePath = (char*)path;

  // open file
  FILE* fh = fopen(filePath, "r");
  if(!fh) die("fopen");

  // create space to save each line
  char* line[4096+2];
  // record the line number
  unsigned long lineNum = 0;

  // read file per line, if hit ---> enqueue()
  while(fgets(line, sizeof(line), fh)) {
    lineNum++;
    // check if hit
    if(strstr(line, searchString)) {
      // protect "enqueue" and "hitsNr"
      P(semMutex);

      enqueue(filePath, line, lineNum);
      hitsNr++;

      V(semMutex);
    }
  }

  // check the reason of break :  errno or EOF
  if(ferror(fh)) die("fgets");

  //close file
  if(fclose(fh) == EOF) die("fclose");

  // one thread dead
  decVal(&activeGrepThreads);

  // one thread dead --> unblock the main, check if the number of active thread is 0 or not.
  V(semNotify);

  //free one vacancy
  V(semLimit);
	return NULL;
}
