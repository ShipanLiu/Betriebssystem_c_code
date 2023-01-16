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

  searchString = argv[1];

  statsMutex = semCreate(1);
  if(statsMutex == NULL) die("semCreate");

  newDataSignal = semCreate(0);
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
  processDir(treeStr);

  //thread 的使命完成 ---> activeCrawThreads减一
  decStat(stats.activeCrawlThreads);
  //在死亡之前 解开 main 里面的堵塞, 好让 main 及进行输出。
  V(newDataSignal);

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
 *
 *
 * 这个函数的
 */



static void* processDir(char* path) {
	// TODO: $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ implement me!

  // 这里就涉及 打开 文件夹 等 代码了
  DIR* dirPointer = opendir(path); //比如 opendir(".")
  if(dirPointer == NULL) {
    die("opendir");
  }

  //上面的 操作， 打开了 一个 文件夹， 要记录这个文件夹
  incStat(&stats.dirs);
  // notify the main function and output the latest statistics
  V(newDataSignal);

  struct dirent* entry = NULL; // 为 readdir() 做准备
  errno = 0;

  entry = readdir(dirPointer);
  // 我们用 while loop， 把 这个 entry 里面的所有的 file 找到。
  // 因为这个是一个 path， 只能是
  // 我们期待 出现 比如 "./" 或者
  while(entry != NULL) {
    // 每个文件夹里面都有 "." 和 ".."  这两个文件夹
    /**
     * example： 看最后的 code 部分
     *
     * path: "."
     * 操作一： DIR* d = opendir(".");
     * 操作二： loop this function:  readdir(d);
     * 操作三： 输出结果如下：
     *
     * this is a regular file: main.c    <---- entry1
     * this is a folder: ..              <---- entry2
     * this is a folder: .               <---- entry3
     * this is a regular file: a.out     <---- entry4
     *
     * 说明 在 path "."  路径下面 存在 四个 entry 分别是 两个 folder， 两个 regular file
     *
     *
     *
    */
    if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      processEntry(path, entry);
    }

    //load next entry
    errno = 0;
    // dirPointer is DIR-Struktur, is ein Iterator und speichert jeweils aktuelle Position
    // readdir() liefert einen "entry" und setzt den Dir-Iterator auf den Folgeeintrag.
    // 就是 会自动 下延续,
    //!!!!!!!!!! entry 就是 打开 path 里面的 item， 可以是 folder， 可以是 regular file
    entry = readdir(dirPointer);
  }

  // 到这里 说明 entry = NULL, 两种情况， 一种是 errno， 一种是读到EOF
  if(errno) die("readdir");

  // 到最后 当然要把 dirPointer close 一下
  if(closedir(dirPointer) == -1) die("closedir");

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
 *
 * 上面的 processDir  每次发现一个新的 entry，就会 调用一次 processEntry.
 *
 */
static void* processEntry(char* path, struct dirent* entry) {
	//TODO:  $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$ implement me!

  // 现在 就要判断是 regular file 还是 folder

  struct stat buf;

  int pathLength = strlen(path);
  // 假如 path 是 "."， 举两个例子
  // entry1 的 d_name 是 "folder1"
  // entry2 的 d_name 是 "folder2"

  // pathPlusEntry = "./folder1" 或者  "./folder2"
  // pathPlusEntryLength 就是 path + entry->d_name + 2 （因为 / 和 '\0'）
  int pathPlusEntryLength = pathLength + strlen(entry->d_name) + 2;

  char* pathPlusEntry = (char*)malloc(pathPlusEntryLength*sizeof(char)); //

  if(pathPlusEntry == NULL) die("malloc");

  //现在把 path 和 entry 合在一起，中间加上 '/'

  strcat(pathPlusEntry, path);
  strcat(pathPlusEntry, "/");
  strcat(pathPlusEntry, entry->d_name);

  //现在你的 pathPlusEntry = "./item"
  // you have to check what is this item, regular file? folder? or sysLink?
  if(lstat(pathPlusEntry, &buf) == -1) {
    free(pathPlusEntry);
    die("lstat");
  }

  // if directory
  if(S_ISDIR(buf.st_mode)) {
    // back to processDir
    processDir(pathPlusEntry);
  }
  if(S_ISREG(buf.st_mode)) {
    // start a new grep thread, 由  grepThreadsSem 这个 Semaphore 来控制
    P(grepThreadsSem);

    pthread_t tid;
    errno = pthread_create(&tid, NULL, processFile, pathPlusEntry);
    if(errno != 0) die("pthread_create");

    errno = pthread_detach(&tid);
    if(errno != 0) die("pthread_detach");


    incStat(&stats.activeGrepThreads);

    //  unblock the main function and let main function output the latest statistics
    V(newDataSignal);
  }

  // 不要忘记 free

  free(pathPlusEntry);

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
 *
 * processFile() 就是 FILE 的 老一套了，就是 打开 文件， 一行一行 查找。
 */
static void* processFile(void* path) {
	//TODO: $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$  implement me!

  //这是 thread function，  上来 就 立刻 casten
  char* filePath = (char*)path;


  incStat(&stats.files);
  //一旦更新 statistics ， 马上unblock main 函数。
  V(newDataSignal);

  FILE* fh = fopen(filePath, "r");
  if(!fh) die("fopen");

  //check if this file can hit the search string or not
  // 题目已经给出， each line is shorter than 4096
  char line[4096+2];

  while(fgets(line, sizeof(line), fh)) {
    //现在读入了一个 line， 更新 stats
    incStat(&stats.lines);

    // 每一行 只要记录 hit 一次就行。
    if(strstr(line, searchString)) {
      incStat(&stats.lineHits);
    }

    // let main out put the latest statistics (especially the line and linehits)
    V(newDataSignal);

  }
  if(ferror(fh)) die("fgets");

  if(fclose(fh) == EOF) die("fclose");

  // 这个 thread 生命 要走向终结
  decStat(&stats.activeGrepThreads);

  // free 1 thread to the limit
  V(grepThreadsSem);

  // unblock main for new statistics(activeGrepThreads的值改变了)
  V(newDataSignal);

	return NULL;
}





/*下面代码 帮助 研究清楚 什么是entry*/

/*

new tutorial test code,  替补的老师的讲解

 */

/* #include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  // types eg. ino_t --> Inode number
#include <dirent.h> // readdir() opendir
#include <errno.h>
#include <unistd.h> // execvp(), readLink()
#include <sys/stat.h>  //lstat()  S_ISLINK

static void die(char* msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
  DIR* d = opendir(".");
  // error handling
  if(d == NULL) {
    die("opendir");
  }

  // now open this directory
  struct dirent* de = NULL; // because the returned value of readdir() is struct dirent* 类型
  do {
    errno = 0;
    // 在loop中第一次调用结束之后， readdir 会自动把 d指向下一个
    de = readdir(d);
    if(de == NULL) {
      if(errno) {
        die("readdir");
      } else {
        // errno 保持为0，但是 de = NULL, 说明读到EOF 了， 看Ubung folie
        break;
      }
    }

    struct stat sb;
    if(lstat(de->d_name, &sb) == -1) {
      die("lstat");
    }

    if(S_ISLNK(sb.st_mode)) {
      if(printf("%s", de->d_name) < 0) {
        die("printf");
      }
    }

    if(S_ISDIR(sb.st_mode)) {
        printf("this is a folder: %s\n", de->d_name);
    }

    if(S_ISREG(sb.st_mode)) {
        printf("this is a regular file: %s\n", de->d_name);
    }


  } while(1); // while loop 的原因 是我先要 把所有的 entry 都 loop 一下。

  if(fflush(stdout)) {
    die("fflush");
  }

  return 0;
} */

//OUTPUT：

/* this is a regular file: main.c
this is a folder: ..
this is a folder: .
this is a regular file: a.out */