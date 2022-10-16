

/** To Do
 * - Promptsymbol ausgeben = "<Arbeitsverzeichnis>:" 就是把 当前的目录 给打印出来 done
 * - Eine Zeile von der Standardeingabe einlesen.
 * - Zeile in Kommandoname und Argumente zerlegen.
 * - Trennzeichen: Leerzeichen und Tabulatoren
 * - Neuen Prozess erzeugen,  hier “Kommando” mit “Argumenten” ausführen.
 * - Terminierung bei EOF (Ctrl-D)
 *
 * - Vordergrundprozess:
 *      - mit waitpid auf Terminierung warten
 *      - Exitstatus + Befehlszeile ausgeben
 *      - Dann neue Eingabe einlesen.
 *
 * - Hintergrundprozess:
 *      - Zeile endet mit "&".
 *      - Shell wartet nicht auf Terminierung des Prozesses
 *      - direkt neuen Prompt anzeigen
 *      - PID und Kommandozeile in verketteter Liste speichern -> plist.c
 *
 * - Vor Prompt:
 *  - Alle bis dahin terminierten Hintergrundprozesse aufsammeln (Zombies).
 *  - Exitstatus ausgeben
 *
 * - cd als eigenen Befehl implementieren
 *      - Arbeitsverzeichnis wechseln
 * - jobs als eigenen Befehl implementieren
 *      - alle laufenden Hintergrundprozesse ausgeben
 */


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>      // for waitpid()
#include <sys/wait.h>      // for waitpid()
#include <unistd.h>
#include <stdbool.h>

#include "plist.h"

// Maximal sind 1337 Zeichen inkl. \n erlaubt.
#define MAX_LINE 1337

static void die(const char *s) {
    perror(s);
    exit(EXIT_FAILURE);
}


// Prompt ausgeben
static void print_prompt(void) {
    size_t len = 1024;   // 你不知道 路径到占用多少空间， 所以先 存1024个bytes
    char *cwd = NULL;  // 定义一个 buffer， 存起来

    /**
     * getcwd holt das aktuelle Arbeitsverzeichnis als String und speichert dies in cwd.
     * Die Länge ist vorher nicht bekannt.
     * Daher kann es sein, dass der Puffer cwd nicht groß genug ist, dann schlägt getcwd() fehl.
     * Den Fall kann man erkennen, da in hier dann die errno auf ERANGE gesetzt wird.
     * -> Dann muss cwd vergrößert werden.
     * Alle anderen Errnowerte bedeuten einen "echten" Fehler.
     * 
     * 
     * char *getcwd( char *buffer, int maxlen );
     * 参数说明：getcwd()会将当前工作目录的绝对路径复制到参数buffer所指的内存空间中,
     *  参数maxlen为buffer的空间大小。
     * On success, getcwd() will return a pointer to a string 
     * containing the pathname of the current working director
     * 
     * On failure, getcwd() return NULL, and errno is set to
     * indicate the error
     * 
     * C库宏ERANGE表示范围错误，如果输入参数超出范围，则会发生范围错误，在该范围内定义数学函数并将errno设置为ERANGE。
     * 
     * 
     * 
     * 
     * 
     * 
     */
    while (1) {
        cwd = realloc(cwd, len);  //pointer cwd 指向开辟的新的空间， 
        if (cwd == NULL) {
            die("realloc");
        }

        // 你不知道 cwd 的空间够不够， 所以， getcwd  函数会出错， 然后返回NULL，说明空间不够。
        // 假如 getcwd(cwd, len) != NULL 的话， 说明成功了，空间够了， 那就break
        if (getcwd(cwd, len) != NULL) {
                break;
        }
        // 空间不够肯定会导致 设置 error的值， 我们再这里只 关注 ERANGE 意外的 错误
        if (errno != ERANGE) {
            die("getcwd");
        }
        len *= 2;
    }
    
    printf("%s: ", cwd);  // 根据题意： Promptsymbol und folgt mit doppeltpoint aus geben 
    // 需要 flush，防止 再 缓存区里面的东西 没有完全 弄出来，
    // printf()输出时是先输出到缓冲区，然后再从缓冲区送到屏幕上。
    // 那什么情况下才会将缓冲区里的内容送到屏幕上呢？一，使用fflush（stdout）强制刷新  
    fflush(stdout);

    free(cwd);
}

/**
 * Exitstatus ausgeben
 * Mit WIFEXITED fragen, ob der Prozess einen Exitstatus hat.
 * Erst dann darf dieser mit WEXITSTATUS abgerufen werden.
 */
static void print_exit(char *buf, int status) {
    if (WIFEXITED(status)) {
        // 我们要把 commandozeile 打印出来。 并且把 status 也打印出来
        printf("Exitstatus [%s] = %d\n", buf, WEXITSTATUS(status));
    } else {
        printf("No exitstatus [%s]\n", buf); // else-Fall optional
    }
}

// laufende Hintergrundprozesse ausgeben -> "<pid> <Kommandozeile>"
static int print_job(pid_t pid, const char *buf) {
    printf("%d %s\n", (int)pid, buf);
    return 0;
}


// Zombies aufsammeln
static void collect_zombies(void) {
    pid_t pid;
    int status;
    
    /**
     * waitpid:
     * -1, um auf beliebigen Kindprozess zu warten
     * WNOHANG, um sofort zurückzukehren, falls es keine fertigen Kindprozesse gibt
     * 
     * 
     * pid_t waitpid(pid_t pid, int *status, int options);
     * 
     * pid=-1	等待任何子进程，此时的waitpid()函数就退化成了普通的wait()函数
     * 
     * WNOHANG 	如果pid指定的子进程没有结束(子进程 还没有死)，则waitpid()函数立即返回0，而不是阻塞在这个函数上等待；
     * 如果结束了，则返回该子进程的进程号。
     * 
     * 
     */
    // 当 waitpid() 返回的 pid > 0 的时候， 说明 子进程已经死， 返回死了的 子进程的 进程号. 
    // 返回pid = -1 的时候， 说明 fehler. 
    // 返回 pid = 0 说明 kinder 还活着， 没有死 
    // wir gucken ob es noch kinder Zombie gibt, wenn ja, dann nehmen wir das raus und gebe auch die drin stehende Connandozeile aus
    // wenn kein zombie kind nicht mehr findet, dann break die while Schleife 
    while ((pid = waitpid(-1, &status, WNOHANG)) != 0) {
        // 进程号 一般都 大于0
        if (pid < 0) {
          if (errno == ECHILD) {
              // Es gibt keine Kindprozesse -> dies ist kein Fehler
              break;
          }
          die("waitpid");
        }

        /**
         * 
         *  我们之前已经 将 hintergrundprozess 加入到了plist.c 里面的 list 里面了。
         *  我们必须将这个 已经死掉的 hintergrundprozess 从 list 里面 除掉。
         *  把藏在这个 hintergrundprozess 里面的东西拿出来（是个 commando Zeilen 放到 buf 里面） 
         * 
         * 
        */
        // 题目给出的 commondozeile 的长度最大是1337（包含 '\n'）， 我们的 buf 要 1338 因为要加上 '/0'
        char buf[MAX_LINE + 1];

        /**
         * für Infos zu removeElement siehe plist.h
         * Die Liste enthält nur die laufenden Hintergrundprozesse.
         * Kindprozess mit der ProzessID pid ist fertig, kann aus Liste entfernt werden.
         * Kommandozeile des Prozesses aus der Liste in buf speichern.
         * 
         * int removeElement(pid_t pid, char *buf, size_t buflen)
         */
        if (removeElement(pid, buf, sizeof(buf)) < 0) {
          continue;
        }
        
        // Exitstatus ausgeben
        print_exit(buf, status);
      }
  }

int main(void) {
    while (1) {
        // 把 kinder 彻底终结 + 终结的kind 从 list 里面移除
        collect_zombies();
        // 把当前的 目录打印下来
        print_prompt();

        // 现在 要 新增 输入 
        // Zeile von der Standardeingabe einlesen
        // MAX_LINE = 1337 Zeichen inkl. \n + 1 für \0
        char buf[MAX_LINE + 1];  /* 因为需要 '\0' */
        // 从 stdin 里面读取， 并且 写入到buf 里面
        if (fgets(buf, sizeof(buf), stdin) == NULL) {  // fgets()  如果读入错误或遇到文件结尾(EOF)，则返回NULL. 
            // C 库函数 int feof(FILE *stream) 测试给定流 stream 的文件结束标识符。
            // 要记住，你最多输入1337个字符，在按下 回车之后 会加上  '\n' 和 EOF。
            // 我们要判断 返回 NULL 是读了EOF 还是 有错了 
            if (feof(stdin)) {
                break;  // 如果是读到了 stdin， 那就 break 不 die
            }
            die("fgets");
        }
        
        /**
         * Akzeptierte Zeilen dürfen maximal 1337 Zeichn inklusive \n lang sein
         * Falls Puffer voll und letztes gelesenes Zeichen ist nicht \n -> Zeile hat mindestens 1337 Zeichen und ist daher zu lang.
         * Rest der Zeile mit fgetc weglesen, bis EOF oder \n erreicht ist.
         * 
         * 现在要检查 你 输入的情况了， strlen(buf) = MAX_LINE 表示 buf 放满了 
         * 假如你 输入 多了, buf只能盛放 1337个字符， 最后一个 字符 不是 ‘\n’
         * 你本来应该 输入 1336字符 + '\n' ，但是 你输入了 比如1339个字符。 比如： buf 里面的 1337个字符 + ‘A’ + '\n'
         */
        if (strlen(buf) == MAX_LINE && buf[MAX_LINE-1] != '\n')
        {
            fprintf(stderr, "Input line too long\n");

            // 用 c 把 stdin 里面的多的剩下的 吃掉, 记住下面的 代码！！！！ 背过！！！
            // 比如： buf 里面的 1337个字符 + ‘A’ + '\n'  我要把 'A' 和 ‘\n’ 吃掉   注意 是 do while() 会先行一步
            int c;
            do {
                c = fgetc(stdin);
            } while (c != EOF && c != '\n');
            
            // Fehlerbehandlung fgetc()
            if(ferror(stdin))
                die("fgetc");
                
            // Zeile ignorieren
            continue;
        }
        
        // Leerzeilen überspringen
        if (strlen(buf) <= 1) {   // 只有 ‘\n’ 的时候， 或者用户直接输入了 ctrl + D ， 输入了EOF
           continue;
        }


        
        // 最后的\n entfernen，换成'\0'
        buf[strlen(buf)-1] = '\0'; 

        /**
         * Falls das letzte Zeichen ein & ist, handelt es sich um einen Hintergrundprozess.
         * & entfernen
         * 
         * 上面把 最后一个字符换成了 '\0' ， 再次调用strlen(buf) ， 长度不算 '\0'
         */
        bool background = false;
        if (buf[strlen(buf)-1] == '&') {
           buf[strlen(buf)-1] = '\0'; // 最后的\n entfernen，换成'\0'
           background = true;
        }







        /**
         * Puffer kopieren, da dieser durch strtok() gleich zerstückelt wird.
         * Kommandozeile wird aber noch gebraucht, um den Exitstatus auszugeben
         * oder um bei einem Hintergrundprozess in der Liste gespeichert zu werden.
        */
       // 把刚输入到 buf 里面的命令 copy 到 cpy 里面
        char cpy[sizeof(buf)];
        strcpy(cpy, buf);

        /**
         * Eingabezeile zerteilen und argv[] Array erstellen.
         * Trennzeichen sind Leerzeichen und Tabulatoren.
         * 
         * 
         *  #include <string.h>
            #include <stdio.h>
            
            int main () {
            char str[80] = "This is - www.runoob.com - website";
            const char s[2] = "-";
            char *token;
            
            // 获取第一个子字符串 
            token = strtok(str, s);
            
            //继续获取其他的子字符串
            while( token != NULL ) {  // 完事了之后， 返回 NULL
                printf( "%s\n", token );
                
                token = strtok(NULL, s);  // 第二次使用的 时候， 把 NULL 写进去 就行了
            }
            
            return(0);
            }
         * 
         * 
         * 
         * 输出结果：
         *  This is 
            www.runoob.com 
            website
         * 
         * 
         * 比如 buf 里面存的是 : echo hello world  ===> 
         * argv[0] = echo
         * argv[1] = hello
         * argv[2] = world
         * argv[3] = NULL
         * 
         */
        int i = 0;
        char* argv[MAX_LINE/2+1 + 1 /* NULL */]; // argv【】 里面存的是字符串， 因为最后返回的 NULL 也要被保存
                                                 //  最多有 MAX_LINE/2+1 个部分， 因为buf是 zeichen + leerzeichen， 一半一半组成的
        argv[i++] = strtok(buf, " \t"); // argv[0] 存 第一部分， 然后 i++， 第一部分一般是 commando
        while ((argv[i++] = strtok(NULL, " \t")) != NULL) {
           continue;
        }


        
        // Es wurde nichts oder nur Trennzeichen eingegeben., 判断是否有东西。
        if (argv[0] == NULL) {
           continue;
        }


        // 下面 是区别 是那种命令了

        /**
         * 'cd'
         * Nutzung: "cd <directory>"
         * -> argv[0] = "cd"
         * -> argv[1] = <directory>
         * -> argv[2] = NULL
         * Falls argv[1] == NULL ist, wurde zu wenig eingegeben. -> Verzeichnis fehlt，easy
         * Falls argv[2] != NULL ist, wurde zu viel eingegeben， easy
         * chdir() ändert das Arbeitsverzeichnis.
         * chdir 是C语言中的一个系统调用函数（同cd），用于改变当前工作目录，其参数为Path 目标目录，可以是绝对目录或相对目录
         * 
         * 
         * int strcmp(const char *str1, const char *str2)
         *  如果返回值 < 0，则表示 str1 小于 str2。
            如果返回值 > 0，则表示 str1 大于 str2。
            如果返回值 = 0，则表示 str1 等于 str2。
         */
        
        // 假如是 "cd"  命令的话
        if (strcmp(argv[0], "cd") == 0) {
              if (argv[1] == NULL || argv[2] != NULL) {
                  fprintf(stderr, "usage: cd <directory>\n"); // optional 或者 用perror("usage: cd <directory>\n wrong!!!")
              } else if (chdir(argv[1]) != 0) {  // int chdir(const char * path); 成功则返回0, 失败返回-1, errno 为错误代码.
                  perror("chdir");
              }
              continue;  // 一定要continue， 要不然 下面的两个if 也要执行， 一个if 负责 jobs， 一个if 负责 除了 cd 和 jobs 的其他命令
        }

        /**
         *  jobs命令可以查看当前有多少在后台运行,
         * 比如：
         * 
         *  [1]   7895 Running                 gpass &
            [2]   7906 Running                 gnome-calculator &
            [3]-  7910 Running                 gedit fetch-stock-prices.py &
            [4]+  7946 Stopped                 ping cyberciti.biz
         * 
         * 
         * 
         * 'jobs'
         * Soll PID und Kommando von allen laufenden Hintergrundprozesse ausgeben -> walkList() siehe plist.c / plist.h
         * Nutzung: nur "jobs" eingeben, jobs 命令是 不需要参数的， 所以我们期望 只有 argv[0] = "jobs" 并且 argv[1] = NULL
         * -> argv[0] = "jobs"
         * -> argv[1] = NULL
         * Falls argv[1] != NULL ist, wurde zu viel eingegeben.
         */
        if (strcmp(argv[0], "jobs") == 0) {
            if (argv[1] != NULL) {
              fprintf(stderr, "usage: jobs\n"); // optional或者用 perror() 更好
            } else {
              walkList(print_job); //wir laufen durch die Liste und für jede Listelemente soll alle PID und Kommand ausgeben
            }
            continue;
        }


        
        // 如果 既不是 cd 命令 也不是 jobs 命令， 那也要执行。之前的 cd 和 jobs 不分 background， 但这里， 其他的命令要分 background 还是 front
        // Alle anderen Kommandos in einem Kindprozess ausführen.
        pid_t pid = fork();
        if (pid < 0) {
          die("fork");
        } else if (pid == 0) { // Hier sind wir im Kind -> Kommando ausführen.
          execvp(argv[0], argv);   // execvp() 函数 的第二个参数 是一个数组， 注意 命令要重复两次
          die("exec"); // 套路背过！假如 execvp执行成功给， 就不会执行这个die
        }

        // Hier sind wir im Elternprozess.
        /**
         * 在我们输入 buf 里面的命令了，假如最后结尾为 & , 说明这是一个 background ， 意思就是说让 kinder process
         * 去执行， 所以要 假如 到 plist 里面的list 里面。
         * 
         * 
         * 
        */

        if (background) {
            /**
             * Falls es ein Hintergrundprozess ist:
             * -> ProzessID und Kommando in Liste einfügen und nicht auf Terminierung des Prozesses warten.
             * 为甚？ 因为 有collect_zombies（） 会自动清理 list 里面的 dead kinder， 所以我们不用去等 这个 命令完全结束
             */
            if (-2 == insertElement(pid, cpy)) {
              // setzt nicht die errno, siehe plist.h
              fprintf(stderr, "out of memory");        // 这里不能 用 perror来代替， 因为使用 perror的前提是： error wird gesetzt.
              exit(REXIT_FAILUE);
            }
        } else {
            /**
             * 这里是 Vordergrund， 不想 background 有 有collect_zombies（） 函数 来清理 ， 我们这里需要自己去清理 dead kinder，这里是 父进程的区域， 
             * 所以我们就等待 kinderprocess 死亡
             * Vordergrundprozess
             * Auf Kind mit der PID warten, die wir von fork() bekommen haben.
             */
            int status;
            if (waitpid(pid, &status, 0) < 0) { // 假如第三个参数是 0， 说明没有 option，这此时waitpid()函数就完全退化成了wait()函数
                                                // waitpid()一般返回 被 terminated 的 kinderprocess 的 pid， 返回一个小于 0 的数， 说明出错了
                die("waitpid");
            }
            // Hier Exitstatus ausgeben.
            print_exit(cpy, status);
        }
    } // End of While
}




/**
 * 执行过程 ：
 * 
 * 1. step ：  make
 * 
 * 2. step：   ./calsh
 * 
 * 例子：
 *  [root@java100 u_8_2_clash_answer]# ./clash
    /root/Betriebssystem_workplace/u_8_2_clash_answer: jobs
    /root/Betriebssystem_workplace/u_8_2_clash_answer: echo hallo world $
    hallo world $
    Exitstatus [echo hallo world $] = 0
    /root/Betriebssystem_workplace/u_8_2_clash_answer: jobs
    /root/Betriebssystem_workplace/u_8_2_clash_answer: 




    可以 看到 输入 sleep 5& 命令 之后， 进入background, 可以 立刻输入下一个命令  echo hallo world
    但是 输入 sleep 5 之后， 没有进入background， 所以在输入 下一条命令 之前， 需要 等 5 秒才可以

    /root/Betriebssystem_workplace/u_8_2_clash_answer: sleep 5&
    /root/Betriebssystem_workplace/u_8_2_clash_answer: echo hallo world
    hallo world
    Exitstatus [echo hallo world] = 0
    /root/Betriebssystem_workplace/u_8_2_clash_answer: 
    Exitstatus [sleep 5] = 0
    /root/Betriebssystem_workplace/u_8_2_clash_answer: 
    /root/Betriebssystem_workplace/u_8_2_clash_answer: 
    /root/Betriebssystem_workplace/u_8_2_clash_answer: sleep 5







    Exitstatus [sleep 5] = 0


 * 
 * 
 * 
 * 
 * 
*/


