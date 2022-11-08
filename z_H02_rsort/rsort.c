#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define bufferMaxSize 12 // word_length + '\n' + '\0'

static int wordNum = 0;


// errno is set and exit with failure
static void kill(char* string) {
  perror(string);
  exit(EXIT_FAILURE);
}

static void warn(char* str) {
  fputs(str, stderr);
}

static int compareFunc(const void *a, const void *b) {
  char* const* A = (char* const*)a;
  char* const* B = (char* const*)b;
  return 0 - strcmp(*A, *B);
}



static char** input() {

  int wordArrSize = 2;

  // create buffer for holding the world
  char* buffer = malloc(bufferMaxSize * sizeof(char));
  if(buffer == NULL) {
    kill("malloc has problem");
  }

  // we do not know the size of the words, so first allocate 50 places.
  char** wordsArr = malloc(wordArrSize * sizeof(char*));

  if(wordsArr == NULL) {
    kill("malloc at assigning wordsArr");
  }

  while(1) {
    fflush(stdin);
    errno = 0;
    char* word = fgets(buffer, bufferMaxSize, stdin);
    if(word == NULL) {
      if(errno != 0) {
        kill("fgets failed");
      }
      break;
    }

    // get the real length of the word
    size_t wordLength = strlen(word);
    // printf("word: %s\n", word);
    // printf("wordLength: %d\n", wordLength);

    // if the word is too short(just a '\n')
    if(wordLength <= 1) {
      // printf("you can not just input an enter");
      continue;
    }

    // if the word is too long
    if(wordLength == bufferMaxSize-1 && word[wordLength-1] != '\n') {
      warn("word is too long");

    // get rid of the extra bits in the stdin
    int c;
    do{
      c = getchar();
    }while(c != EOF && c != '\n');

    // abandon this word, read next line.
      continue;
    }

    // get rid of the '\n' at the end of each word
    if(word[wordLength-1] == '\n') {
      word[wordLength-1] = '\0';
      wordLength--;
      // printf("wordLength after --: %d\n", wordLength);
    }

    // if the wordArr is full
    if(wordNum >= wordArrSize) {
      // printf("add more space");
      wordArrSize += wordArrSize;
      wordsArr = realloc(wordsArr, wordArrSize*sizeof(char*));
      if(wordsArr == NULL) {
        kill("relloc");
      }
    }

    // put the word in the wordArr
    char* wordCpy = malloc((wordLength+1)*sizeof(char)); // create space for word + '\0'
    if(wordCpy == NULL) {
      kill("malloc");
    }

    strcpy(wordCpy, word);
    wordsArr[wordNum] = wordCpy;
    wordNum++;
  }

  // stdin debug
  if(ferror(stdin)) {
    kill("stdin");
  }

  return wordsArr;

}



int main() {

  //01-input
  char** wordArr = input();
  //02-sort
  qsort(wordArr, wordNum, sizeof(char*), compareFunc);
  //03-output
  for(int i = 0; i < wordNum; i++) {
    // use puts but not fputs
    if(puts(wordArr[i]) == EOF) {
      kill("puts");
    }
    free(wordArr[i]);
  }

  //04-free the space
  free(wordArr);

  //05-clean stdout
  if(EOF == fflush(stdout))
    kill("fflush");

  exit(EXIT_SUCCESS);
}


/*
check if you have to use that much printf(Fehlermeldung erlaubt?)


 */