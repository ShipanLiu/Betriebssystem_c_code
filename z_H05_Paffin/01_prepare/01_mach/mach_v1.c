#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <pthread.h>
#include <stdbool.h>
#include <string.h>

#include "queue.h"
#include "run.h"
#include "sem.h"

#define FLAG_RUNNING    1
#define FLAG_OUTPUT     2
#define FLAG_POISON     3

static SEM* sem_wait;
static SEM* sem_limit;

static void die(const char *s) {
    perror(s);
    exit(EXIT_FAILURE);
}


// 这个 函数 是提前给定的，接受一个 字符串， 把 里面的 positive number 提炼出来。字符串的部分可以要， 也可以不要
// 其中，参数string是要转换的字符串，end_ptr是字符串的指针，该指针指向了参数string中没有被转换的部分，
// 如果不需要该参数，可以将其设置为NULL；参数base表示转换的进制。如果转换成功，
// strtol()函数的返回值是转换后的long类型的值，如果转换失败，则返回值是0。
static int parse_positive_int_or_die(char *str) {
    errno = 0;
    char *endptr;


    long x = strtol(str, &endptr, 10);
    if (errno != 0) {
        die("invalid number");
    }
    // Non empty string was fully parsed
    if (str == endptr || *endptr != '\0') {
        fprintf(stderr, "invalid number\n");
        exit(EXIT_FAILURE);
    }
    if (x <= 0) {
        fprintf(stderr, "number not positive\n");
        exit(EXIT_FAILURE);
    }
    if (x > INT_MAX) {
        fprintf(stderr, "number too large\n");
        exit(EXIT_FAILURE);
    }
    return (int)x;
}


/********** write you code here***********/

// 这是 output thread 的 function
static void *thread_output(void *unused) {
    (void)unused;

    // Whileschleife, die die Ausgabe aller Arbeiterthreads ausgegeben werden soll
    while (true) {
        char *cmd, *out;
        int flags;

        // 就是 输出 cmd 的状态 + out，  根据 flags，  running是 1， output 是 2， poison 是 3
        // 关键就是 搞清楚 queue_get() 是 干什么的。
        if (queue_get(&cmd, &out, &flags)) {
            die("queue_get");
        }

        /* poison pill */
        if (flags == FLAG_POISON) {
            break;
        }

        if (flags == FLAG_RUNNING) {
            if (printf("Running `%s` ...\n", cmd) < 0) {
                die("printf");
            }

        } else {
            if (printf("Completed `%s`: \"%s\"\n", cmd, out) < 0) {
                die("printf");
            }
        }
        free(cmd);
        free(out);
    }
    return NULL;
}

static void *thread_work(void *arg) {

    char *cmd = arg;

    errno = pthread_detach(pthread_self());
    if (errno) {
        die("pthread_detach");
    }

    char *cmd2 = strdup(cmd);
    if (!cmd2) {
        die("strdup");
    }

    if (queue_put(cmd2, NULL, FLAG_RUNNING)) {
        die("queue_put");
    }

    char *out;
    run_cmd(cmd, &out);

    if (queue_put(cmd, out, FLAG_OUTPUT)) {
        die("queue_put");
    }

    V(sem_wait);
    V(sem_limit);
    return NULL;
}

int main(int argc, char **argv) {

// 应该长这样:        ./mach 7 my_mach_file.txt       , 发现 参数的 数量 必须是 3 个
    if (argc != 3) {
        fprintf(stderr, "usage: %s <threads> <mach file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    //  你需要给 thread 的数量，万一你给的 是一个字符串的 数字，
    // 需要用 给定的parse_positive_int_or_die()来解析一下
    int threads = parse_positive_int_or_die(argv[1]);

// sem_lock() returns -1 with error, returns 0 with success
    if (sem_lock()) {
        die("queue");
    }

    sem_wait = semCreate(0);
    if (!sem_wait) {
        die("semCreate");
    }

// 技巧： 把sem_limit的值 设置 成 thread 的数量。 不讲武德
    sem_limit = semCreate(threads);
    if (!sem_limit) {
        die("semCreate");
    }

    FILE *fh = fopen(argv[2], "r");
    if (!fh) {
        die(argv[2]);
    }

    // 先创建 一个 output thread.  负责输出
    pthread_t tid_output;
    errno = pthread_create(&tid_output, NULL, thread_output, NULL); // liefert errno-Wert oder 0 zurück
    if (errno) {
        die("pthread_create");
    }

    // Zähler, wie viele Threads gestartet wurden
    size_t threads_started = 0;
    char buf[4096+1];// 每一行word 最长 4096 再加上 '\0'

    while (fgets(buf, sizeof(buf), fh)) {
        // falls es eine Leerzeile ist, sind alle Befehle der aktuellen Gruppe eingelesen
        // strcmp的返回值为 0， 假如 buf 和 ‘\n’相等的话。
        // 说明是 读到了最后一行了。
        if (!strcmp(buf, "\n")) {
          // 假如 是 个 leerzeile， 才会进入
            while (threads_started > 0) {
                P(sem_wait);  // sem_wait 的初始值 是0， 会卡在这里， 需要 V 来释放。
                threads_started--;
            }
            continue;
        }

        // 假如不是 leerzeile， 则会进行：

        strtok(buf, "\n"); // strtok entfernt das \n aus dem Buf
        char *x = strdup(buf);
        if (!x) {
            die("strdup");
        }

        // 这一行之后， 就会开始创建 thread， 执行每一行的 命令。
        //  当 sem_limit  里面的值 不够的时候，就会一直卡在这里， 不会 在创建新的 thread 了。
        P(sem_limit); // Diese Semaphore wird inkrementiert. Ein Thread arbeitet nun in einem kritischen Bereich bzw. es ist festgelegt, wie viele Threads parallel arbeiten dürfen.

        // 从这里开始 是 critical area, 上面有 P() 进行保护。
        threads_started++;

        pthread_t tid;
        errno = pthread_create(&tid, NULL, thread_work, x);
        if (errno) {
            die("pthread_create");
        }
    }//end-while

    if (ferror(fh)) {
        die("fgets");
    }
    fclose(fh);


    while (threads_started > 0) {
        P(sem_wait); // 一直 会 卡在这里。 推测， 一个 thread  完事之后，会 V(sem_wait), 这里是 main 函数 ，
                     // main 里一直记录  thread_started 的数量。 threads_started--,  目的是 最后保证 没有 thread_started,
                     // 全部 thread 已经死亡（当然不包括  output thread）
        threads_started--;
    }


    // Alle Ausgaben fertig gespeichert, wird so nun in der Warteschlange vermerkt.
    if (queue_put(NULL, NULL, FLAG_POISON)) {
        die("queue_put");
    }

    // Warten, bis der Ausgabethread fertig ist.
    // output thread  不能用 pthread_detatch， 要让他 一直 活到最后， 然后 手动 终结
    errno = pthread_join(tid_output, NULL);  // NULL表示 我不需要 返回值。
    if (errno) {
        die("pthread_join");
    }

    // optional
    if (fflush(stdout)) {
        die("fflush");
    }

    queue_deinit();
    semDestroy(sem_wait);
    semDestroy(sem_limit);

    return EXIT_SUCCESS;

}
