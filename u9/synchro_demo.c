// Übung 11, T02
// 21.06.2022

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include "sem.h"
#include "list.h"

static void usage(void);
static void die(const char *msg);
static void err(const char *msg);
static void *search(void *arg);

/**
 * Aufgabenstellung:
 * Parallel mehrere Dateien zeilenweise nach einem Suchstring durchsuchen. Gefundene Zeilen werden in eine Liste eingefügt und vom Hauptthread ausgegeben.
 *
 * Aufruf:
 * ./synchro_demo <zu suchendes Wort> <zu durchsuchende Datei_1> ... <zu durchsuchende Datei_n>
 */

/**
 * Funktionen aus den Folien:
 * - int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *args);
 * - pthread_t pthread_self(void);
 * - void pthread_exit(void *retval);
 * - int pthread_join(pthread_t thread, void **retvalp);
 * - int pthread_detach(pthread_t thread);
 *
 * - SEM *semcreate(int initVal);
 * - void P(SEM *sem);
 * - void V(SEM *sem);
 * - void semDestroy(SEM *sem);
 */

// TODO: global variables, defines, etc.

static SEM *sem_mutex; // gegenseitiger Ausschluss, Sem um kritische Abschnitte zu schützen
static SEM *sem_found; // Signalisierung, Sem um den Hauptthread zu informieren, dass etwas in der Liste zur Ausgabe bereit steht.

typedef struct pthread_args{
    char *file;
} pthread_args;


static char *search_pattern;
static int active_threads;

#define LINE_MAX (4094 + 1 + 1)

int main(int argc, char *argv[]) {
	if(argc < 3) {
		usage();
	}
    
    // TODO: initialize variables


    sem_mutex = semCreate(1);
    if(sem_mutex == NULL)
        err("semCreate() failed");
    
    sem_found = semCreate(0);
    if(sem_found == NULL)
        err("semCreate() failed");

	search_pattern = argv[1];
	active_threads = argc-2;

    /**
     * Struktur-Array, um für jeden der erstellten Threads eine Struktur zu haben.
     * Dies dient dazu, um den Thread-start-routinen gleich ihre Argumente zu übergeben.
     * In der Struktur steht der Name der zu durchsuchenden Datei.
     */
    pthread_args args[active_threads];
    
	
    /**
     * In Schleife über alle Dateien laufen, die wir durchsuchen sollen.
     * Dabei wird das Struktur-Element mit dem Dateinamen initialisiert.
     * Dann wird eine Variable für die Thread-ID erstellt.
     * Dann wird ein Thread erzeugt. Dieser führt die Funktion "search()" mit dem Dateinamen in seiner Struktur durch.
     */
    for(int i=2; i<argc; i++){
        args[i-2].file = argv[i];
    
        pthread_t tid;
        errno = pthread_create(&tid, NULL, &search, &args[i-2]);
        if(errno != 0)
            die("pthread_create");
    }
    
    /**
     * Main wartet, dass das Wort gefunden wurde.
     * -> Sie ruft P(sem_found) auf.
     * Dieser Sem ist am Anfang mit 0 initialisiert und entblockiert erst, wenn ein Thread etwas gefunden hat und V() aufgerufen hat.
     * Dieser Thread hat ein Element in eine Liste eingefügt. Das Element wollen wir nun aus der Liste herausnehmen.
     * Dafür müssen wir diesen Abschnitt mit dem sem_mutex schützen.
     * Wir erhalten die eingefügte Zeile als Rückgabewert von removeElement();
     * Diese Zeile geben wir dann aus.
     * Abbruchbedingung: nichts mehr in der Liste, dann verlassen wir die Schleife und räumen noch auf.
     */
    *
	char *line;
	do {
        P(sem_found);
        P(sem_mutex);
		line = removeElement();
        V(sem_mutex);
        
		if(line != NULL) {
			printf("%s", line);
			free(line);
		}
	} while(line != NULL);

	printf("done\n");
    
    semDestroy(sem_mutex);
    semDestroy(sem_found);
}

static void *search(void *a) {
    
    // TODO: Pfad holen

    errno = pthread_detach(pthread_self); // Rückgabewert wird nicht benötigt, Thread räumt sich nach Terminierung selbst auf.
    if(errno != 0)
        die("pthread_detach()");
    
    pthread_args *arg = (pthread_args *)a;
    
	char *path = arg->file;

	FILE *file = fopen(path, "r");
	if(file == NULL) {
		die("fopen");
	}

    /**
     * InsertElement und removeElement sind Funktionen, die auf gloablen Ressourcen arbeiten, daher bilden sie einen kritischen Abschnitt und müssen geschützt werden.
     * Nur ein Thread darf an der Liste arbeiten, um die Konsistenz zu bewahren.
     * Wenn etwas gefunden wurde, signalisiert dieser Thread der main mit V(), dass etwas zur Ausgabe bereit steht.
     */
    
	char buf[LINE_MAX];
	while(fgets(buf, sizeof(buf), file) != NULL) {
		if(strstr(buf, search_pattern) != NULL) {
            P(sem_mutex);
			if(-1 == insertElement(buf)) {
				err("insertElement() failed");
			}
            V(sem_mutex);
            V(sem_found);
		}
	}

	if(ferror(file)) {
		die("fgets");
	}

	if(0 != fclose(file)) {
		die("fclose");
	}
    
    /**
     * Falls das, was wir suchen, in keiner Datei vorhanden ist, würde der Hauptthread ewig warten und nie terminieren.
     * Also sorgen wir dafür, dass der letzte Thread einmal mehr V() aufruft und den Hauptthread so in jedem Fall am Ende weckt.
     * Dieser nimmt ein Element aus der leeren Liste, bekommt NULL zurück, beendet die Schleife und räumt auf.
     */
    
    P(sem_mutex);
    active_thread--;
    if(active_threads == 0){
        V(sem_found);
    }
    V(sem_mutex);
    
    
	return NULL;
}

static void usage(void) {
	fprintf(stderr, "Usage: synchro <search-pattern> <file> [<file> ...]\n");
	exit(EXIT_FAILURE);
}

// errno gesetzt
static void die(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

// errno nicht gesetzt
static void err(const char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}
