// done by shipan at 04.01.2023


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "halde.h"

/*
* Aufgabe: 4xmalloc, 4xfree, 4xmalloc, 4xfree, ein Randfall
* Randfälle:
*   - malloc(0)
*   - calloc(0, size)
*   - calloc(nmemb, 0)
*   -> gibt NULL oder einzigartigen Pointer wieder, der gefreed werden kann
*/

int main(int argc, char *argv[]) {
	printList();


  //四次 malloc  + frei geben

	char *m1 = malloc(200*1024);
	printList();

	char *m2 = malloc(200*1024);
	printList();

	char *m3 = malloc(200*1024);
	printList();

	char *m4 = malloc(200*1024);
	printList();

	free(m1);
	printList();

	free(m2);
	printList();

	free(m3);
	printList();

	free(m4);
	printList();





// 四次malloc + freigeben。
	m1 = malloc(100*1024);
	printList();

	m2 = malloc(100*1024);
	printList();

	m3 = malloc(100*1024);
	printList();

	m4 = malloc(100*1024);
	printList();

	free(m1);
	printList();

	free(m2);
	printList();

	free(m3);
	printList();

	free(m4);
	printList();

    // Randfall, Randfall 就是 badcase,   比如  gave malloc() a 0  比如  give ralloc a NULL pointer
    m1 = realloc(NULL, 1000*1024);
    printList();

	exit(EXIT_SUCCESS);
}
