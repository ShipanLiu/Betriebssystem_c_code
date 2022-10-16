// Übung 31.05.2022
// Übungsgruppe 02

/*
 * Mit -O0 -g kompilieren, um Debuggen zu ermöglichen.
 */

#include <stdio.h>

static void initArray(long *array, size_t size){
    for(size_t = 0; i <= size; i++)
        array[i] = 0;
}

int main(void){
    long buf[7];
    long *array = buf;
    
    initArray(buf, sizeof(buf)/sizeof(long));
    
    while(array != buf + sizeof(buf)/sizeof(long)){
        printf("%d\n", *array);
        array++;
    }
}
