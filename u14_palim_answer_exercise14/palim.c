#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

// 我自己加的
#include <pthread.h>
#include <sys/stat.h>
#include <string.h>
#include "sem.h"


//statistics 就是你要 传输的所有的数据。
struct statistics {
	int lines;
	int lineHits;
	int files;
	int fileHits; // files 里找到 目标 hits
	int dirs;
	int activeGrepThreads;
	int maxGrepThreads; //负责 file
	int activeCrawlThreads; // 负责层层递进
};

// (module-)global variables
static struct statistics stats;
// TODO: add variables if necessary

// Semaphoren
static SEM* statsMutex;
static SEM* newDataSignal;
static SEM* grepThreadsSem;

static char* searchString; // Globalvariable 搜索关键字。


// function declarations
// Einstiegspunkt für die  "craw threads"  und ruft die Funktion processDir() zum Durchsuchen von path auf.
static void* processTree(void* path);
//
static void* processDir(char* path);
// 判读 是 regular file 还是 folder
static void* processEntry(char* path, struct dirent* entry);
// Einstiegspunkt for the "grep thread",  开始真正的寻找。
static void* processFile(void* path);
// TODO: add declarations if necessary


// if the "errno" is not set.
static void usage(void) {
	fprintf(stderr, "Usage: palim <string> <max-grep-threads> <trees...>\n");
	exit(EXIT_FAILURE);
}


// if the "errno" is set.
static void die(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

// helper function to help increase/decrease a variable which should be protected by a semaphore
static void incStat(int* variable) {
  P(statsMutex);
  *variable += 1;
  V(statsMutex);
}

static void decStat(int* variable) {
  P(statsMutex);
  *variable -= 1;
  V(statsMutex);
}


// helper function to print current statistics.
static void printStatistics() {
  P(statsMutex);
  int lines = stats.lines;
	int lineHits = stats.lineHits;
	int files = stats.files;
	int fileHits = stats.fileHits;
	int dirs = stats.dirs;
	int activeGrepThreads = stats.activeGrepThreads;
  V(statsMutex);

  //  当你 输出的 时候， 就没有必要 去 P 和 V 了， 因为 variable的 值 已经确定了
  int status = printf("%d/%d lines, %d/%d files, %d directories, %d active threads \r",
    lineHits,
    lines,
    fileHits,
    files,
    dirs,
    activeGrepThreads
  );

  if(status < 0) {
    die("printf");
  }

// 在 printf 之后，也要 fflush
  if(fflush(stdout) == EOF) {
    die("fflush");
  }




}





//         !!!!!!!!!!!!!!!!下面开始 main() !!!!!!!!!!!!!!!!!!!!!!!!!

/**
 * \brief Initializes necessary data structures and spawns one crawl-Thread per tree （说明会有 很多tree）
 * 这里的 tree 和 Uebung slide 里面的 tree 类似。
 * Subsequently, waits passively on statistic updates and on update prints the new statistics.
 * If all threads terminated, it frees all allocated resources and exits/returns.
 */
int main(int argc, char** argv) {
  //  ./palim <string> <max grep threads number> <trees>
	if(argc < 4) {
		usage();
	}

	// convert argv[2] (<max-grep-threads>) into long with strtol()
	errno = 0;
	char *endptr;
  //  strtol 就是 把数字部分 提取 到 stats.maxGrepThreads 里面， 把 string 部分提取到 endptr 里面，endptr没用
	stats.maxGrepThreads = strtol(argv[2], &endptr, 10);

	// argv[2] can not be converted into long without error
  //理论上 *endptr should be '\0', because we expect argv[2] consists only of number, the string should not exist
	if(errno != 0 || endptr == argv[2] || *endptr != '\0') {
		usage();
	}

	if(stats.maxGrepThreads <= 0) {
		fprintf(stderr, "max-grep-threads must not be negative or zero\n");
		usage();
	}

	// $$$$$$$$$$$$$$$$$$$$$$$$ TODO: implement me!

  searchString = argc[1];

  statsMutex = semCreate(1);
  if(statsMutex == NULL) die("semCreate");

  newDataSignal = semCreate(1);
  if(newDataSignal == NULL) die("semCreate");

  grepThreadsSem = semCreate(stats.maxGrepThreads);
  if(grepThreadsSem == NULL) die("semCreate");


  // 开始 创建 craw threads, 有几个 tree 就创建几个 craw threads， tree 是 以字符串的形式 给出的。
  // 要 在 forloop 之前 就把 所有的 tid 创建完成。
  pthread_t tids[argc-3];
  for(int i=3; i<argc; i++) {
    errno = pthread_create(&tids[i-3], NULL, processTree, argv[i]); // 不要写成&argv[i]，因为argv[i]里面的内容本身 就是一个char* 类型的 指针（字符串）
    if(errno != 0) die("pthread_create");

    // 开始 改变 activeCrawThreads   的数值， 因为创建了新的 thread
    incStat(&stats.activeCrawlThreads);

    // thread 自动毁灭模式(也可以在 processTree 函数里面写)
    errno = pthread_detach(tids[i-3]);
    if(errno != 0) die("pthread_detach");
  }


  // 不同于 SS21 里面的 Aufgabe 4 mach， 这里的 ausgabe 的任务应该由 main 来完成。
  // 所以 这里的main 应该 passive warten，  实现一个 ausgabe的功能。

  // 只要 有 active threads 的 存在， 不管是 craw threads 还是 grep threads,  都有 更新 statistic 的可能
  // 所以 我们的 判断条件 也非常 直观明朗。 但是 你要 zugreifen craw threads 和 grep threads 这两个 global variable，
  // 所以 你需要 mutex 的保护。
  while(1) {
    // 一旦有一个 craw thread 或者 grep thread 灭亡-----> 会更新 statistic ---> 死之前就会调用V(newDataSignal)
    // 一旦有一个 thread 调用了 V()， 那么这里 就不会一直堵塞。
    P(newDataSignal);

    // 堵塞状态结束， 开始运行。
    P(statsMutex);
    int crawActiveNr = stats.activeCrawlThreads;
    int grepActiveNr = stats.activeGrepThreads;
    V(statsMutex);

    if(crawActiveNr > 0 || grepActiveNr > 0) {
      printStatistics(); // 注意 printStatistics() 函数 里面 本身就使用 P 和 V 的保护。
    } else {
      // 说明 所有的 threads 已经死亡。
      break;
    }
  }

  // 最后， 为了确保 让结果保持在 terminal 上面， 并且 行光标 指向下一行， 进行如下操作。
  printStatistics();
  if(printf("\n") < 0) die("printf");

  // 每次的 printf 之后， 都要 fflush
  if(fflush(stdout) == EOF) die("fflush");


  //  最后 一定要 消灭 Semaphore！
  semDestroy(statsMutex);
  semDestroy(newDataSignal);
  semDestroy(grepThreadsSem);

  return(EXIT_SUCCESS);
}







/**
 * \brief Acts as start_routine for crawl-Threads and calls processDir().
 *
 * It updates the stats.activeCrawlThreads field.
 *
 * \param path Path to the directory to process
 *
 * \return Always returns NULL
 *
 * processTree 这个 函数函数 是 craw tread 的 函数。
 */
static void* processTree(void* path) {
	//TODO:$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ implement me!
  char * treeStr = (char*) path;

	return NULL;
}







/**
 * \brief Iterates over all directory entries of path and calls processEntry()
 * on each entry (except "." and "..").
 *
 * It updates the stats.dirs field.
 *
 * \param path Path to directory to process
 *
 * \return Always returns NULL
 */
static void* processDir(char* path) {
	// TODO: $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ implement me!

	return NULL;
}

/**
 * \brief Spawns a new grep-Thread if the entry is a regular file and calls processDir() if the entry
 * is a directory.
 *
 * It updates the stats.activeGrepThreads if necessary. If the maximum number of active grep-Threads is
 * reached the functions waits passively until another grep-Threads can be spawned.
 *
 * \param path Path to the directory of the entry
 * \param entry Pointer to struct dirent as returned by readdir() of the entry
 *
 * \return Always return NULL
 */
static void* processEntry(char* path, struct dirent* entry) {
	//TODO:  $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ implement me!

	return NULL;
}







/**
 * \brief Acts as start_routine for grep-Threads and searches all lines of the file for the
 * search pattern.
 *
 * It updates the stats.files, stats.lines, stats.fileHits, stats.lineHits
 * stats.activeGrepThreads fields if necessary.
 *
 * \param path Path to the file to process
 *
 * \return Always returns NULL
 */
static void* processFile(void* path) {
	//TODO: $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$  implement me!

	return NULL;
}
