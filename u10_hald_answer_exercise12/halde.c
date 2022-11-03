/**
 * Aufgabe4 - halde:
 * - malloc()
 *      - ersten freien Bereich in Freispeicherliste suchen, der für die angeforderte Speichermenge groß genug ist
 *      - diesen Bereich aus Freispeicherliste entfernen
 *      - ist der Bereich größer als benötigt und größer als für ein Listenelement nötig, wird der Speicherbereich geleit und der Rest durch ein neues Listenelement verwaltet
 *      - herausgenommenes Listenelement: als next-Pointer wird eine Magic Number 0xbaadf00d eingetragen
 *      - liefert Zeiger auf Bereich hinter dem Listenelement (Element mit dem Magic-Wert) zurück
 *      - malloc(0) -> liefert NULL-Pointer
 * - free()
 *      - hängt freizugebenden Bereich vorne in die Liste ein, keine Verschmelzung
 *      - vorher auf Magic-Wert prüfen, erst dann darf der Speicher freigegeben werden
 *      - kein Magic-Wert? -> abort();
 *      - free(NULL) -> kehrt ohne Fehler zurück
 * - realloc()
 *      - auf malloc() + memcpy() + free() abbilden
 *      - d.h. Speicher mit neuer Größe anfordern, alten Inhalt kopieren, alten belegten Speicher freigeben
 *      - realloc(NULL, size); = malloc(size);
 *      - neuen Speicher allokieren schlägt fehl -> return NULL, errno gesetzt
 * - calloc()
 *      - Speicher mit malloc() anfordern, Byteweise mit 0x0 initialisieren
 */

#include "halde.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/// Magic value for occupied memory chunks.
#define MAGIC ((void*)0xbaadf00d)

/// Size of the heap (in bytes).
#define SIZE (1024*1024*1)

/// Memory-chunk structure.
struct mblock {
	struct mblock *next;
	size_t size;
	char storage[];
};

/// Heap-memory area.
static char memory[SIZE];

/// Pointer to the first element of the free-memory list.
static struct mblock *head;

/// Helper function to visualise the current state of the free-memory list.
void printList(void) {
	struct mblock *lauf = head;

	// Empty list
	if (head == NULL) {
		char empty[] = "(empty)\n";
		write(STDERR_FILENO, empty, sizeof(empty));
		return;
	}

	// Print each element in the list
	const char fmt_init[] = "(off: %7zu, size:: %7zu)";
	const char fmt_next[] = " --> (off: %7zu, size:: %7zu)";
	const char * fmt = fmt_init;
	char buffer[sizeof(fmt_next) + 2 * 7];

	while (lauf) {
		size_t n = snprintf(buffer, sizeof(buffer), fmt
			, (uintptr_t) lauf - (uintptr_t)memory, lauf->size);
		if (n) {
			write(STDERR_FILENO, buffer, n);
		}

		lauf = lauf->next;
		fmt = fmt_next;
	}
	write(STDERR_FILENO, "\n", 1);
}


void *malloc (size_t size) {
    
    /**
     * head-Element initialisieren, nur dann, wenn help->next != MAGIC, weil:
     * - head == NULL reicht nicht aus, denn wenn alle Speicherbereiche herausgegeben sind, gibt es keinen freien Speicher mehr, der verwaltet werden muss, weshalb es auch kein Listenelement mehr in der Liste gibt und head zeigt folglich auf NULL.
     * - wird jetzt überprüft, ob head->next != MAGIC ist, wissen wir, falls nicht true, dass head->next == MAGIC, demnach ist in der Liste ein Element enthalten, was gerade herausgegeben ist
     * - ist head->next != MAGIC, ist der Speicherbereich noch nicht für unsere Liste optimiert worden und head ist tatsächlich noch nicht initialisiert
     *
     * Andere Möglichkeit: eine static Variable mit Überprüfung von var == false, die in der ersten Runde bei der Initialisierung des heads dann auf true gesetzt wird.
     */
    if(head == NULL){
        struct mblock* help = (struct mblock*) memory;
        if(help->next != MAGIC){
            // init head, empty list
            help->size = SIZE - (sizeof(struct mblock));
            help->next = NULL;
            
            head = help;
        }
    }
    
    if(size == 0)
        return NULL;
    
    
    struct mblock *lauf = head; // Laufzeiger
    struct mblock **prev_next = &head; // Schleppzeiger, hilfreich bei der Verkettung, wenn ein Element herausgenommen wird.
    
    
    // Prüfe, ob es ein Element mit genügend speicher gibt.
    // Schleife läuft, bis entweder eins gefunden wurde, oder läuft bis zum Ende, falls es keins gibt.
    while(lauf != NULL && lauf->size < size){
        prev_next = &(lauf->next);
        lauf = *prev_next;
    }
    
    // Am Ende, wenn es kein Element gibt, ist lauf == NULL, weil es kein nächstes Element mehr gab.
    // Wir haben also keinen ausreichend großen Speicherbereich gefunden -> errno setzen.
    if(lauf == NULL){
        errno = ENOMEM;
        return NULL;
    }
    
    // Falls nicht genug Speicher zum Aufteilen vorhanden ist, geben wir den ganzen Block heraus.
    // Der vorherige Next-Pointer zeigt nun auf das nachfolgende Element.
    // Das Element, das den für uns passenden Speicher verwaltet, ist also nicht mehr in der Verkettung (wurde einfach übersprungen).
    if( (lauf->size - size) <= sizeof(struct mblock)){
        *prev_next = lauf->next;
    } else {
        // Fall der Speicher groß genug ist, um ihn aufzuteilen.
        // Neuer Block liegt direkt hinter dem angeforderten Speicher:
        /**
        Bsp. malloc(10);
        - angenommen Listenelement ist 5 Bytes groß
            || gefundenes Element X, verwaltet 25 Byte                                 ||              25 Bytes, die von dem Element X verwaltet werden                    ||
            || gefundenes Element X, verwaltet 25 Byte, wir brauchen 10     || hier sind 10 Bytes || nächstes Element Y für Rest | Rest = 25-10-5 = 10  ||
                                                          ^ lauf->storage ist hier
                                                                        ^ lauf-storage + size ist hier (size = 10) -> ist der Ort für das nächste Element
         */
        struct mblock* neu = (struct mblock*) (lauf->storage + size);
        
        // siehe Beispiel: neue Größe =  gefundene Größe (=25) - gewünschte Größe (=10) - Blockgröße (=5, im Beispiel)
        neu->size = lauf->size - size - sizeof(struct mblock);
        neu->next = lauf->next; // Verkettung herstellen
        
        *prev_next = neu; // Verkettung herstellen
    }
    
    // Element als herausgegeben markieren
    lauf->next = MAGIC;
    
    // Zeiger auf Speicher zurückgeben
    return lauf->storage;
}

void free (void *ptr) {
    
    if(ptr == NULL)
        return;
 /*
    | mblock | speicher |
    ^mbp     ^ptr
  
  - ptr zeigt auf den Speicherbereich, der freigegeben werden soll
  - wir brauchen den dazugehörigen MBlock
  - wie folgt erhalten wir den Pointer, beide Varianten (1 und 2) sind äquivalent
 */
    // Variante 1: struct mblock *mbp = (struct mblock*)((size_t) ptr - sizeof(struct mblock))
    struct mblock *mbp = (struct mblock *)ptr - 1; // Variante 2
    
    // Element vorne einfügen
    if(mbp->next == MAGIC){
        mbp->next = head;
        head = mbp;
    }
    
    abort();
}

void *realloc (void *ptr, size_t size) {
    
    // laut Aufgabenstellung
    if(ptr == NULL){
        return malloc(size);
    }
    
    // laut Aufgabenstellung
    if(size == 0){
        free(ptr);
        return NULL;
    }
    
    // Abbilden auf: malloc+memcpy+free
    // Fehlerbehandlung nicht vergessen
    void *new_ptr = malloc(size);
    if(new_ptr == NULL)
        return NULL;
    
    /**
     * Hier dürft ihr nur die kleinere Größe von beiden bei memcpy angeben,
     * 1. Fall: ihr gebt standardmäßig die Größe des vorherigen Speicherbereichs an memcpy()
     *  - >  wenn der neue Speicherbereich kleiner ist, kopiert memcpy() zu viel Speicher in zu wenig Platz
     * 2. Fall: ihr gebt standardmäßig die Größe des neuen Speicherbereichs an memcpy()
     *  - >  wenn der alte Speicherbereich kleiner ist, kopiert memcpy() zu viel Speicher, der nicht zum alten Bereich gehörte
     */
    struct mblock *mbp = (struct mblock *)ptr - 1;
    size_t smallerSize = mbp->size;
    if(smallerSize > size)
        smallerSize = size;
    
    new_ptr = memcpy(new_ptr, ptr, smallerSize);
    free(ptr);
    return new_ptr;
}

void *calloc (size_t nmemb, size_t size) {
    
    // catch overflow
    size_t bytes = nmemb * size;
    if(size != 0 && bytes / size != nmemb){
        errno = ENOMEM;
        return NULL;
    }
    
    // malloc+memset
    void *ptr = malloc(bytes);
    if(ptr != NULL){
        memset(ptr, 0, bytes);
        return ptr;
    }
    
    return NULL;
}
