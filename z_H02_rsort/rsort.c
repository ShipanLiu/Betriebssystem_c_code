#include <stdio.h>
#include <stdlib.h>


#define singleWordLength = 100
#define singleWorldSize = 101

static void kill(char* string) {
    perror(string);
    exit(EXIT_FAILURE);
}


static char* input() {

  int wordNum = 0;
  int wordArrSize = 0;

  // create buffer for holding the world
  char* buffer = malloc(102 * sizeof(char));
  if(buffer == NULL) {
    kill("malloc has problem");
  }

  // we do not know the size of the words
  char* wordsArr[] = NULL;











}



int main() {

  //01-input

  //02-sort

  //03-output

  //04-free the space


}