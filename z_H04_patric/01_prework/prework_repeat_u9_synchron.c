#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

//controll the input bits
#define MAX_LINE (4094+1+1)


#include "list.h"
#include "sem.h"

// global variable!!!! Klausur 注意！！！！！！！
static SEM* sem1;
static SEM* sem2;

// targeted word for searching
static char* word;
static int fileNumber = 0;



typedef struct search_args{
  char* file;
}search_args;


// die function
static void die(char* msg) {
  fputs(msg, stderr);
  exit(EXIT_FAILURE);
}

// warn function(no exit with failure)
static void warn(char* msg) {
  fputs(msg, stderr);
}

// according to the pthread_create(), search() function should return a void* type,
// and the argument of search() function should also be type void*
static void* search(void *a) {

  // we just add the pthread_detach function for collecting
  errno = pthread_detach(pthread_self);
  if(errno != 0) {
    die("pthread_detach");
  }

  // cast to the search_args* type(cast between pointer type has no danger)
  char* path = ((search_args*)a) -> file;  // becauase it is a pointer, so you have to use -> but not .
  FILE* file = fopen(path, "r");
  if(file == NULL) {
    die("fopen");
  }

  // you get the file pointer, then read the string out
  // fgets need a buffer to save the string
  char strBuf[MAX_LINE];


  if(fgets(strBuf, sizeof(strBuf), file) == NULL) {
    if(ferror(file)) {
      die("fgets");
    }
  }

  // find the word, then insert it
  if(strstr(strBuf, word) != NULL) {
    p(sem1);

    if(insertElement(strBuf) == -1) {
      die("insertElement");
    }

    // now insert finished
    V(sem1);
    //now the main thread can work
    V(sem2);
  }

  // fclose
  if(fclose(file) == EOF) {
    die("fclose");
  }



  // 在涉及到 多个 threads 对 单个 变量 进行操作的时候， 需要上锁。

  P(sem1);
  // this file is done
  fileNumber--;
  if(fileNumber == 0) {
    //让 main thread 接着 removeElement
    V(sem2);
  }



  // I want a void* back
  return NULL;
}





int main(int argc, char* argv[]) {

//KLAUSUR！！！！！！！ check argument number,  不能用 die，因为errno 没有set ， 不能用 warn，因为我们想要 exit failure
if(argc <= 2) {
  fputs("input too short", stderr);
  exit(EXIT_FAILURE);
}
// create sem
sem1 = semCreate(1);
if(sem1 == NULL) {
  die("semCreate");
}

sem2 = semCreate(0);
if(sem2 == NULL) {
  die("semCreate");
}

// read the input
// argv[1] = "the word to find"  argv[2.....n-1] = "the file name"

word = argv[1];
fileNumber = argc - 2;

// because the filenames will be the parameter of the search function, so we create a buffer of search_args type
search_args filesBuffer[fileNumber];

for(int i = 0; i < fileNumber; i++) {
  filesBuffer[i].file = argv[2+i];

  //for each file, we create a thread for using
  pthread_t tid;
  errno = pthread_create(&tid, NULL, &search, &filesBuffer[i]);
  if(errno != 0) {
    die("pthread_create");
  }
}

// 上面 一口气 创建了 那么多的thread， 现在在 main 中进行 移除的操作。

char* str
while(1) {
  P(sem2);

  P(sem1);
  str = removeElement();
  V(sem1);
  if(str != NULL) {
    prinf("%s", line);
  }else {
    break;
  }

}

printf("done!!!");


semDestroy(sem1);
semDestroy(sem2);

  exit(EXIT_SUCCESS);
}
