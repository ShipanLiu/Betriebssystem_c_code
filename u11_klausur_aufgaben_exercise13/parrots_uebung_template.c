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

// Hauptfunktion (main)
    if(argc != 2) { usage (); }
    // Initialisierung



    // Threads starten


    // Arbeitspakete aus der Datei auslesen

    
    // "Haupt"schleife

    
    // Threads + Ressourcen aufräumen

// Ende Hauptfunktion



// Funktion Rechenthread

// Ende Rechensthread



// Ausgabethread

// Ende Ausgabethread
