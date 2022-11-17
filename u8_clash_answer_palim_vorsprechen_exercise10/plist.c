#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include "plist.h"

/* Die Funktionen insertElement() und removeElement() bitte unveraendert lassen!
 * Falls Sie einen Bug in dieser Implementierung finden, melden Sie diesen bitte
 * an bs@lists.ruhr-uni-bochum.de
 */

static struct qel {
	pid_t pid;
	char *cmdLine;
	struct qel *next;
} *head;

// 类似于 struct qel* head;


/**
 *  \brief Invokes a callback function on each element in the list.
 *
 *     The callback function is passed the pid-command line pair for the current
 *     list element. The callback function shall return 0 to request further
 *     processing of the list. Any other return value will cause the early
 *     termination of the list walk. The callback function must not modify the
 *     process list.
 *
 *  \param callback Pointer to the function to be invoked for each list element.
 */

// walklist 接收一个 函数 作为 参数
void walkList(int (*callback) (pid_t, const char *)) {
	// TODO: implement me

    /**
     * Die Funktion walkList() soll einmal über alle Listeneinträge laufen und für jeden Eintrag die übergebene Callback-Funktion ausführen.   easy
     * Die Callback-Funktion soll die pid und die Kommandozeile der laufenden Hintergrundprozesse ausgeben. -> print_jobs Funktion in clash.c     easy
     * Die laufenden Hintergrundprozesse sind in der verketteten Liste gespeichert.  注意 在 linked list 里面 存放着
     * Wenn in clash.c "walkList(print_jobs);" aufgerufen wird und hier "callback(x->pid, x->cmdline)", dann werden die Parameter x->pid und x->cmdline an print_jobs gegeben.
	 *
	 * C 语言结构体之点运算符( . )和箭头运算符( -> )的区别
	 *
	 *  相同点：两者都是二元操作符，而且右边的操作数都是成员的名称。
		不同点：点运算符( . )的左边操作数是一个结果为结构的表达式；
        箭头运算符( -> )的左边的操作数是一个指向结构体的指针。

		1 typedef struct // 定义一个结构体类型：DATA
		2 	{
		3 		char key[10]; // 结构体成员：key
		4 		char name[20]; // 结构体成员：name
		5 		int age; // 结构体成员：age
		6 	}DATA;
		7
		8  DATA data; // 声明一个结构体变量
		9  DATA *pdata; // 声明一个指向结构体的指针
		10
		11 // 访问数据操作如下：
		12 data.age = 24; // 结构体变量通过点运算符( . )访问
		13 pdata->age = 24; // 指向结构体的指针通过箭头运算符( -> )访问
	 *
     */

    for (struct qel* x = head; x; x = x->next) {   // 中间的哪个单独的 x 可以替换成  x != NULL
        if (callback(x->pid, x->cmdLine)) {   // callback()就是 传过来的 print_jobs()  函数
                                              // // 正常 情况下 callback 返回 0，  if（0） 就不会执行 berak， 假如 不是 0， 那就执行 break。
            break;
        }
    }
}

int insertElement(pid_t pid, const char *cmdLine) {
	struct qel *lauf = head;
	struct qel *schlepp = NULL;

	while (lauf) {
		if (lauf->pid == pid) {
			return -1;
		}

		schlepp = lauf;
		lauf = lauf->next;
	}

	lauf = malloc(sizeof(struct qel));
	if (lauf == NULL) {
		return -2;
	}

	lauf->cmdLine = strdup(cmdLine);
	if (lauf->cmdLine == NULL) {
		free(lauf);
		return -2;
	}

	lauf->pid  = pid;
	lauf->next = NULL;

	/* Einhaengen des neuen Elements */
	if (schlepp == NULL) {
		head = lauf;
	} else {
		schlepp->next = lauf;
	}

	return pid;
}

int removeElement(pid_t pid, char *buf, size_t buflen) {
	if (head == NULL) {
		return -1;
	}

	struct qel *lauf = head;
	struct qel *schlepp = NULL;

	while (lauf) {
		if (lauf->pid == pid) {
			if (schlepp == NULL) {
				head = head->next;
			} else {
				schlepp->next = lauf->next;
			}

			strncpy(buf, lauf->cmdLine, buflen);
			if (buflen > 0) {
				buf[buflen-1]='\0';
			}
			int retVal = (int)strlen(lauf->cmdLine);

			/* Speicher freigeben */
			free(lauf->cmdLine);
			lauf->cmdLine = NULL;
			lauf->next = NULL;
			lauf->pid = 0;
			free(lauf);
			return retVal;
		}

		schlepp = lauf;
		lauf = lauf->next;
	}

	/* PID not found */
	return -1;
}
