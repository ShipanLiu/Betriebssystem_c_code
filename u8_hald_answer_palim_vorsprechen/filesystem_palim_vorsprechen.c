 //finished   -- 14.10.2022


/*
 Aufgabe:
 Wir wollen ein Programm schreiben, welches unser
 aktuelles Verzeichnis durchläuft und uns alle darin
 vorhandenen Symlinks ausgibt. 


 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

/**
 * Verwendete Funktionen:
 *  Verzeichnis öffnen                             -> DIR *opendir(const char *dirname);
 *  Eintrag aus dem Verzeichnis lesen              -> struct dirent *readdir(DIR *dirp);
 *  Geöffnetes Verzeichnis schließen               -> int closedir(DIR *dirp);
 *  Metadaten auslesen                             -> int lstat(const char *path, struct stat *buf);
 * 
 *  -> wir verwenden lstat, weil wir die Symlinks der Datei ausgeben wollen
 *  -> stat läuft dem Symlink nach und gibt die Metadaten des    “Ziels”     aus
 *  -> lstat gibt die Metadaten des ”Symlinks selbst“ aus, das wollen wir so
 * 
 * lstat 和 stat  的区别 ：  stat和lstat的区别：当文件是一个符号链接时，lstat返回的是该符号链接本身的信息；而stat
 *                          返回的是该链接指向的文件的信息。
 * 所以你想要 stmlinks 的信息的时候， 应该 用 lstat
 * 
 * 
 * 
 */


static void die(const char *msg){
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(void){
    
    /** aktuelles Verzeichnis öffnen
     * . bezieht sich auf das aktuelle Verzeichnis (Selbstreferenz) ;
     */
    
    // 打开当前的文件夹
    DIR * dirPointer = NULL;
    if((dirPointer = opendir(".")) == NULL)
        die("opendir");
    
    /**
     * über Einträge iterieren
     * dirent = directory entry, Speicher für einen ausgelesenen Eintrag ;
     */
    // buf ist der Speicher für die Metadaten, die uns lstat liefert.
    struct dirent* entry = NULL;  // struct dirent* 类型是 readdir() 的返回值
    struct stat buf;   // struct stat 类型的 buf 是  lstat（） 函数 需要的。
    
    while(1){
        /**
         * Besonderer Fall der FB:
         *  readdir() gibt NULL bei EOF oder Fehler zurück.
         *  Fehlerfall: Return NULL und errno wurde verändert
         *  Keine Einträge mehr zum lesen: Return NULL und errno wurde nicht verändert -> EOF erreicht
         *  -> Errno vor jedem Aufruf von readdir() auf 0 setzen und für den Fall, dass readdir() NULL zurückgibt,
         *    kann so ein Fehler von EOF unterschieden werden.
         */

        /**
         * 在发生错误时，大多数的 C 或 UNIX 函数调用返回 1 或 NULL，同时会设置一个错误代码 errno(比如从1 到 20,每个数代表一种错误)，该错误代码是全局变量，
         * 表示在函数调用期间发生了错误。您可以在 errno.h 头文件中找到各种各样的错误代码。
         *  所以，C 程序员可以通过检查返回值，然后根据返回值决定采取哪种适当的动作。
         * 开发人员应该在程序初始化时，把 errno 设置为 0，这是一种良好的编程习惯。0 值表示程序中没有错误。
         * 
         * 
        */
        errno = 0;
        entry = readdir(dirPointer);    // 在 readdir() 执行之后， 会自动把 dirPointer 更新到下一个 目标, 所以我们要在外面加一个 while(1) 来配合
                                        // readdir 在 出错的时候 or 读完的时候， 都会 返回一个NULl， 所以最后需要判断
                                        // 是否是真正地   出错了， 假如出错了， error 就不再是 0 了。

        if(entry == NULL)   // entry = NULL 说明 读完了， 或者是 出错了， 两种情况下， 我们都不希望 继续执行
            break;
        /**
         * entry zeigt nun auf einen Eintrag in unserem geöffneten Verzeichnis
         * Typ struct dirent enthält d_name und d_ino
         */
        
        // Metadaten mit lstat holen, können dann über buf abgefragt werden
        /**
         * 我通过 readdir() 返回值 得到的 entry 是 struct dirent* 类型的
         * entry 就相当于 课件上 的 Verzeichnis table  ， 有 inode name 和 filename 对应
         * 
         * struct dirent {
         *      ino_t d_ino;    // inode name 
         *      char d_name[];  // filename   
         * 
         * }
         * 
        */
        if(lstat(entry->d_name, &buf) == -1) // 把 根据文件名 “entry->d_name” ，查找文件详细信息， 并且 把这个信息放到buf 里面
                                                // 假如是 一个symlink 的话， lstat() will return the symlink itself, but the stat() will return the destination 
            die("lstat");
        
        /**
         * Dateityp herausfinden
         * st_mode Komponente enthält den Typ (Verzeichnis, Symlink, reguläre Datei...)
         * S_ISLNK gibt ”true“ zurück, falls es ein Symlink ist
         * -> negieren, um andere Einträge zu ignorieren
         * 
         * 
         * buf 就是 struct stat 类型：
         * 
         * struct stat {
                dev_t st_dev;
                ino_t st_ino; // Inode  number
                mode_t st_mode;  // File type
                nlink_t st_nlink; // nummber of hard links
                uid_t st_uid;    // user id
                gid_t st_gid;
                dev_t st_rdev;
                off_t st_size;  // total size in bytes
                blksize_t st_blksize;
                blkcnt_t st_blocks;
                time_t st_mtime;
                time_t st_ctime;
            };
         * 
         * 
         * 
         *   几个常见的宏 来判断 st_mode
         *   S_ISLNK(st_mode)：是否是一个Symlink.
         *   S_ISREG(st_mode)：是否是一个常规文件.
         *   S_ISDIR(st_mode)：是否是一个目录
         * 
         *   S_ISCHR(st_mode)：是否是一个字符设备.
         *   S_ISBLK(st_mode)：是否是一个块设备
         *   S_ISFIFO(st_mode)：是否 是一个FIFO文件.
         *   S_ISSOCK(st_mode)：是否是一个SOCKET文件 
         * 
         * 
         */
        
        if(!S_ISLNK(buf.st_mode))  //   如果不是 我们要找的 symlink， 直接 continue
            continue;
        
        /**
         * 出现一个新函数 ： readLink()
         * #include<unistd.h>
            ssize_t readlink(const char *path, char *buf, size_t bufsiz);   成功返回 size， 失败 返回 -1
         * readlink()会将参数path（一个存在的软连接， 这里就是 entry->d_name）的符号链接内容存储到参数buf所指的内存空间，
         * 返回的内容不是以 '\0' 作字符串结尾，但会将字符串的字符数返回，这使得添加 '\0' 变得简单。
         * 若参数bufsiz小于符号连接的内容长度，过长的内容会被截断，如果 readlink 第一个参数指向一个文件而不是符号链接时，
         * readlink 设 置errno 为 EINVAL 并返回 -1。 readlink()函数组合了open()、read()和close()的所有操作。 
         * Mit readlink kann der Inhalt ausgelesen werden.
         * Dies entspricht dem Ziel, auf welches der Symlink zeigt.
         */

        // 创建一个容器， readlink() 会把 内容放进去， readlink() 读出地内容是最后是 不自动加 ‘/0’ 的
        char dst[buf.st_size + 1];
        dst[buf.st_size] = '\0';

        if(readlink(entry->d_name, dst, buf.st_size) == -1)
            die("entry->d_name");
    
        // Symlinks und Ziel ausgeben
        printf("%s -> %s\n", entry->d_name, dst); // 输出结果： symlink_test -> ./test.txt ， 这个 ./test.txt  就是 这个 软连接symlink_test 里面的真正内容， 被放到了 dst 里面了
        
    } // End of While
    
    // errno wurde verändert -> Fehler bei readdir()
    if(errno != 0)  // 假如 error 不再是 我们设置的 初始值 0， 说明 error 已经被修改， 说明 在读到 末尾（EOF）之前出错了
        die("readdir");
    
    // Verzeichnis schließen， closedir() 函数 接受的 是 Dir* 类型的 dirPointer
    if(closedir(dirPointer) == -1)
        die("closedir");
}


/**
 * 如何执行 ：  首先需要有 symlink （软连接）
 *  ln 命令是一个非常重要命令，它的功能是为某一个文件在另外一个位置建立一个同步的链接。类似windows下的快捷方式。
    Linux文件系统中，有所谓的链接(link)，我们可以将其视为档案的别名，而链接又可分为两种 : 硬链接(hard link)与软链接(symbolic link)，硬链接的意思是一个档案可以有多个名称，而软链接的方式则是产生一个特殊的档案，该档案的内容是指向另一个档案的位置。硬链接是存在同一个文件系统中，而软链接却可以跨越不同的文件系统。

 *  ln [参数][源文件或目录][目标文件或目录]      参数 ： -s 代表软链接(符号链接)
 *  
 * 
[root@java100 u_8]# touch test.txt
[root@java100 u_8]# vim test.txt 
[root@java100 u_8]# ln -s ./test.txt ./symlink_test
[root@java100 u_8]# ll
总用量 4
lrwxrwxrwx. 1 root root 10 10月 14 12:06 symlink_test -> ./test.txt
-rw-r--r--. 1 root root 17 10月 14 12:05 test.txt

下一步：
gcc -std=c11 -pedantic -D_XOPEN_SOURCE=700 -Wall -Werror -o findSymlink filesystem_palim_vorsprechen.c

下一步 执行 ： 
[root@java100 u_8]# ./findSymlink 
symlink_test -> ./test.txt


 * 
 * 
 * 
 * 
 * 
*/
