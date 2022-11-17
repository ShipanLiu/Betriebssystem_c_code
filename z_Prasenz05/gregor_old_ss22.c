#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "aio.h"
#include "wait.h"

int main(int argc, char** argv){ //expects call like './prog "echo this is a test"' or './prog "ls -f"'
    int max = 100; //arbitrary upper bound for no. of args, NEEDS to be at least 2, or while() below could access unallocated memory
    char* token[max]; //here we save our extracted tokens

    if(argc < 2){ //no program to execute, abort
        //Error handling
        exit(EXIT_FAILURE);
    }


    { //Limits visibility of i
        int i = 0;
        token[i++] = strtok(argv[1], " "); //argv[0] is OUR program's name, token[0] is the program we want to execute

        if(token[0] == NULL){ //argv[1] is empty string
            //Error handling
            exit(EXIT_FAILURE);
        }

        //get pointer to tokens, store them in array
        //token[] needs to be at least of size 2 for the first access to be valid
        while(token[i] = strtok(NULL, " ")){
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