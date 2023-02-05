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

static unsigned int ginterior = 0; // insgesamt gefundene Interior-Points
static unsigned int gboundary = 0; // insgesamt gefundene Boundary-Points


// start main
int main(int argc, char** argv) {

  if(argc != 2) usage();


  // initialisierung（先创建一个 节点， 相当）
  work_queue = qCreate();
  if(work_queue == NULL) die("qCreate");

  sem_mutex = semCreate(1);
  if(sem_mutex == NULL) die("semCreate");

  sem_notify = semCreate(0);
  if(sem_notify == NULL) die("semCreate");


  // 下面的思路是： 先把 所有的thread 先 start
  // 因为按照题目 要求， 你就应该 首先创建 “CALC_THREADS” 这么多的 threads，
  // 而且 man page 只给了你  pthread_join 这个函数， 因为你只有  “CALC_THREADS” 这么多个 Rechenthreads,
  // 却要处理 比如1000 行 内容， 所以 Rechenthreads 不能轻易死亡， 只有把 queue 里面的 所有 triangle 完事
  // 之后，才能 去结束 Rechenthreads， 所以 有理由 去使用 pthread_join() （你不调用 pthread_join 的话， Rechenthreads 是不会死的）

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

  // 现在开始 读取 triangle.txt 文件， 把每一行 装入 queue。

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

    // get rid of the '\n'
    if(line[MAX_LINE] == '\n') {
      line[MAX_LINE] = '\0';
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
  // struct triangle tri;
  // bool success = parseTriangle(line, &tri);
  // if(!success) {
  //   fprintf(stderr, "bad parse");
  //   continue;
  // }

  // // create new node and insert it into the queue
  // struct triangle* node = malloc(sizeof(struct triangle));
  // if(node == NULL) die("malloc");

  // *node = tri;

  // qPut(work_queue, node);

  // another way of writing code
  // 目的： 把line 变成 tri（前提是 malloc 一个 new node）, 然后把 tri 装进去 queue 用 qPut
    struct triangle* tri = malloc(sizeof(struct triangle));
    if(tri == NULL) die("malloc");

    bool success = parseTriangle(line, tri);
    if(!success) {
      free(tri);
      fprintf(stderr, "bad parse");
      continue;
    }
    qPut(work_queue, tri);

  }


  // Threads + Ressourcen aufraumen
  fclose(fh);



  // 额外塞进去 CALC_THREADS 个 NULL
  // Hinweise（来自题目）： Speichern Sie den Wert NULL in der Queue um die Arbeitsthreads "zuverlässig" zu beenden
  // 意思是： 只要有一个 Arbeitsthread 碰到了NULL， 就说明完事了。（有可能 这几个 Arbeitsthreads 同时 碰到NULL， 所以定义这么多 NULL）
  for(size_t i = 0; i<CALC_THREADS; i++) {
    qPut(work_queue, NULL);
  }


  // 之所以 敢 这里用 join， 是因为 thread 不会轻易结束， 里面用了 while schleife，
  // 一旦 一个thread里面有 break了， 这个当父亲的 main 就要在这里收尸

  for(size_t i=1; i<CALC_THREADS+1; i++) {
    errno = pthread_join(ts[i], NULL);
    if(errno != 0) die("pthread_join");
  }

  finished = true; // alle Rechenthreads wurden aufgesammelt
  V(sem_notify); // weckt Ausgabethread und sorgt dafür, dass der Endstand einmal ausgegeben wird

  errno = pthread_join(ts[0], NULL); // Ausgabethread aufsammeln
  if(errno != 0)
      die("pthread_join");

  qDestroy(work_queue);
  semDestroy(sem_mutex);
  semDestroy(sem_notify);
}
//end main


// Funktion Rechenthread function（这就需要在前面 declare了）
// Funktion Rechenthread
static void* calc_thread(void *arg){
    // Soll rechnen, bis er nichts mehr aus der Queue holen kann
    while(42){
        struct triangle *tri = qGet(work_queue); // Element aus Queue holen(指针会自动 指向下一个 元素)
        // 如果碰到 NULL了的话，说明 queue 到最后了。
        if(tri == NULL)
            break; // Bricht ab, sobald NULL aus der Queue geholt wird. Daher fügt die Main für jeden Thread einmal NULL in die Queue.

        // lokale Variablen für Werte des aktuell zu bearbeitenden Dreiecks
        int boundary = 0;
        int interior = 0;
        countPoints(tri, &boundary, &interior); // Punkte zählen

        // globale Werte aktualisieren
        P(sem_mutex);
        gboundary += boundary;
        ginterior += interior;
        V(sem_mutex);

        V(sem_notify); // sagt Ausgabethread, dass neue Werte verfügbar sind
        free(tri);
    }

    return NULL;
}
// Ende Rechensthread


// Ausgabethread
// 因为 Ausgabe thread 要指定多次， 就像 main 负责 输出一样， 这里的Ausgabe thread 也要有 while schleife
static void * print_thread(void *arg){

    // Solange noch Rechenthreads laufen.
    // Wird noch einmal ausgeführt, wenn alle fertig sind.
    while(!finished){
        P(sem_notify); // wartet auf Benachrichtigung von den Rechenthreads

        // aktuelle Werte holen
        P(sem_mutex);
        unsigned int boundary = gboundary;
        unsigned int interior = ginterior;
        V(sem_mutex);

        printf("\rBoundary: %u, Interior: %u", boundary, interior); // \r setzt das Schreiben in der Zeile am Anfang an.
        fflush(stdout);
    }

    return NULL;
}
// Ende Ausgabethread
