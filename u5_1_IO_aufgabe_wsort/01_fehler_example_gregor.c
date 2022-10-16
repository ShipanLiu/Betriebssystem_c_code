#include "stdio.h"
#include "errno.h"
#include "stdlib.h"

int main(int argc, char* argv[]) {
    char read_buffer[16];

    while(1) {
        fgets(read_buffer, 16, stdin);  // 16  一般就是 read_buffer的size， 这里不读一个FILE， 读stdin 流
        // for(int i=0; i<16; i++) {
        //     printf("%c", read_buffer[i]);
        // }
        // if(feof(stdin)) {  // 当 stdin读到1.EOF 或者 2.有问题的时候，会返回EOF， feof(EOF)  的值是一个 positive 
        //     fputs("shit! stdin read EOF\n", stderr);  // 把错误放到 stderr 里面， 会立刻输入到console上面 // fputs（） 和 fprints（） 作用一样 
        //     fflush(stderr);  //完事之后，把 stderr 的puffer 清空， 以免影响下一步
        //     return EXIT_FAILURE;
        // }

        // 第二种 方法：
        if(ferror(stdin)) { // 只有当 stdin 有问题的时候
            perror("something wrong with stdin, Fehlermeldung via perror()");  // 相当于 ：fputs("shit! stdin read EOF\n", stderr);
            fflush(stderr);
            return EXIT_FAILURE;
        }

        printf("Read: %s\n", read_buffer);
        fflush(stdout);
        // check if stdout has Fehler or not
        if(ferror(stdout)) {
            perror("stdout has Fehler");
            fflush(stderr);
            clearerr(stderr); // ferror(stdout)函数值为一个非零值。因此触发 if 条件，
                                // 在调用clearerr(stderr)后，ferror(stderr)的值变为0， 归零 
        }

    }
}
