#include "queue.h"
#include <errno.h>
#include <stdlib.h>

#include "sem.h"

static SEM *sem_lock; // 1
static SEM *sem_notify; // 0

// 为了 创建 list 而创建
struct elem {
    struct elem *next;  // Verkettungszeiger
    char *cmd;          // Speicher für aktuellen Befehl
    char *out;          // Speicher für Ausgabe
    int flags;          // Optionale Flags
};
static struct elem* queue; // Listenkopf

int queue_init(void) {

    // Ein Thread gleichzeitig darf auf der Queue arbeiten.
    sem_lock = semCreate(1);
    if (!sem_lock) {
        return -1;
    }

    // Hilfs-Semaphor, Elemente können erst aus der Queue entfernt werden, sobald welche hinzugefügt wurden.
    sem_notify = semCreate(0);
    if (!sem_notify) {

        // 假如你 sem_notify 创建不成功的话， 拿就把之前 创建的 sem_lock 也销毁，摆烂
        int old_errno = errno;
        semDestroy(sem_lock);
        errno = old_errno;
        return -1;
    }
    return 0;
}

// 全部消灭 sem
void queue_deinit(void) {

    semDestroy(sem_lock);
    semDestroy(sem_notify);

}

// Diese Funktion fügt ein Element in die Warteschlange ein.
// Ein Warteschlangenelement entspricht einer Ausgabe.
int queue_put(char *cmd, char *out, int flags) {

    struct elem *x = malloc(sizeof(*x));
    if (!x) {
        return -1;
    }
    x->next = NULL;
    x->cmd = cmd; // x->cmd 本身就是 pointer， pointer = pointer 是没毛病的。
    x->out = out;
    x->flags = flags;

    // hier beginnt ein kritischer Bereich
    P(sem_lock);

    // queue 是 list 的 header， 假如 header 不存在的话， 那就 queue = x
    if (queue == NULL) {
        queue = x;
    } else {
        struct elem *y = queue;
        // y 是 queue 的替身， 一直loop 到最后， 然后把 x 加到最后
        while (y->next != NULL) {
            y = y->next;
        }
        y->next = x;
    }
    V(sem_lock);    // Kritischer Bereich wird freigegeben. Thread verlässt diesen Bereich.

    V(sem_notify);  // In queue_get() kann erst ein Element entfernt werden, sobald ein Element hinzugefügt wurde. P(sem_notify) wartet, bis V(sem_notify) aufgerufen wurde.
                    // 从而让 queue_get() 不再 处于 堵塞的状态，

    return 0;
}

// Diese Funktion holt ein Element (das älteste = Kopf) aus der Queue und schreibt die Werte in die übergebenen Pointer.
// 看来是一个 FIFO 的system。
int queue_get(char **cmd, char **out, int *flags) {

    P(sem_notify);  // siehe queue_put()，一直 会block 在这里， 指导 queue_put()完事之后，
    P(sem_lock);    // Kritischer Bereich wird betreten, sem_lock wird nun dekrementiert und blockiert.
    // Dies ist ein kritischer Bereich, da nicht mehrere Threads gleichzeitig Elemente aus der Queue löschen sollten.
    //在把 老大（header） 移除之前， 让老二变成老大。
    struct elem *x = queue;
    queue = x->next;
    V(sem_lock);
    // Ende kritischer Bereich. sem_lock wird wieder erhöht und hat Platz für nächsten Thread.

    *cmd = x->cmd;
    *flags = x->flags;
    free(x);

    return 0;
}
