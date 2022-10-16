# finished
## To Do
# - Targets all, clash, clean
# - Zwischenprodukte (plist.o) erstellen
# - vorgeschriebene Kompilierflags
##



CC = gcc
CFLAGS = -std=c11 -pedantic -Wall -Werror -D_XOPEN_SOURCE=700

# 按照题意，应该有一个 all 标题， all 表示我现在有这么多目标需要编译
all: clash

# 按照题意，应该有一个 clash 标题，我 要编译 clash， 那么我需要  clash.o plist.o
clash: clash.o plist.o
	$(CC) -o clash clash.o plist.o

# clash.o  的编译 需要 clash.c plist.h 
# gcc -c  是生成 .o 文件的
# In clash.c wird plist.h inkludiert, und ist somit als Abhängigkeit anzugeben.
clash.o: clash.c plist.h
	$(CC) $(CFLAGS) -c clash.c    


# In plist.c wird plist.h inkludiert, und ist somit als Abhängigkeit anzugeben.
plist.o: plist.c plist.h
	$(CC) $(CFLAGS) -c plist.c

# 按照题意，应该有一个 clean 标题， -f 是 force 的意思
clean:
	rm -f clash clash.o plist.o
	
.PHONY: all clean



#    .PHONY是一个伪目标，可以防止在Makefile中定义的命令的目标  和   工作目录下的实际文件   出现名字冲突，（假如你在当前目录下碰巧有一个文件 叫clean）
#    另一种是提交执行makefile时的效率。
