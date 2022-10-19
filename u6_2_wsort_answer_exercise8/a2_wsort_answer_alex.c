// finished  by shipan    last edit time :  15:00   13.09.2022


/*
Aufgabe 2: wsort (10.0 Punkte)   就是 读取一个文件进来， 然后 进行sort， 每行一个word

1. Schreiben Sie ein Programm wsort
2. eine Liste von W ̈ortern vom Standard-Eingabekanal (stdin) einliest,
3. diese Liste alphabetisch sortiert und die sortierte Liste auf dem Standard-Ausgabekanal (stdout) ausgibt
4. jedes Wort in einer eigenen Zeile   每个 wort 占用一行
5. Zeilen sind durch ein Zeilenumbruch-Zeichen (\n) voneinander getrennt, das selbst nicht Teil des Wortes ist
   Jede Zeile endet mit einem Zeilenumbruch – lediglich die letzte Zeile muss nicht zwangsl ̈aufig ein \n-Zeichen enthalten
   每一行（除了最后一行）结尾 都 \n 结束  ，\n 不算在word里面
6. Leere Zeilen sind ohne Fehlermeldung zu ignorieren  +  W ̈orter, die eine maximale L ̈ange von 100 Zeichen  ̈uberschreiten,
    werden mit einer entsprechenden Fehlermeldung ignorie
7.

*/


/**
 * To Do:
 *  Wsort: liest Liste von Wörtern von stdin ein, sotiert diese Liste und gibt die Wörter in sortierter Reihenfolge auf stdout wieder aus.
 *  - Ein Wort pro Zeile. Alle Zeichen einer Zeile gehören zum Wort.
 *  - \n ist nicht Teil des Wortes.
 *  - Jede Zeile endet mit \n, die letzte Zeile eventuell nicht --> EOF (END OF FILE)
 * 
 *  - ！！！！！！！！！！！！Maximale Wortlänge = 100, mit \n sind es 101 Zeichen.！！！！！！！！！！！！！！！！
 * 
 *  - Überlange Zeilen mit Fehlermeldung auf    stderr(Standard-Fehlerausgabe)（要记住 如何用）     ignorieren.
 *  - Leere Zeilen ohne Fehlermeldung ignorieren.
 *  - Zum Sortieren qsort verwenden.
 *
 *  Qsort:
 *  - void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
 *  - base: ein Pointer auf den Anfang des Arrays, also z.B. argv
 *  - nmemb: Anzahl der Elemente im Array
 *  - size: Größe der Elemente im Array
 *  - compar: Vergleichsfunktion
 *
 * 
 * 要理解 和明白：  
 *  Aufruf zum Testen:
 *   gcc -std=c11 -pedantic -D_XOPEN_SOURCE=700 -Wall -Werror -o wsort a2_wsort_answer_alex.c 
 *  ./wsort < wlist0 > my_wlist0                  // wsort 是我们自己编写的程序， 输入 wlist0 文件， 输出 my_wlist0 
 *  ./wsort.bsteam < wlist0 > bsteam_wlist0        //wsort.bsteam 也是一个排序的程序（已经给出）， 输入 wlist0 文件， 输出 bsteam_wlist0
 *  diff -s -u my_wlist0 bsteam_wlist0    // 见下面执行情况：
 * 
 * [root@java100 u_6_2]# diff -s -u my_wlist0 bsteam_wlist0 
    Files my_wlist0 and bsteam_wlist0 are identical

 * 
 * 通过查询 man diff 得知：         -s, --report-identical-files    ：report when two files are the same
 *                                  -u, -U NUM, --unified[=NUM]    ： output NUM (default 3) lines of unified context
                                                        
 * 
 * 
 * 解释：
 * linux中经常会用到将内容输出到某文件当中，只需要在执行命令后面加上>或者>>号即可进入操作。
 *大于号 >：将一条命令执行结果（标准输出，或者错误输出，本来都要打印到屏幕上面的）重定向其它输出设备（文件，打开文件操作符，或打印机等等），就是输入到文件中。
 *小于号 < ：命令默认从键盘获得的输入，改成从文件，或者其它打开文件以及设备输入, 就是从 键盘上 输入了， 直接从 文件中输入

 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */
 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define Wort_Laenge 100 // Wort = 100 Zeichen
#define Zeilen_Laenge 101 // 100 Zeichen + \n = 101
#define Buf_line 102 // Im Buffer werden von fgets im max. Fall 100 Zeichen + \n + \0 = 102 gespeichert， \0 就是结束符。
#define alloc_at_once 100 // 100 Speicherplätze gleichzeitig allokieren


// 出错的时候， 而且出错会导致退出的情况下， 用这个自定义的 die()
static void die(const char *programm){
    perror(programm);
    exit(EXIT_FAILURE);
}


static int wordCompare(const void *a, const void *b){
    /**
     * a und b müssen auf unsere gewünschten Typen umgecastet werden, da die Vergleichsfunktion für qsort im default mit const void* arbeitet
     * Gelesen als: w1 ist ein Zeiger auf konstante Char-Zeiger
     * Das heißt, dass diese Funktion die Strings, die wir vergleichen wollen (char*) nicht verändern darf.
    */
    char* const *w1 = (char* const *)a;   // 用 *a 把 pointer a 中的值取出来。  给 字符串 w1    *w1  是定义一个 pointer 用的  
    char* const *w2 = (char* const *)b;
    
    // Da wir an die Strings hinter den Pointern wollen, müssen wir diese dereferenzieren.
    return strcmp(*w1, *w2);
}

int main(int argc, char**argv){
    
    char buf[Buf_line]; // Puffer für aktuell eingelesene Zeile
    char **words = NULL; // Array: Doppelzeiger, weil wir Zeiger auf die eingelesenen Worte speichern wollen. Äquivalent zu char* words[] --> zeigt nochmal, dass wir ein Array haben, in dem char-Pointer gespeichert werden.
    unsigned int allocCount = 0; // Zum zählen, wie viel Platz im words-Array allokiert wurde
    unsigned int wordCount = 0; // Zum zählen, wie viel Platz im words-Array bereits belegt ist (Index)
    
    /**
     * fgets ließt Buf_line-1 Zeichen von stdin ein und speichert diese in buf.
     * buf muss Buf_line groß sein, denn fgets setzt ans Ende noch ein \0
     * Fehlerbehandlung von fgets: siehe nach While-Schleife
     */
    while(fgets(buf, Buf_line, stdin)){  // char* fgets(char*s, int n, FILE *fp/stdin)  n是容器的size
        size_t length = strlen(buf);  // size_t strlen(const char* str);  参数是一个 pointer（string pointer 或者 array 地址）
        
        // Überlänge?
        /**
         * Falls fgets eine zu lange Zeile eingelesen hat, ist der Puffer buf voll gefüllt (101 eingelesene Zeichen + \0), allerdings ist das vorletze Zeichen dann nicht \n
         */
        // length == Zeilen_Laenge && buf[Zeilen_Laenge-1] != '\n'  ： 这样说明超了， 
        // length == 101 && buf[100] = '\n'  ： 这样说明正好装满， 
        
        // 把多余的  字母 吃掉
        if(length == Zeilen_Laenge && buf[Zeilen_Laenge-1] != '\n'){
            fprintf(stderr, "input too long\n");  // 可以用 puts 和 perror 代替
            /**
             * An dieser Stelle kein "die()" verwenden, denn:
             * 1: Diese Bedingung haben wir gestellt, es ist kein Programmfehler --> errno wird nicht gesetzt. die() verwendet perror, dies führt also zu Fehlern.
             * 2: die() beendet das Programm mit exit(EXIT_FAILURE), wir möchten überlange Zeilen aber nur ignorieren und nicht unser gesamtes Programm beenden.
             */
            
            /**
             * Der Rest der überlangen Zeile liegt noch auf stdin,
             * wenn wir so weitermachen würden, würde der Rest als nächstes Wort interpretiert werden.
             * Also muss dieser Rest noch zeichenweise "weggelesen" werden.
             * Dieses Weglesen endet bei \n oder EOF, im Falle dass das Dateiende (STRG-D) erreicht wurde.
             */
            int c;
            do{
                c =  getchar();
            }while(c != EOF && c != '\n');
            continue;
        }
        
        
        
        
        // \n ist nicht Teil des Wortes, falls vorhanden --> entfernen
        if(buf[length-1] == '\n'){
            buf[length-1] = '\0';
            length--;
        }
        
        // Leeres Wort? Ignorieren
        // 当你 不 输入字母， 只是回车的时候， 你只有 length = 1 , buf[length - 1] = '\n'  上面会把 '\n'  变成 '\0' 并且 length--
        // 这样 就说明 你什么也没有输入。  
        // 注意 : word size 你要设置成 101 一位最后你要 回车（'\n'）,  但是 buffer size 你要设置成 102， 回车代表 这一轮的 stdin 结束， 
        // 编译器 会在 你 101  的 基础上 加上一个  '\0' 最后 变成 一共 102 个
        if(length <= 0)
            continue;
        
        // korrektes Wort -> einfügen
        // 要是空间不够用的情况下
        if(wordCount >= allocCount){
            // 100 Speicherplätze auf einmal neu allokieren
            allocCount += alloc_at_once;
            /**
             * altes Array mit realloc auf neue Größe vergrößer
             * (allocCount) * sizeof(*words)
             *          ^ Multiplikation
             * Das neue, größere Array soll allocCount viele Plätze haben.
             * Ein Platz entspricht der Größe eines char-Pointers --> *words
             */
            // 重新调整一下空间：
            words = realloc(words, (allocCount)*sizeof(*words));
            // 假如 realloc 出错
            if(words == NULL)
                die("realloc");
        }
    
        // Zeile kopieren, da buf im nächsten Durchlauf überschrieben wird.
        // 因为 buf 在下一轮的 stdin 中 要 被 overwrite， 所以 把里面的内容先 保存下来。
        char *line = strdup(buf);
        // 假如 strdup 出错
        if(line == NULL) {
            die("strdup");
        }

        // Pointer in words-Array speichern
        words[wordCount] = line;  // wordCount 的初始值为0
        wordCount++;
    }
    

    // 假如 上面的 stdin 有出错的话， 会立刻写入 error， ferror(stdin) 就是 检测 有没有 stdin 类型的错误。
    if(ferror(stdin))
        die("fgets/getchar");
    
    qsort(words, wordCount, sizeof(*words), wordCompare);
    /**
     *  Qsort:
     *  - void qsort(void *base, size_t nmemb, size_t size, int (*compar)(const void *, const void *));
     *  - base: ein Pointer auf den Anfang des Arrays, also z.B. argv
     *  - nmemb: Anzahl der Elemente im Array
     *  - size: Größe der Elemente im Array
     *  - compar: Vergleichsfunktion
     */
    
    // Ausgabe
    for(int i=0; i<wordCount; i++){
        if(EOF == puts(words[i]))  // 建议 用 fputs  来代替 puts(会自动结尾换行)，  puts 和 printf 的功能和 用法一样
                                  // fputs(words[i], stdout)
            die("puts");
        // Einzelne Zeiger im Array freigeben
        free(words[i]);
    }
    
    // 因为是二维指针数组，  前面 free words[i], 现在free words
    // gesamtes Array freigeben
    free(words);
    
    // Da wir etwas auf stdout schreiben, kann hier etwas "hängen bleiben".
    // Stdout flushen, um sicher zu sein, dass die Ausgabe (puts) geschrieben wurde.
    if(EOF == fflush(stdout))
        die("fflush");
    
    exit(EXIT_SUCCESS);
}
