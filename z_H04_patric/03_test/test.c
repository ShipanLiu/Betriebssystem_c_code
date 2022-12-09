
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

//  我想 测试 pthread_create的基本操作。  问什么 函数进不去？

typedef struct arg{
    int num;
} arg;


static void *fun(void* a) {


    printf("get in\n");

    arg* myArg = (arg*)a;
    printf("%d\n", myArg->num);

    pthread_detach(pthread_self());
    return NULL;

}

int main()
{
    pthread_t tid;
    arg parameter;
    parameter.num = 99;
    errno = pthread_create(&tid, NULL, fun, &parameter);
    // pthread_join(tid, NULL);
    printf("over\n");
    return 0;
}
