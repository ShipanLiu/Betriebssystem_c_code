/*

new tutorial test code

 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>  // opendir
#include <dirent.h> // readdir()
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

static void die(char* msg) {
  perror(msg);
}

int main(int argc, char* argv[]) {
  DIR* d = opendir("*");
  // error handling
  if(d == NULL) {
    die("opendir");
  }

  // now open this directory
  struct dirent* de = NULL;
  do {
    errno = 0;
    de = readdir(d);
    if(de == NULL) {
      if(errno) {
        die("readdir");
      } else {
        break;
      }
    }

    struct stat sb;
    if(lstat(de->d_name, &sb) == -1) {
      die("lstat");
    }

    if(S_ISLINK(sb.st_mode)) {
      if(printf("%s", de->d_name) < 0) {
        die("printf");
      }
    }

  } while(1);

  if(fflush(stdout)) {
    die("fflush");
  }

  return 0;
}