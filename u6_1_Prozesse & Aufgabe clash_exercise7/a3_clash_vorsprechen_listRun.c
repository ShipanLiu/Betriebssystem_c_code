// finished


//  这是 clash 之前的准备程序。



//  模拟一个 terminate
//  ./listRun（就是argv[0]） 命令(就是argv[1]), argv[1](命令的重复，为了重复) argv[2](这是真正的第一个参数) argv[3]..... argv[n-1]

//  然后  命令 会执行 到每一个 arg 上面 :
// 命令 arg[2]
// 命令 arg[3]
// 命令 arg[4]
// 命令 arg[5]

// 程序的演示 看 onenote    u06





#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>   // für wait() und waitpid(), siehe       man 3 wait
#include <sys/types.h>  // für fork(), siehe man fork
#include <unistd.h>     // für fork()
#include <errno.h>      // für Fehlerbehandlung


// 这个c文件 本来叫  listRun.c  你编译之后， 会产生一个新的 exe 文件   叫 listRun.exe  , 执行这个 listRun.exe 文件
// 直接 用 ./listRun   
// 比如最后 ：./listRun echo hallo world
// 结果：
// hallo
// world
// 0

/*
// 生成 .o 文件
[root@java100 Betriebssystem_workplace]# gcc -std=c11 -pedantic -D_XOPEN_SOURCE=700 -c a3_clash_vorsprechen.c
// 生成 .exe  文件， 改名为 listRun
[root@java100 Betriebssystem_workplace]# gcc -o listRun a3_clash_vorsprechen.o
// 开始执行
[root@java100 Betriebssystem_workplace]# ./listRun echo hallo world
hallo
EXITED WITH: 0
world
EXITED WITH: 0


或者 一步到位生成 listRun.exe :

[root@java100 Betriebssystem_workplace]# gcc -std=c11 -pedantic -D_XOPEN_SOURCE=700 -Wall -Werror -o listRun a3_clash_vorsprechen.c

[root@java100 Betriebssystem_workplace]# ./listRun echo hello world jier
hello
EXITED WITH: 0
world
EXITED WITH: 0
jier
EXITED WITH: 0
[root@java100 Betriebssystem_workplace]# 1







*/






// Eingabe im Terminal: ./listRun programm arg1 ... argn
// argv[0] = ./listRun
// argv[1] = programm
// argv[2] = arg1
// ...
// argv[argc-1] = argn

/*
 * Funktion, um den Code zu verkürzen.
 * Bitte nur nutzen, wenn die errno gesetzt wurde, da hier perror verwendet wird.
 */
static void die(const char *programm){
    perror(programm);
    exit(EXIT_FAILURE);
}



int main(int argc, char** argv){  // argc 就是参数的个数    argv 就是指针指向参数的 数组

    //  至少有两个item， argv[0] :  listRun      argv[1]: Befehl    可以没有参数
    if(argc<2){
        // there should be at least 2 args, arg[0] is copy of the command, arg[1]才是真正的 第一个参数
        // fprintf(stderr, "Usage: ./listRun <programm> <arg1> ... \n");  // same as:  perror("Usage: ./listRun <programm> <arg1> ... \n")
        //                                                                // same as : puts("出错了", stderr);
        // exit(EXIT_FAILURE);
        die("size of argc is too small");
    }
    
    /*
     * Schleife beginnt bei 2, da in argv[2] das erste Argument für den Befehl aus argv[1] steht. Genaue Aufteilung siehe oben.
     * Diese Schleife hat einen Durchgang pro Argument.
     */
    // argv[2]  就是 第一个 参数
    for(int i=2; i<argc; i++){
        
        // Kindprozess erzeugen und PID in p speichern
        pid_t p = fork();
        
        // Fehlerfall
        if(p == -1)
            die("fork");
        
        /*
         * Wenn wir im Kindprozess sind, ist diese Bedingung true und der Inhalt des folgenden Blocks wird ausgeführt.
         * Im Kind wird also der übergebene Befehl mittels execlp() ausgeführt.
         * Wir nutzen hier execlp(), weil wir nur ein einzelnes Argument übergeben.
         * Das übergebene Argument ist das, welches im aktuellen Schleifendurchlauf angesprochen wird.
         * So wird im besten Fall pro Schleifendurchlauf der Befehl (in argv[1], s.o.) mit einem der Argumente in einem Kindprozess ausgeführt.
         */
        if(p == 0){
            // Programm ausfuehren
            execlp(argv[1], argv[1], argv[i], NULL);  // 因为 argv[0] 是 ./listRun 是一个路径。   argv[1]  才是 command ， 根据规定， 后面的 command 又要重复一次, 牛逼
                                                    // argv-array 最后一个元素应该是一个 null
            /*
             * exec() kehrt nur in den aufrufenden Prozess zurück, falls ein Fehler bei der Ausführung aufgetreten ist.
             * Die Zeile in diesem Block nach dem execlp() wird also nur bei einem exec-Fehler ausgeführt und wir brauchen nicht auf einen Fehler prüfen, sondern können direkt die Fehlerbehandlung machen.
             */

            // 下面这一行一般是不会 执行的， 只有当 execlp出错的时候， 才会执行。
            die("exec");
        }
        
        /*
         * Im Elternprozess wollen wir auf das eben erzeugte Kind und dessen Beendigung warten.
         * Wir nutzen wait(), da wir nicht weiter machen wollen, bis der aktuelle Befehl fertig ausgeführt wurde.
         * In wstatus werden von wait() verschiedene Flags gespeichert, die wir später abfragen können.
         */
        // 进入了 父进程
        // wait returning >= 0 tells you a child process has terminated
        // but it does not tell you whether that process terminated successfully or not (or if it was signalled).
        // So, the program needs to do further tests on the status structure that was populated by wait()
        // WIFEXITED(status): did the process exit normally? 
        // WIFEXITED(status) 这个宏用来指出子进程是否为正常退出的，如果是，它会返回一个非零值
        // WEXITSTATUS(status) == 0: did the process exit with exit code 0 , 0 代表成功， 1 代表失败

        // Eltern 必须等 kind 死了之后
        else{
            // 定义一个 变量， 来盛放 status（是一个int） 的值
            int status;
            // wait()会暂时停止目前进程的执行, 直到有信号来到或子进程结束. 如果在调用wait()时子进程已经结束, 则wait()会立即返回子进程结束状态值. 
            // 子进程的结束状态值会由参数status 返回, 而子进程的进程识别码也会一快返回. 如果不在意结束状态值, 则参数 status 可以设成NULL. 即 wait(NULL)
            // wait（） 把 状态放入 status， 返回 kind pid
            pid_t x = wait(&status);
            
            // wait 返回 -1 说明 wait 出错
            // 返回值：如果执行成功则返回子进程识别码(PID), 如果有错误发生则返回-1. 失败原因存于errno 中。
            if(x == -1){
                perror("wait");  // perror 不仅会 把“wait” 打出来， error 里面的 出错原因也会打印 出来
                                // 这里不能直接调用 自己创建的 die() 方法， 执行 die()   程序会直接 exit
                continue;   
            }
            /*
             * Ob das Kind mit einem Exitstatus terminiert ist, können wir mit WIFEXITED(status) abfragen. (true, falls es einen Exitstatus gibt)
             * NUR DANN kann dieser Exitstatus mittels WEXITSTATUS(status) ausgegeben werden.
             */
            
            // 如果 WIFEXITED(status 返回一个非零的 值，说明 子进程完全 exit、
            if(WIFEXITED(status)){
                // 成功 exit
                printf("EXITED WITH: %d\n", WEXITSTATUS(status));  // WEXITSTATUS 会把 status 里面存的值解析出来。
                // 输出结果：  EXITED WITH: 0  ，   0就是代表成功的意思。
            }
        }
    }
}
