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
#define SIZE (1024*1024*1)   // 2^20  =   1 MiBytes  就是 题目里卖弄规定的 大小

///这是 定义的 memory block，就是 一个节点，目的是 服务于 后面盛放内容的空间， 这个节点自己本身不盛放用户定义的内容（比如用户想要 放一个 string），
// Memory-chunk structure.
struct mblock {
	struct mblock *next; // 里面存放的是 MAGIC
	size_t size;//  存放 die Größe von nachfolgende Block
	char storage[]; // 这里面 存的是 跟 die Adresse von allocated Speicher（ 比如m1， 见Ubung page13） 有关
};

///这就是 一整个 区域 Heap-memory area.
static char memory[SIZE];

/// Pointer to the first element of the free-memory list.
//  之后这个 head pointer 本身 要immer akualisirt werden（因为 被分配 出去的空间 不再在likned list了， 所以 header 本身也要 更新），在当有新的 Speicher 被 malloc 之后.
static struct mblock *head;

/// Helper function to visualise the current state of the free-memory list.
// printList 的作用就是：
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

    //malloc() 里面的参数是 0， 根据题意：wird malloc() mit der Größe 0 aufgerufen, dann liefert malloc() einen  NULL-Pointer zurück
    if(size == 0)
        return NULL;

    /**
     * head-Element initialisieren, nur dann, wenn help->next != MAGIC, weil:
     * - head == NULL reicht nicht aus, denn wenn alle Speicherbereiche herausgegeben sind, gibt es keinen freien Speicher mehr, der verwaltet werden muss, weshalb es auch kein Listenelement mehr in der Liste gibt und head zeigt folglich auf NULL.
     * - wird jetzt überprüft, ob head->next != MAGIC ist, wissen wir, falls nicht true, dass head->next == MAGIC, demnach ist in der Liste ein Element enthalten, was gerade herausgegeben ist
     * - ist head->next != MAGIC, ist der Speicherbereich noch nicht für unsere Liste optimiert worden und head ist tatsächlich noch nicht initialisiert
     *
     * Andere Möglichkeit: eine static Variable mit Überprüfung von var == false, die in der ersten Runde bei der Initialisierung des heads dann auf true gesetzt wird.
     */
    /**
     * 把这个 1M 空间的heap 想象成为 Ubung 里面的 长条。  *memory 是这个 heap 的开始 pointer，  *header 刚开始
     * 和 memory 重合， 在 多次 malloc 之后， 长条里面*header 会被顶到最上面， heap 里面根本没有 足够的 16bytes空间 来盛放 *header,
     * 更不用说再分配 内容 空间了。   所以 head == NULL 并不足够说明代表 head 不存在， 而是 空间不够而已。 所以 还要 判断 (struct mblock*)memory.next 是否是一个 MAGIC
     * 假如是一个 MAGIC， 说明 memory 后面的第一个空间早就 被 使用， 所以就会验证 *header 是存在的。
     *
    */
    if(head == NULL){
        struct mblock* help = (struct mblock*) memory;
        if(help->next != MAGIC){
            // 说明根本就没有 malloc 过，（因为malloc() 过之后，负责 “内容区域“ 的那个 struct mblock 里面就会带上MAGIC ）
            // 说明 header 根本就不存在 header， 那就initialize 一个header
            // init head, empty list
            help->size = SIZE - (sizeof(struct mblock));
            help->next = NULL;     // 看 Ubung slide page 10， 这就是一个 struct mblock的 初始状态， size是heap
                                  //剩余的空间， next 是 NULL
            // 更新head
            head = help;
        }
    }



    //  现在 *head  找了一个替身， 为malloc 一块内容 做准备
    struct mblock *lauf = head; // Laufzeiger
    // pre_next里面存的是 head pointer 的地址，  *(prev_next) = *header , 即header pointer 本身
    struct mblock** prev_next = &head; // Schleppzeiger(拖拉指针), hilfreich bei der Verkettung, wenn ein Element herausgenommen wird.

    // 问题： 为什么有这么多 struct mblock 来供 lauf->next,  因为可能之前 malloc 了很多次， 也free 了很多次， 但是在free 之后， malloc 之前创建的 struct block 并不会
    // 消失， 而且其中还保存着 之前 负责的 “内容块” 的 size， 这也就是为什么可以获得  lauf->size
    // Prüfe, ob es ein Element mit genügend speicher gibt.
    // lauf != NULL && lauf->size < size 这个条件就是： 我还有一个 struct mblock,到那时 这个struct mblock后面 的 size 不够 你要求的 size 了。
    // Schleife läuft, bis entweder 一个合适的空间 gefunden wurde, oder läuft bis zum Ende, falls es keins gibt.
    while(lauf != NULL && lauf->size < size){
        // case1: 能走这一步， lauf = NULL 是 不可能的， 所以这里的 lauf != NULL 是一个 多余的条件
        // case2: 比方说，选了老大， 老大不可以(lauf->size < size)， 接着看老二 也不可以， 但是 老三可以（lauf->size > size,  空间足够大），
        // 由于 老三可以(lauf->size > size), 所以while循环 break， lauf 变成了 lauf->next
        prev_next = &(lauf->next);
        lauf = *prev_next;
    }

    // 当 在while循环里面 遍历完全 之后， 发现 所有的 lauf->size 都 小于 要求的 size， 所以 在最后一次循环中，
    //   prev_next = &(lauf->next); 这时候的 lauf已经是最后 一个 struct mblock了， lauf->next = NULL,  所以 lauf = lauf->next = NULL， while 循环因为case1（lauf 是 NULL） 被打断

    // Am Ende, wenn es kein Element gibt, ist lauf == NULL, weil es kein nächstes Element mehr gab.
    // Wir haben also keinen ausreichend großen Speicherbereich gefunden -> errno setzen.
    // 当lauf = NULL 的时候， 说明遍历到最后一个 struck mblock了，也没有找到 ， 所以空间不足
    if(lauf == NULL){
        errno = ENOMEM;
        return NULL;
    }

    // 走到这一步， 就说明 lauf 就是我们需要得 struct mblock,
    // Falls nicht genug Speicher zum Aufteilen vorhanden ist, geben wir den ganzen Block heraus.
    // Der vorherige Next-Pointer zeigt nun auf das nachfolgende Element.
    // Das Element, das den für uns passenden Speicher verwaltet, ist also nicht mehr in der Verkettung (wurde einfach übersprungen).
    // 这个block的大小只够 存 这 10bytes 的，剩余的空间(falls vorhandeln)， 不够再开发了， 所以将这个整个block 踢出去。
    // *prev_next 就是 head pointer， 因为 lauf 这个整个 block 被踢了出去(从linked list)，所以就不考虑lauf了，直接更新 header 指向 lauf->next
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

        //lauf->storage 就是一个地址(相当于 base)，lauf->storage + size 里面得size 相当于 偏移量。
        struct mblock* neu = (struct mblock*) (lauf->storage + size);

        // siehe Beispiel: neue Größe =  gefundene Größe (=25) - gewünschte Größe (=10) - Blockgröße (=5, im Beispiel)
        // 下面计算剩下得大小。
        neu->size = lauf->size - size - sizeof(struct mblock); // sizeof(struct mblock) 是新 struct mblock 大小。
        neu->next = lauf->next; // Verkettung herstellen， 因为 lauf 被踢出去了，在被踢出去之前， 把 lauf->next 下一个元素交接给 neu->next，表明 lauf 这个 lock完成使命， 被踢出去， neu 接上， 以便list不断
        // *prev_next 就是 header pointer， 现在更新 header， 见 Uebung page 11
        *prev_next = neu; // Verkettung herstellen ， 这个*prev_next 就是 最新状态的header， 里面的size 是 25 - 10 -5 = 10， 说明这个 header 指向的mblock 可以盛放 10 个 byte
    }

    // Element als herausgegeben markieren
    lauf->next = MAGIC; // 这里得lauf 就是 被踢出去的block, 对 10 byte 负责， 所以加上magic 标志。 已经被从list （list 里面只能是 frei blocks）踢出去了

    /**
     * 总结：
     * lauf->next 里面分两种情况 存储：
     * 假如你 lauf是 frei block， 应该被 insert 到list 中， lauf->next 就是存的是 下一个 加入的element，
     * 加入lauf 是个 负责 10bits  的 block， lauf->next 就是 MAGIC， 不会被加入到list 里面。
    */

    // “Zeiger auf Speicher” zurückgeben
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
        mbp->next = head; // 把 MAGIC 抹掉， 换成 header， header 这是在上面
        head = mbp; // header 更新， header 下来了
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
