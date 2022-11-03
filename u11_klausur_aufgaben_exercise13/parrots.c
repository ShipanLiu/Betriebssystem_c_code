/**
 * Aufgabe 2:
 * Zeilenweise Koordinaten von Dreiecken aus einer per Kommandozeile übergebenen Datei einliest und die Anzahl der ganzzahligen Koordinaten auf den Kanten und innerhalb der Dreiecke berechnet.
 *
 * Format Dreieck: (x1,y1),(x2,y2),(x3,y3)
 * Zeile: maximal 1024 Zeichen ohne \n \0
 * MAX_LINE = 1024
 * Überlange Zeile /  falsch formatierte Zeile: mit Warnmeldung ignorieren.
 * Umwandlung der Zeile und Berechnung der Punkte: Funktionen in triangle.o
 *  -> parseTriangle()
 *  -> countPoints()
 *
 * Zwischenstand regelmäßig aktualisieren und ausgeben.
 * Berechnungen werden an CALC_THREADS Rechenthreads ausgelagert.
 * -> Ausgabethread signalisieren, dass neue Zwischenergebnisse vorliegen
 */

/**
 * Struktur Hauptthread:
 * Datenstrukturen initialisieren
 * Rechenthreads erzeugen (CALC_THREADS viele)
 * zeilenweise übergebene Datei einlesen
 * Arbeitspakete (struct) erzeugen und in Workqueue einfügen
 * Nach Einlesen und Abarbeiten aller Dreiecke wartet er auf die Beendigung aller Threads, gibt deren Ressourcen frei und beendet sich.
 *
 * Struktur Rechenthreads:
 * Dreiecke aus Workqueue entnehmen
 * Zählung der Punkte mit countPoints() durchführen
 * Ausgabethread signalisieren, dass neue Werte vorliegen
 *
 * Struktur Ausgabethread:
 * nach der Signalisierung durch Rechenthreads: aufsummierte Anzahl an interior und boundary Punkten ausgeben (stdout)
 *
 * Kommunikation über modulglobale Variablen (-> static!)
 * Keine FB für Ausgaben auf stdout/stderr nötig
 *
 * Auf korrekte Synchro achten!
 * Passiv warten, printf etc. nicht in kritischen Abschnitten
 *
 * Hinweise:
 * NULL in Queue speichern, um Rechenthreads zuverlässig zu beenden
 * alle Operationen der Queue brauchen keine Synchro
 */


#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "sem.h"
#include "triangle.h"
#include "queue.h"

/* Funktionen aus sem.h */
SEM *semCreate(int initVal); // sets errno on failure
void semDestroy(SEM *sem);
void P(SEM *sem);
void V(SEM *sem);

// Funktionen triangle.h (parseTriangle, countPoints): SIEHE MANPAGE
// Funktionen queue.h (qCreate, qPut, qGet, qDestroy): SIEHE MANPAGE

static const size_t MAX_LINE = 1024;
static const size_t CALC_THREADS = 5;

/**
 * Angegebene Funktionen lesen lohnt sich, kann Zeit sparen.
 * Variablennamen sinnvoll wählen, ein Buchstabe ist zwar kurz, kann aber zu Flüchtigkeitsfehlern führen. Trotzdem aber auch nicht zu lange Namen wählen, dadurch geht Zeit verloren.
 */

static void die(const char message[]) {
    perror(message);
    exit(EXIT_FAILURE);
}

static void usage(void) {
    fprintf(stderr, "Usage: ./parrots <file>\n");
    exit(EXIT_FAILURE);
}

// TODO Funktions- und Strukturdeklarationen, globale Variablen etc.
static QUEUE* work_queue = NULL; // Queue enthält zu bearbeitende Pakete

static unsigned int ginterior = 0; // insgesamt gefundene Interior-Points
static unsigned int gboundary = 0; // insgesamt gefundene Boundary-Points

// Volatile, damit die Variable nicht vom Compiler optimiert wird. So bleibt die Variable up-to-date für die Threads (ohne Synchro, ist so nicht nötig).
// Variable, um dem Ausgabethread zu sagen, dass alle Rechenthreads beendet und aufgesammelt wurden.
static volatile bool finished = false;

static SEM *sem_mutex = NULL; // Sem als Mutex, um kritische Bereiche zu schützen
static SEM *sem_notify = NULL; // Sem zur Signalisierung, Rechenthreads sagen Ausgabethread Bescheid (V()), dass es etwas zum Ausgeben gibt, während der Ausgabethread darauf wartet (P()), dass die Rechenthreads ihm Bescheid geben.

static void* print_thread(void *arg); // Funktion für Ausgabethread
static void* calc_thread(void *arg); // Funktion für Rechenthreads

// Hauptfunktion (main)
int main(int argc, char *argv[]){
    if(argc != 2) { usage (); }
    // Initialisierung

    work_queue = qCreate();
    if(work_queue == NULL)
        die("qCReate");
    
    sem_mutex = semCreate(1);
    if(sem_mutex == NULL)
        die("semCreate");
    
    sem_notify = semCreate(0);
    if(sem_notify == NULL)
        die("semCreate");
    
    // Threads starten
    
    pthread_t ts[CALC_THREADS + 1]; // CALC-THREADS viele Rechenthreads und ein Ausgabethread
    errno = pthread_create(&ts[0], NULL, print_thread, NULL); // Ausgabethread erzeugen
    if(errno != 0)
        die("pthread_create");
    
    // Rechenthreads erzeugen.
    for(size_t i = 0; i < CALC_THREADS; i++){
        errno = pthread_create(&ts[i+1], NULL, calc_thread, NULL);
        if(errno != 0)
            die("pthread_create");
    }


    // Arbeitspakete aus der Datei auslesen
    FILE *fh = fopen(argv[1], "r"); // Datei zum Lesen öffnen
    if(fh == NULL)
        die("fopen");
    
    // "Haupt"schleife
    while(42){
        // Zeile aus Datei einlesen
        char line[MAX_LINE + 2];
        if(fgets(line, MAX_LINE+2, fh) == NULL){
            if(ferror(fh))
                die("fgets");
            break; // aufhören, wenn Datei ausgelesen ist.
        }
        
        // Überlange Zeile erkennen
        size_t len = strlen(line);
        if(len == MAX_LINE + 1 && line[MAX_LINE] != '\n'){
            fprintf(stderr, "Line too long!\n");
            
            int c;
            do{
                c = fgetc(fh);
            }while(c != EOF && c != '\n');
            if(ferror(fh))
                die("fgetc");
            
            continue;
        }
        
        // Zeile als Dreieck lesen und in Struktur speichern
        struct triangle tri;
        bool success = parseTriangle(line, &tri);
        
        if(!succes){
            fprintf(stderr, "Invalid line format!\n");
            continue;
        }
        
        struct triangle *trim = malloc(sizeof(struct triangle));
        if(!trim)
            die("malloc");
        
        *trim = tri;
        qPut(work_queue, trim); // gefundenes Dreieck in Queue einfügen.
    }
    
    // Threads + Ressourcen aufräumen
    fclose(fh);
    for(size_t i = 0; i<CALC_THREADS; i++){
        qPut(work_queue, NULL);
        // Für jeden RechenThread NULL einfügen, jeder kann dann eins entfernen und sich richtig beenden.
    }
    
    for(size_t i = 1; i<=CALC_THREADS; i++){
        errno = pthread_join(ts[i], NULL); // Rechenthreads explizit mit deren ThreadID aufsammeln und aufräumen
        if(errno != 0)
            die("pthread_join");
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
// Ende Hauptfunktion



// Funktion Rechenthread
static void* calc_thread(void *arg){
    // Soll rechnen, bis er nichts mehr aus der Queue holen kann
    while(42){
        struct triangle *tri = qGet(work_queue); // Element aus Queue holen
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
