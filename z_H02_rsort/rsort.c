/*
T00.25

Shipan Liu  108019201553
Minhua Liu  108020210282

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define bufferMaxSize 102 // word_length + '\n' + '\0'

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

  int wordArrSize = 50;

  // create buffer for holding the world
  char* buffer = (char*)malloc(bufferMaxSize*sizeof(char));

  // we do not know the size of the words, so first allocate 50 places.
  char** wordsArr = (char**)malloc(wordArrSize * sizeof(char*));
  if(wordsArr == NULL) {
        kill("malloc at assigning wordsArr");
    }

  while(1) {
    if(EOF == fflush(stdin)) {
		kill("fflush stdin");
	}

    char* word = fgets(buffer, bufferMaxSize, stdin);
    if(word == NULL) {
	  // if error
      if(ferror(stdin)) {
	    kill("fgets -> stdin");
	  }
	  // if read EOF
      break;
	}

    // get the real length of the word
    size_t wordLength = strlen(word);

    // if the word is too short(just a '\n')
    if(wordLength <= 1) {
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
    }

    // if the wordArr is full
    if(wordNum >= wordArrSize) {
      wordArrSize += wordArrSize;
      wordsArr = (char**)realloc(wordsArr, wordArrSize*sizeof(char*));
      if(wordsArr == NULL) {
        kill("relloc");
      }
    }

    // put the word in the wordArr
    char* wordCpy = (char*)malloc((wordLength+1)*sizeof(char)); // create space for word + '\0'
    if(wordCpy == NULL) {
      kill("malloc");
    }

    strcpy(wordCpy, word);
    wordsArr[wordNum] = wordCpy;
    wordNum++;
  }

  // we do not need buffer any more
  free(buffer);

  return wordsArr;

}



int main() {

  //01-input
  char** wordsArr = input();
  //02-sort
  qsort(wordsArr, wordNum, sizeof(char*), compareFunc);
  //03-output
  for(int i = 0; i < wordNum; i++) {
    // use puts but not fputs
    if(puts(wordsArr[i]) == EOF) {
      kill("puts");
    }
    free(wordsArr[i]);
  }

  //04-free the space
  free(wordsArr);

  //05-clean stdout
  if(EOF == fflush(stdout)){
    kill("fflush stdout");
  }

  exit(EXIT_SUCCESS);
}
