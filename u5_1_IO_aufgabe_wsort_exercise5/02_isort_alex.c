#include <stdio.h>
#include <stdlib.h> // <-- qsort, EXIT_SUCCESS..., rand...  true 这个值 可以写while(true) 代替 while(1)
#include <time.h> // time -> Nondeterministisch

#define NUMBER_COUNT 15
#define NUMBER_RANGE 10

/* Hilfsfunktion zum Ausgeben des Arrays */
static void printArray(int* nums)
{
    for(int i = 0; i < NUMBER_COUNT; ++i)
    {
        /* Fehlerbehandlung für Ausgaben */
         //  printf 的返回值是int
        if(printf("%i ", nums[i]) < 0)
        {
            perror("printf");
            /* exit in Funktionen außerhalb der main *
             * notwendig um das Programm zu beenden  */
            exit(EXIT_FAILURE);
        }
    }
    /* Fehlerbehandlung für Ausgaben */
    // putchar 就是往 console 上面 写一个char
    // 最后 要往 console 上面 写入一个 \n ， 假如这一步 出错
    if(putchar('\n') == EOF)
    {
            perror("putchar");
            exit(EXIT_FAILURE);
    }

}

/* Callback Funktion, die intern von qsort aufgerufen wird */
//  这是标准的 compare 函数， 从 slides 上面 抄过来的
static int compareInt(const void* a, const void* b)
{
    int ai = *(const int*)a;
    int bi = *(const int*)b;

    /* Vorzeichen bestimmen Sortierreihenfolge */
    if(ai < bi) return -1;
    if(ai > bi) return +1;
    return 0;
}

int main(int argc, char* argv[])
{
    // Zufallszahlgenerator initialisieren
    //srand(time(NULL)); // Extra: Nondeterministisch
    //srand(4711); // Deterministisch

    /** Array mit Zufallszahlen erstellen **/

    /* Array allokieren, nicht initialisiert */
    int* nums = malloc(NUMBER_COUNT*sizeof(int));  // 产生 15 个int 的空间
    // 假如产生空间出错， 说明 不够 15 个 int的位置
    if(nums == NULL) /* Überprüfen, ob noch genügend *  
                      * Speicher vorhanden ist       */
    {
        perror("Malloc fehlgeschlagen");
        /* In der main kann sowohl exit als *
         * auch return verwendet werden     */
        return EXIT_FAILURE;
    }

    /* Zufallszahlen generieren und in das Array schreiben*/
    // 产生 15 个 随机数 范围是 0-9
    for(int i = 0; i < NUMBER_COUNT; ++i)
    {
        nums[i] = rand() % NUMBER_RANGE;
    }

    printArray(nums);
    
    /** Sortieren des Arrays **/

    /* Sortieren des Arrays, vorsichtig: Parameter nicht verwechseln *
     * compareInt oder &compareInt -> Funktionszeiger                */
    qsort(nums, NUMBER_COUNT, sizeof(int),
          &compareInt);

    printArray(nums);

    /** Cleanup **/

    /* Speicher freigeben!!! */
    free(nums);

    /* Ausschreiben der verbleibenden Bytes im Ausgabepuffer *
     * Fehlerbehandlung auch hier notwendig                  */
    //  假如 fflush（stdout） 出错了， 那也需要 fehlerbehandlung
    if(fflush(stdout) == EOF)
    {
        perror("fflush");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
