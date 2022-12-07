#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

static void die(char* msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

static void warn(char* msg) {
  fputs(msg, stderr);
}

static int checkLine(char* str) {
  // copy the str for later compare
  char* strCpy = strdup(str);
  if(strCpy == NULL) {
    die("strdup");
  }
  //check if space exists or not
  char *afterToken = strtok(str, " \t");
  //if there exists space, then return 0
  return strcmp(afterToken, strCpy) == 0 ? 1 : 0;
}

static void getPoints(char* str) {
    int index = 0;
    int i = 0;
    int pointArr[6];
    while(i<6) {
        if(i == 0) {
          pointArr[index] = (int)(strtok(str, ",")[1]);
        } else {
            char temp[2] = strtok(NULL, ",");
            if(i%2 != 0) {
                pointArr[index] = (int)temp[0];
            }else {
                pointArr[index] = (int)temp[1];
            }
        }
        index++;
        i++;
    }

}



int main(int argc, char* argv[]) {

  char input[] = "(1,1),(1,1),(1,1)";
//   printf("%s\n", checkLine(input) == 1 ? "good string" : "bad string");

//   int v1x;
//   int v1y;
//   char* firstItem = strtok(input, ",");
//   printf("%s\n", firstItem);

  // getPoints(input);
  printf("jier");



  exit(EXIT_SUCCESS);
}
