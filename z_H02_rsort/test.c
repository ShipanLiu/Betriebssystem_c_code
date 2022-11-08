#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define num 12

// errno is set and exit with failure
static void kill(char* string) {
  perror(string);
  exit(EXIT_FAILURE);
}


static void func1() {
  char* str = "jier";
  size_t size = strlen(str);
  printf("%c", str[size-1]);
}

static void func2() {
  char* buffer = (char*)malloc(num*sizeof(char));
  // flush stdin before using
  fflush(stdin);
  printf("start reading the 1. word\n");
  //size_t bufferLength = sizeof(buffer); 错误！ sizeof(buffer) 相当于 sizeof(pointer)永远是8！
  char* word = fgets(buffer, num, stdin);  // according to manpage, the fgets will only read num-1 bits(最好情况下 最后一个 bits 是 \n, 这样才不会超)
  size_t wordLength = strlen(word);
  printf("输入之后wordLength:  %d\n", strlen(word));
  printf("输入之后wordLength(明显错误, 永远不要用 sizeof):  %d\n", sizeof(word));
  printf("输入之后BufferLength: %d\n", strlen(buffer));
  printf("word: %s\n",word);
  printf("buffer: %s\n",buffer);
  printf("最后一个bit: %d\n", word[wordLength-1]); // 结果是 10，  10 对应的就是 '\n'

  //吃掉 缓存区（注意缓存区 不等价于  stdin）
  int c;
  do{
    c = getchar();
  }while(c != EOF && c != '\n' && c != '\0');

  printf("start reading the 2. word\n");

  char* word1 = fgets(buffer, num, stdin);
  printf("buffer: %s\n",buffer);
  printf("word1: %s\n",word1);
  printf("最后一个bit: %d\n", word[wordLength-1]);

}

//  the difference between strcpy and strdup
static void func3() {
  // 使用 strdup， 会自动调用 malloc， 结果假如 不用了 话， 必须free。
  // char* str = "shipanliu";
  // char* line1 = strdup(str);
  // printf("line1: %s\n", line1);


  // 使用 strcpy， 但是事先 必须声明 一个 malloc 的 地址（就是 开辟了 空间 的 地址）。

  char buffer[12];
  char* word = fgets(buffer, 12, stdin);
  if(word == NULL) {
    kill("fgets");
  }

  size_t wordLength = strlen(word);
  if(word[wordLength-1] == '\n') {
    word[wordLength-1] = '\0';
    wordLength--;
    printf("wordLength: %d\n", wordLength);
    printf("last bit: %c\n", word[wordLength-1]);
  }

  char* line2 = malloc((wordLength+1)*sizeof(char)); // create space for word + '\0'
  printf("line2 adress: %p\n", line2);
  // char line2[strlen(str)];
  if(line2 == NULL) {
    kill("malloc");
  }

  strcpy(line2,word);

  printf("after copy\n");

  printf("line2: %s\n", line2);
  printf("word again: %s\n", word);
  printf("word adress: %p\n", word);
  printf("line2 adress: %p\n", line2);

}

int main() {
  // func2();
  func3();
}