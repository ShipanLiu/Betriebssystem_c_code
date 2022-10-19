// Übung 11, T02
// 21.06.2022

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include "sem.h"
#include "list.h"  //这个就是本身 linux 自带的一个库


/*这里是 函数声明*/ 

static void usage(void);

// 发生错误， 调用perror（） 和 exit（）
static void die(const char *msg);

// err() 的目的： 在发生错误 且 error 不被设置的情况下， 不能使用perror， 而是用 fprintf(stderr, ".....")
static void err(const char *msg);
// die search() Funktion ist for the thread to use
static void *search(void *arg);

/**
 * Aufgabenstellung:
 * Parallel mehrere Dateien zeilenweise nach einem Suchstring durchsuchen. 
 * Gefundene Zeilen werden in eine Liste eingefügt und vom Hauptthread ausgegeben.
 * 
 * 就是我要在 不同多个 datei 里面找一个指定的 string， 每一datei 应该被分配一个 thread， 独立寻找这个 string
 *
 * Aufruf:
 * ./synchro_demo <zu suchendes Wort> <zu durchsuchende Datei_1> ... <zu durchsuchende Datei_n>
 */

/**
 * Funktionen aus den Folien:
 * - int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *args);
 * - pthread_t pthread_self(void);
 * - void pthread_exit(void *retval);
 * 
 * - int pthread_join(pthread_t thread, void **retvalp);
 * - int pthread_detach(pthread_t thread);
 *
 * - SEM *semcreate(int initVal);
 * - void P(SEM *sem);
 * - void V(SEM *sem);
 * - void semDestroy(SEM *sem);
 */

// TODO: global variables, defines, etc.

static SEM *sem_mutex; // gegenseitiger Ausschluss, Sem um kritische Abschnitte zu schützen
static SEM *sem_found; // Signalisierung, Sem um den Hauptthread zu informieren, dass etwas in der Liste zur Ausgabe bereit steht.

// 自定义一个结构体，名字叫： pthread_args  ， 作用：作为 pthread_create 的第四个参数， 主要利用里面的 char* file
typedef struct pthread_args{
    char *file;
} pthread_args;


static char *search_pattern;
static int active_threads;

#define LINE_MAX (4094 + 1 + 1)




int main(int argc, char *argv[]) {
	if(argc < 3) {
		usage();
	}
    
    // TODO: initialize variables


    sem_mutex = semCreate(1);
    if(sem_mutex == NULL)
        err("semCreate() failed");
    
    sem_found = semCreate(0);
    if(sem_found == NULL)
        err("semCreate() failed");

	search_pattern = argv[1];  // argv[0] 是 命令   argv[1] 是 要找的string， 这里命名为 search_pattern
	active_threads = argc-2;  // argc 是 main 参数的个数， 这里 argc-2   代表一共有多少个datei， 那就创建多少个 thread

    /**
     * Struktur-Array, um für jeden der erstellten Threads eine Struktur zu haben.
     * Dies dient dazu, um den Thread-start-routinen gleich ihre Argumente zu übergeben.
     * In der Struktur steht der Name der zu durchsuchenden Datei.
     */
    pthread_args args[active_threads];  // 因为在 使用pthread_create()的时候，里面的第四个参数 是 应该是一个 struktur 结构体
                                        // args[] 是一个数组， 里面盛放的是 pthread_args 类型（我们自定义的 struct）
    
	
    /**
     * In Schleife über alle Dateien laufen, die wir durchsuchen sollen.
     * Dabei wird das Struktur-Element mit dem Dateinamen initialisiert.
     * Dann wird eine Variable für die Thread-ID erstellt.
     * Dann wird ein Thread erzeugt. Dieser führt die Funktion "search()" mit dem Dateinamen in seiner Struktur durch.
     */
    for(int i=2; i<argc; i++){
        args[i-2].file = argv[i]; // 把datei的name 放进了 args[0] 里面
    
        pthread_t tid;
        errno = pthread_create(&tid, NULL, &search, &args[i-2]); // "search()"   will use   "args[i-2]"   as it's parameter
                                                                // pthread_create 成功返回0，顺便把 errno 初始化， 不成功返回 “错误值”，
                                                                // 正好把 errno 设置一下， so that yo can use "perror()"
        if(errno != 0)
            die("pthread_create");
    }
    
    /**
     * Main wartet, dass das Wort gefunden wurde.
     * -> Sie ruft P(sem_found) auf.
     * Dieser Sem ist am Anfang mit 0 initialisiert und entblockiert erst, wenn ein Thread etwas gefunden hat und V() aufgerufen hat.
     * Dieser Thread hat ein Element in eine Liste eingefügt. Das Element wollen wir nun aus der Liste herausnehmen.
     * Dafür müssen wir diesen Abschnitt mit dem sem_mutex schützen.
     * Wir erhalten die eingefügte Zeile als Rückgabewert von removeElement();
     * Diese Zeile geben wir dann aus.
     * Abbruchbedingung: nichts mehr in der Liste, dann verlassen wir die Schleife und räumen noch auf.
     */
    *
	char *line;
	do {
        P(sem_found);
        P(sem_mutex);
		line = removeElement();
        V(sem_mutex);
        
		if(line != NULL) {
			printf("%s", line);
			free(line);
		}
	} while(line != NULL);

	printf("done\n");
    
    semDestroy(sem_mutex);
    semDestroy(sem_found);
}



/*下面是 子 functions*/


 // soll nach den 'search_pattern' durchsuchen
static void *search(void *a) {
    
    // TODO: Pfad holen

    errno = pthread_detach(pthread_self); // Rückgabewert wird nicht benötigt, Thread räumt sich nach Terminierung selbst auf.
    if(errno != 0)
        die("pthread_detach()");
    
    pthread_args *arg = (pthread_args *)a;
    
	char *path = arg->file;

	FILE *file = fopen(path, "r");
	if(file == NULL) {
		die("fopen");
	}

    /**
     * InsertElement und removeElement sind Funktionen, die auf gloablen Ressourcen arbeiten, daher bilden sie einen kritischen Abschnitt und müssen geschützt werden.
     * Nur ein Thread darf an der Liste arbeiten, um die Konsistenz zu bewahren.
     * Wenn etwas gefunden wurde, signalisiert dieser Thread der main mit V(), dass etwas zur Ausgabe bereit steht.
     */
    
	char buf[LINE_MAX];
	while(fgets(buf, sizeof(buf), file) != NULL) {
		if(strstr(buf, search_pattern) != NULL) {  //  找到了，
            P(sem_mutex); // 占用 sem_mutex
			if(-1 == insertElement(buf)) {    // 插入到list 里面
				err("insertElement() failed");
			}
            V(sem_mutex); //释放 sem_mutex
            V(sem_found);
		}
	}

	if(ferror(file)) {
		die("fgets");
	}

	if(0 != fclose(file)) {
		die("fclose");
	}
    
    /**
     * Falls das, was wir suchen, in keiner Datei vorhanden ist, würde der Hauptthread ewig warten und nie terminieren.
     * Also sorgen wir dafür, dass der letzte Thread einmal mehr V() aufruft und den Hauptthread so in jedem Fall am Ende weckt.
     * Dieser nimmt ein Element aus der leeren Liste, bekommt NULL zurück, beendet die Schleife und räumt auf.
     */
    
    P(sem_mutex);
    active_thread--;
    if(active_threads == 0){
        V(sem_found);
    }
    V(sem_mutex);
    
    
	return NULL;
}

static void usage(void) {
    // 这个就是 在不设置 errno 的情况下 来 报错
	fprintf(stderr, "Usage: synchro <search-pattern> <file> [<file> ...]\n");
	exit(EXIT_FAILURE);
}

// errno gesetzt
static void die(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

// errno nicht gesetzt
static void err(const char *msg) {
	fprintf(stderr, "%s\n", msg);
	exit(EXIT_FAILURE);
}
