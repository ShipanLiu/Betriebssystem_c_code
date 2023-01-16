#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>

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

// function declarations
static void* processTree(void* path);
static void* processDir(char* path);
static void* processEntry(char* path, struct dirent* entry);
static void* processFile(void* path);
// TODO: add declarations if necessary

static void usage(void) {
	fprintf(stderr, "Usage: palim <string> <max-grep-threads> <trees...>\n");
	exit(EXIT_FAILURE);
}

static void die(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

/**
 * \brief Initializes necessary data structures and spawns one crawl-Thread per tree.
 * Subsequently, waits passively on statistic updates and on update prints the new statistics.
 * If all threads terminated, it frees all allocated resources and exits/returns.
 */
int main(int argc, char** argv) {


	if(argc < 4) {
		usage();
	}

	// convert argv[2] (<max-grep-threads>) into long with strtol()
	errno = 0;
	char *endptr;
	stats.maxGrepThreads = strtol(argv[2], &endptr, 10);

	// argv[2] can not be converted into long without error
	if(errno != 0 || endptr == argv[2] || *endptr != '\0') {
		usage();
	}

	if(stats.maxGrepThreads <= 0) {
		fprintf(stderr, "max-grep-threads must not be negative or zero\n");
		usage();
	}

	// TODO: implement me!

	return EXIT_SUCCESS;
}

/**
 * \brief Acts as start_routine for crawl-Threads and calls processDir().
 *
 * It updates the stats.activeCrawlThreads field.
 *
 * \param path Path to the directory to process
 *
 * \return Always returns NULL
 */
static void* processTree(void* path) {
	//TODO: implement me!

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
	// TODO: implement me!

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
	//TODO: implement me!

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
	//TODO: implement me!

	return NULL;
}
