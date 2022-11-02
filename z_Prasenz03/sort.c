/*
这是 Ubung3 上课老师讲的例子。


 */


#include <stdio.h>
#include <stdlib.h>
#include "sort.h"

#define SIZE 5 // Warum ist es an dieser Stelle sinnvoll ein Makro zu verwenden? Was ist der Vorteil im Vergleich zu einer Variablen?

int main(int argc, char** argv) {
    int * arr = malloc(SIZE * sizeof(int));   //free + fehlerbehandlung

    if(arr == NULL) {
      perror("malloc");
    }

    for (int i = 0; i < SIZE; i++) {
        arr[i] = i; // Was ist eine alternative Schreibweise für arr[i]?    *(arr+i)   ==扩展=>   *((char*)arr+i) = '\0'
    }

    print(arr, SIZE);

    // int (*func)(const void *, const void *) = some_sorting_function; // Beispiel für die Deklaration eines Funktionszeigers.
    qsort(arr,SIZE,sizeof(int),some_sorting_function); // wie sieht der Funktionsaufruf aus?

    print(arr, SIZE);

    free(arr)

    return EXIT_SUCCESS;
} // Was fehlt in dieser Funktion (es sind mehrere Dinge)?  Fehler behandlung

static int some_sorting_function(const void * a, const void * b) {
    const int * int_a = (const int*)a;  // Was steht in den Pointern a bzw. b und was passiert hier damit?
    const int * int_b = (const int*)b;

    if(*int_a < *int_b) {
        return -1;
    }

    else if (*int_a > *int_b) {
        return 1;
    }
    else {
        return 0;
    }
} // Warum funktioniert diese Funktion nicht korrekt?
// Wie muss sie geändert werden, damit sie umgekehrt sortiert?
// use static, local werden, nicht von anderen c file aufrufen
static inline void print(int * arr, int arr_size) { // Wofür sorgt das "inline"(sofort aufrufen)? Funktioniert die Funktion auch ohne dieses Keyword?
    for (int i = 0; i < arr_size; i++) {
        printf("%d ", arr[i]);
        if(fflush(stdout) == EOF) { // Warum benötigen wir fflush()? Warum kann printf() fehlschlagen(speicher voll)?
            perror("printf in print()");
            // Warum benötigen wir hier kein weiteres fflush() (es gibt zwei Gründe)?  exit + error 都会自动fflush
            // In welchem Fall würden wir eines benötigen?
            exit(EXIT_FAILURE); // Warum sollte man exit() nicht in "Bibliotheksfunktionen" verwenden? Welches Keyword sollte in der Signatur dieser Funktion ergänzt werden?
        }
    }

    printf("\n");

    if(fflush(stdout) == EOF) {
        perror("second printf in print()");
        exit(EXIT_FAILURE);
    }
}