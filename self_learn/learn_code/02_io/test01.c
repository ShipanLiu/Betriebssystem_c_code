#include<stdio.h>
#include<string.h>

//test fopen and fclose
void write1() {
    FILE *f_write = fopen("test.txt", "w");


    // 如果磁盘已经满的话
    // if you read a file, and the file doies not exist, this time you will get a NULL
    if(f_write == NULL) {
        printf("something is wrong");
        return;
    }

    // 写文件具体操作。
    //char buf[32] = "hello world";
    char buf[32] = "hello world \n"; // 文件最后有换行， 光标到下一行
     
    int i = 0;
    while(buf[i] != '\0') {
        fputc(buf[i], f_write);
        i++;
    }

    // 关闭文件
    fclose(f_write);
}


void read1() {
    FILE *f_read = fopen("test.txt", "r");

    if(f_read == NULL) {
        printf("read failed");
    }

    // 那么读到 哪里呢？  读到文件尾巴 ：  标志 EOF ， means end of file
    char ch = 0;
    // if you do not read EOF, please read always
    while((ch = fgetc(f_read)) != EOF) {
        printf("%c", ch);
    }
}



// 案例1： 用户输入的所有字符，都会直接写入到文件中
// 知道用户输入：  :quit代表输入结束

void test03() {
    FILE *f_write = fopen("test.txt", "w");
    if(f_write == NULL) {
        printf("write in file failed");
        return;
    }

    while(1) {
        // 首先定义一个缓冲区
        char buf[128] = "";  // char buf128] = {0, 0, 0, ...... , 0, 0, 0};
        // get the value from the keyboard
        //char *fgets(char *s, int size, FILE *stream);
        // I only read size 个  char, the rest I will abandon.
        // 在输入一些之后时候，每次回车的时候， 程序会往下面执行， 下面会检查一遍":quit" 牛逼。
        fgets(buf, sizeof(buf), stdin);  // 区别于 fgetc(), fgetc是按照char读文件， fgets是按照 行 来读
        printf("因为你回车了， 程序往下面执行");
        // check if you are going the stop inputing
        // I only check 5 bits because ":quit is 5 bits"
        if(strncmp(buf, ":quit", 5) == 0) {
            break;
        }

        // write buf into the file
        int i = 0;
        while(buf[i] != '\0') {
            fputc(buf[i++], f_write);
        }
    }

    // 关闭文件
    fclose(f_write);


}



//案例： 按 行  和 fputs  写文件
void test04() {
    FILE *f_write = fopen("test.txt", "w");
    if(f_write == NULL) {
        printf("the file write failed");
        return;
    }

    // 字符串数组只能这么定义， 三个char pointer 都指向每一个字符串的第一个字母
    char *buf[] = {"jier1\n", "jier2\n", "jier5\n"};
    int len = sizeof(buf) / sizeof(char *);
    for(int i = 0; i < len; i++) {
        fputs(buf[i], f_write);
    }

    // close
    fclose(f_write);

}


//案例： 按 行  和 fputs  写文件
void test05() {
    FILE *f_read = fopen("test.txt", "r");
    if(f_read == NULL) {
        printf("the file read failed");
        return;
    }

    // when is it time to the end of the file?
    // there is a function we can use :   feof(FILE *)
    // !0 : it is already the end of the file
    // 0:  not the end yet

// 我想要这个在 if 0 中的代码块不要运行, 假如要运行的话， 改成1
#if 0
    char buf[1024] = {0};
    while(!feof(f_read)) {// 如果没有读到文件结尾， 一直按行读取
        //fgets(buf, sizeof(buf), f_read);

        //去掉文件中每一行的 \n ， 改成\0,  如： jier1\n\0   ---> jier1\0\0 
        //buf[strlen(buf) - 1] = '\0';

        //printf("%s\n", buf);

        // jier1
        // jier2
        // jier5
        // shipan Liu
        // shipan Li

        // not right, because the last line is saved with \n and EOF, the buf will not save the \n but 
        // remains the last buf, which willn cause the last line duplicates

        // 现在是正确的方式：
        char * ret = fgets(buf, sizeof(buf), f_read);

        // fgets 读到文件尾会返回一个NULL
        if(ret == NULL) {
            break;
        }

        //我想去掉文件中每一行的换行符 \n ， 改成\0,  如： jier1\n\0   ---> jier1\0\0 
        buf[strlen(buf) - 1] = '\0';
        // 我在打印的时候， 又想要加上 换行符  
        printf("%s\n", buf);

    }

// 我们走else 的代码， 不走if 0 的代码
//下面是优化的代码
#else 
    char buf[1024] = {0};
    // if fgets returns some value , that means it reads
    while(fgets(buf, sizeof(buf), f_read)) {
        // get rid of the \n at the end of each line
        buf[strlen(buf) - 1] = '\0';
        printf("%s\n", buf);
    }

#endif
    fclose(f_read);
}







//  文件版的四则运算， 有一个文件大小不确， 每行内容都是一个四则运算(需要你随机产生50 个，
// 随机数在 1到100， 运算符: +-*/)， 还没有算出结果， 写一个程序，
//  自动算出其结果后修改文件。



 




int main() {
    //write1();
    //read1();
    //test03();
    //test04();  
    test05(); 
    return 0;
}