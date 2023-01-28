#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CMD_MAX 100

static void die(const char* msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

static void usage(void) {
  fprintf(stderr, "Usage: hupsi <n>\n");
  exit(EXIT_FAILURE);
}

struct process {
  pid_t pid;

  //Eigene Mitgelieder


};

// Makros, Funktionsdeklarationen globale Variablen


// Function main
int main(int argc, char** argv) {



}

// Ende Funktion main


// Funktion run
static pid_t run(char* cmdline) {

}
// Ende Funktion run


//
static void waitProcess(void) {


//auf beliebigen Process warten


// struct process zu PID aus Processliste ermitteln


// (Vorbereiten der) Ausgabe


// Process aus Liste entfernen & Speicherfreigeben




}
//Ende Funktion waitProcess
