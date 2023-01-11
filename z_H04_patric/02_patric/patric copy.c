// Korrektur: Rico Wedig
// Punkte: 6 von 14

// 写的 乱七八糟。

/*I----> +--------------------------------------------------------------------+
         | Die grundlegende Idee und Umsetzung ist zu erkennen, insbesondere  |
         | die Erstellung der Arbeiter-Threads und des Ausgabe-Threads.       |
         | Allerdings sollte nur EIN Ausgabe-Thread erzeugt werden, der von   |
         | den Arbeiter-Threads über ein Semaphore (z.B. sem_notify)         |
         | benachrichtigt wird, dass eine Änderung stattgefunden hat. Zudem  |
         | ist die Verwendung der Semaphoren nicht wirklich nachvollziehbar   |
         | und eher verwirrend. Es wäre besser (insbesondere aus             |
         | Korrekturgründen) die Semaphoren mit entsprechenden Namen zu      |
         | versehen (sem_lock für den kritischen Abschnitt, sem_notify,      |
         | sem_thread_limit, der das passive Warten auf die Arbeiter-Threads  |
         | realisiert oder irgendwie ähnliche Namen etc.), da sonst die      |
         | Verwendung und der Zweck der Semaphoren sehr verwirrend ist.       |
         |                                                                    |
         | Für ein besseres Verständnis der Semaphoren in Bezug auf die     |
         | patric empfehle ich euch die Tafelübung zur patric zu besuchen,   |
         | da Threads und Semaphoren ein wichtiger Bestandteil der KLausuren  |
         | ist. In einer alten Klausur kam bspw. eine Abwandlung der patric   |
         | als Programmieraufgabe dran.                                       |
         +-------------------------------------------------------------------*/

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
/*I----> +--------------------------------------------------------------------+
         | Zugriff auf globale Variablen muss mit einem Semaphore abgescihert |
         | werden (-1)                                                        |
         +-------------------------------------------------------------------*/

/*I----> +--------------------------------------------------------------------+
         | Der Ausgabe-Thread soll nach jeder Änderung über ein Semaphore   |
         | informiert werden (-0.5)                                           |
         +-------------------------------------------------------------------*/
  bnodeSum += boundary;
  inodeSum += interior;
  nodeSum += (boundary + interior);
}


static void *outputThreadFunction(void *a) {
  errno = pthread_detach(pthread_self());
  if(errno != 0) {
    die("pthread_detach");
  }

  /**
   * Der Ausgabe-Thread müsste in einer Schleife jeweils den
   * aktuellen Stand ausgeben. Die main bricht dann die while
   * Scheife des Ausgabe-Threads ab, wenn alle Arbeiter-Threads
   * fertig sind.
   * Also so etwas wie
   * static int running = 1 als globale Variable oben, dann
   * while(running){
   *    printf(boundary, interior, active_threads etc.)
   * } im Ausgabe-Thread
   *
   * main(){ ... running = 0; etc.}
   *
   * Natürlich kann das auch anders gelöst werden statt mit while.
   * Aber es muss sich am Ende um EINEN Ausgabe-Thread handeln der
   * dauernd den aktuellen Stand ausgibt, sobald sich dieser ändert.
   * (-1)
   */

/*I----> +--------------------------------------------------------------------+
         | printf greift auf globae Variablen zu --> Semaphore (-1)           |
         +-------------------------------------------------------------------*/

  P(sem2);
  fprintf(stdout, "the boundary node number: %d\n", bnodeSum);
  fprintf(stdout, "the interior node number: %d\n", inodeSum);
  fprintf(stdout, "the thread number is: %d\n", threadNumber);
  fprintf(stdout, "the closed thread number is %d\n", closedThreadNumber);
  fprintf(stdout, "================================================\n");
  bnodeSum = 0;
  inodeSum = 0;
  nodeSum = 0;

/*I----> +--------------------------------------------------------------------+
         | bnodeSum etc. sollen nicht zurückgesetzt werden, da die globalen  |
         | Punkte der Dreiecke insgesamt gezählt werden sollen (-0.5)        |
         +-------------------------------------------------------------------*/

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


/*I----> +--------------------------------------------------------------------+
         | Laut deiner Beschriebung sind die Semaphoren sem1 und sem0 für    |
         | den kritischen Abschnitt gedacht. Dafür wird aber nur ein         |
         | Semaphor benötigt, wozu 2? Zudem musst du das Semaphore nach dem  |
         | kritischen Abschnitt wieder freigeben, sonst ergibt das ja keinen  |
         | Sinn (-0.5) (sem0). Zudem ist countPoints nicht Teil des           |
         | kritischen Abschnitts, da die Funkion selbst den kritischen        |
         | Abschnitt behandeln soll (siehe Kommentare in der callback)        |
         +-------------------------------------------------------------------*/

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

/*I----> +--------------------------------------------------------------------+
         | Fehlerbehandlung sscanf fehlt (-1)                                 |
         +-------------------------------------------------------------------*/

/*I----> +--------------------------------------------------------------------+
         | Die Verwendung deiner Semaphoren ergibt nicht wirklich einen Sinn. |
         | Insgesamt bräuchtest du mind. 4 Semaphoren für folgenden Zweck:  |
         | - sem_lock --> für den kritischen Abschnitt (Erhöhen der Punkte  |
         |   der Dreiecke etc.) sem_notify --> um dem Ausgabe-Thread zu       |
         |   signalisieren, dass die Werte geändert/aktualisiert wurden und  |
         |   der Ausgabe-Thread den aktuellen Stand ausgeben kann             |
         |   sem_num_of_threads --> für das passive Warten des main-Threads  |
         |   auf die Arbeiter-Threads sem_finished --> Zum Signalisieren,     |
         |   dass die Threads beendet sind                                    |
         |                                                                    |
         | Du hast zwar ausreichend Threads initialsisiert, aber deren        |
         | Verwendung ist nicht wirklich nachvollziehbar (-1.5). Schau dir    |
         | hierzu am besten die Tafelübung zur patric an.                    |
         +-------------------------------------------------------------------*/

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

/*I----> +--------------------------------------------------------------------+
         | Es sollte nur ein Ausgabe-Thread erzeugt werden und dieser auch    |
         | als Erstes, oben im Code vor der while. Danach dann in der while   |
         | die Arbeiter-Threads erzeugen. (-1)                                |
         +-------------------------------------------------------------------*/

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

/*I----> +--------------------------------------------------------------------+
         | Das passive Warten auf die Beendigung der Threads sollte nach der  |
         | While erfolgen (-1). Ich weiß nicht, ob sem4 (oder sem3?) dafür  |
         | erzeugt wurde. Wenn ja, dann müsstest du hier so etwas wie        |
         |                                                                    |
         | for(int i = 0; i < threadNumber; i++){ P(sem4)/P(sem3); }          |
         |                                                                    |
         | threadNumber wird dabei am Anfang auf 0 initialisisert und bei     |
         | jeder Erstelung eines Arbeiter-Threads jeweils erhöht. Und in     |
         | den Arbeiter-Threads und dem Ausgabe-Thread ruftst du am Ende      |
         | jeweils V(sem4)/V(sem3) auf (oder welches Semaphore dafür auch    |
         | immer gedacht war). Schaue dir hier am besten die Tafelübung zur  |
         | patric an.                                                         |
         +-------------------------------------------------------------------*/


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
/*P----> +--------------------------------------------------------------------+
         | Punktabzug in dieser Datei: 8 Punkte                               |
         +-------------------------------------------------------------------*/
