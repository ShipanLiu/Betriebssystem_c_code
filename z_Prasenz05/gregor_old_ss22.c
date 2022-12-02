/*
这个 和clash 很相似， 在课上给出的

this program 缺少 父亲 程序 终结 子程序的 使用 wait()过程。


*/


#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "aio.h"
#include "wait.h"

int main(int argc, char** argv){ //expects call like './prog "echo this is a test"' or './prog "ls -f"'
    int max = 100; //arbitrary upper bound for no. of args, NEEDS to be at least 2, or while() below could access unallocated memory
    char* token[max]; // 存的是 从argv[0]  开始 的 所有的  即 token[0] = argv[0] ,argv[0] 是 我们写的 程序 比如 ./clash,  here we save our extracted tokens  就是 用 strtol（） 拆开的部分们
                      // argv[2]  就是 第一个 参数
    if(argc < 2){ //no program to execute, abort   argv[0] 是 ./prog 比如，  argv[1]  才是 “ls ./”  可以写成 if(argv <= 1)
        //Error handling
        exit(EXIT_FAILURE);
    }


    { //Limits visibility of i
        int i = 0;
        token[i++] = strtok(argv[1], " "); //argv[0] is OUR program's name（比如 ./clash）, argv[1]是比如 “ls ./jier”    这里 我们 关注的是 argv[1],
                                           //token[0] 里面存的是 Befehl， 比如 ls ， token[1]里面存的是 ./jier

        if(token[0] == NULL){ //argv[1] is empty string， 就是没有 参数 的意思。
            //Error handling
            exit(EXIT_FAILURE);
        }

        //get pointer to tokens, store them in array
        //token[] needs to be at least of size 2 for the first access to be valid
        while(token[i] = strtok(NULL, " ")){   // token[1] = strtok(NULL)
            /* strtok() returns NULL when it has ALREADY READ the last token
                This will break the loop if not too many tokens are given*/
            i++;
            if(i >= max) //check for too many args
            {
                //Error handling
                exit(EXIT_FAILURE);
            }
        }

        //i is the number of tokens we counted and new bound for token[] (NOT array index! That's i-1!)
        pid_t child;

        if(i == 1){ //i == 0 was already caught above
            child = fork();

            if(child == -1){
                //Error handling
                exit(EXIT_FAILURE);
            }
            else if(child == 0)
            {
                    execlp(token[0], token[0], NULL); //don't forget to add programme name and NULL

                    //Fehlerbehandlung
                    exit(EXIT_FAILURE);
            }

            wait(NULL);
        }
        else{
            for(int e = 1; e < i; e++){
                child = fork();

                if(child == -1){
                    //Error handling
                    exit(EXIT_FAILURE);
                }
                else if(child == 0){
                  // for each argument call one time(differrnt from clash, in clash you just call one time with the whole parameters)
                    execlp(token[0], token[0], token[e], NULL); //don't forget to add programme name and NULL

                    //Error handling
                    exit(EXIT_FAILURE);
                }
                else{
                    wait(NULL);
                }
            }
        }
    } //Limits visibility of i

    exit(EXIT_SUCCESS);
}