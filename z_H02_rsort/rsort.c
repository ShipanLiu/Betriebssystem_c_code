#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define singleWordMaxLength  100 // word_length
#define singleWordMaxSize  101 // word_legth + '\n'
#define bufferMaxSize 102 // word_length + '\n' + '\0'


// errno is set and exit with failure
static void kill(char* string) {
  perror(string);
  exit(EXIT_FAILURE);
}

static void warn(char* str) {
  fputs(str, stderr);
}


static char* input() {

  int wordNum = 0;
  int wordArrSize = 0;

  // create buffer for holding the world
  char* buffer = malloc(bufferMaxSize * sizeof(char));
  if(buffer == NULL) {
    kill("malloc has problem");
  }

  // we do not know the size of the words
  char** wordsArr = NULL;

  while(1) {
    char* word = fgets(buffer, singleWordMaxSize, stdin);
    if(word == NULL) {
      kill("fgets failed");
    }

    // get the real length of the word
    size_t wordLength = strlen(word);
    printf("word: %s", word);
    printf("wordLength: %d", wordLength);

    // if the word is too short(just a '\n')
    if(wordLength == 1) {
      printf("you can not just input an enter");
      continue;
    }

    // if the word is too long
    if(wordLength == singleWordMaxSize && word[wordLength-1] != '\n') {
      warn("word is too long");

    // get rid of the extra bits in the stdin
    int c;
    do{
      c = getchar();
    }while(c != EOF && c != '\n');

    // abandon this word, read next line.
      continue;
    }

    // get rid of the '\n' at the end of each line
    if(word[wordLength-1] == '\n') {
      word[wordLength-1] = '\0';
      wordLength--;
      printf("wordLength after --: %d", wordLength);
    }


  }



}



int main() {

  //01-input
  input();

  //02-sort

  //03-output

  //04-free the space


}