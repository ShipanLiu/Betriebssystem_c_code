#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define num 12


static void func1() {
  char* str = "jier";
  size_t size = strlen(str);
  printf("%c", str[size-1]);
}

static void func2() {
  char* buffer = (char*)malloc(num*sizeof(char));
  //size_t bufferLength = sizeof(buffer); 错误！ sizeof(buffer) 相当于 sizeof(pointer)永远是8！
  char* word = fgets(buffer, num-1, stdin);
  size_t wordLength = strlen(word);
  printf("输入之后wordLength:  %d\n", strlen(word));
  printf("输入之后wordLength(明显错误, 永远不要用 sizeof):  %d\n", sizeof(word));
  printf("输入之后BufferLength: %d\n", strlen(buffer));
  printf("word: %s\n",word);
  printf("buffer: %s\n",buffer);
  printf("最后一个bit: %d", word[wordLength-1]); // 结果是 10，  10 对应的就是 '\n'

  // 吃掉 缓存区（注意缓存区 不等价于  stdin）
  // int c;
  // do{
  //   c = getchar();
  // }while(c != EOF && c != '\n');

  // char* word1 = fgets(buffer, num-1, stdin);
  // printf("buffer: %s\n",buffer);
  // printf("word1: %s\n",word1);

}

int main() {
  func2();
}