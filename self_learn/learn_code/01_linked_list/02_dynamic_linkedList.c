//C语言 链表基础知识清晰讲解（黑马）
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>


// 先定义节点：
struct LinkNode {
    int data;
    struct LinkNode *next;
};

 

// 初始化linkedlist
struct LinkNode* Init_LinkedList() {
    // 01创建 头指针， 分配地址， 指向第一个元素
    struct LinkNode *header = malloc(sizeof(struct LinkNode));
    // 初始化头节点的 data 和 next, 这个头节点是个dummy， 不算是第一个元素
    header->data = -1;
    header->next = NULL;

    // 出啊关键尾部指针, 把header 赋给 pRear， pRead就代表了 header
    struct LinkNode *pRear = header;

    //02现在开始插入数据
    int val = -1;
    while(true) {
        printf("please insert the value");
        scanf("%d", &val);
        // 输入结束
        if(val == -1) {
            break;
        }

        // 想要把输入要的值当作node 插进去， 首先要创建节点, 开辟新的空间
        struct LinkNode *newnode = malloc(sizeof(struct LinkNode));
        newnode->data = val;
        newnode->next = NULL;

        // node 开辟创建完之后， 需要插入到链表中， 使用尾部节点, 
        pRear->next = newnode;  // 从 Prear->NULL  到 Prear->newcode

        // 完成这一步之后， header 的替身（pRear） 帮助 header 的 next 指向了newnode， 
        // 接下来 pRead 不再是 header 的替身， 因为下一步 pRead 变成了newnode 的替身， 
        // 所以接下来header 一直指向 的是 第一个插入的newnode， 牛逼逻辑。 pRear创建的真好

        // 这个时候， newnode 变成了pRear， pRear不再是 header 的替身， 而是 newcode 的替身。 
        pRear = newnode;
    }
    // retuen头节点，相当于 返回链表
    return header;
}

// 遍历
int forEach_linkedList(struct LinkNode *header) {
    // 说明这个链表根本旧不存在，或者这个header就不是链表
    if(header == NULL) {
        // 说明失败
        return 0;
    }

    // 辅助指针
    struct LinkNode *pCurrent = header->next;
    while(pCurrent != NULL) {
        printf("%d ", pCurrent->data);
        pCurrent = pCurrent->next;
    }
    // 说明成功
    return 1;
} 


 
// 在oldval的前面插入一个新的newval
//参数 struct LinkNode *header 代表一个linkedList
// 问题是 oldval 可以找到， 但是之前的哪个node 怎么找到？ 又不是双向的链表， 因此使用两个pointer
struct LinkNode* InsertByValue_LinkList(struct LinkNode *header, int oldval, int newval) {
    // 两个指针 平行向右移动， when the 2. pointer->data is oldval, the the 1.pointer->data is the preview node before oldval
    if(header == NULL) { 
        // 返回旧的linkedList
        return header;
        // 或者 直接 return
    }

    // 定义两个 辅助指针
    struct LinkNode *pPrev = header;
    struct LinkNode *pCurrent = pPrev->next;

    // 焦点要放到pCurrent上， 因为你要找到oldvalue
    while(pCurrent != NULL) {
        if(pCurrent->data == oldval) {
            break;
        }

        // if not find, right shift the two pointers
        pPrev = pCurrent;
        pCurrent = pCurrent->next;
    }

    // 假如linkedList里面没有oldval， 说明我没有进入break 说明我遍历到了最后， 最后pCurrent->next的值是NULL：
    if(pCurrent->next == NULL) {
        // list 里面根本没有oldval
        // 返回旧的linkedList
        return header;
    }

    //先为newVal开辟空间
    struct LinkNode *newNode = malloc(sizeof(struct LinkNode));
    newNode->data = newval;
    newNode->next = NULL;

    // newNode 插入到链表中
    newNode->next = pCurrent; 
    pPrev->next = newNode; 

    return header;
}


// 删除值为val的结点
void RemoveByValue_LinkedList(struct LinkNode *header, int delValue) {
    if(header == NULL) {
        return;
    }

    //定义连个辅助变量
    struct LinkNode *pPrev = header;
    struct LinkNode *pCurrent = pPrev->next;

    while(pCurrent != NULL) {

        if(pCurrent->data == delValue) {
            break;
        }
        // 同步移动两个指针向右
        pPrev = pCurrent;
        pCurrent = pCurrent->next;
    }
    
    // delValue 没有在 链表中
    if(pCurrent == NULL) {
        return;
    }

    //  now pCurrent is pointing to delValue which is about to be deleted
    // delete, 重新建立节点的关系。
    pPrev->next = pCurrent->next;
    free(pCurrent);
    pCurrent = NULL;

}

// 清空
// 除了header保留之外， 其他的节点都遍历依次释放空间
void Clear_LinkedList(struct LinkNode *header) {
    //假如 header 是NULL 的话， 说明这个链表根本不存在
    if(header == NULL) {
        return;
    }

    // 创建辅助pointer
    struct LinkNode *pCurrent = header -> next;
    while(pCurrent != NULL) {
        //保存一下当前node中的next， 在当前node被free之前
        struct LinkNode *pNext = pCurrent->next;
        free(pCurrent);
        //update pCurrent, let pCurrent points to next point
        pCurrent = pNext; 
    }

    // 完事之后，把header中的 next更新一下
    header->next = NULL;
}


void test01() {
    /*test initial linkedList and loop the list*/
    int returnedValue = forEach_linkedList(Init_LinkedList());
    printf("%d", returnedValue);
}

void test02() {
    /*test insert newval ahead of the oldval*/
    struct LinkNode *oldLinkedListHeader = Init_LinkedList();
    struct LinkNode *newLinkedListHeader = InsertByValue_LinkList(oldLinkedListHeader, 3, 100);
    forEach_linkedList(newLinkedListHeader);
}

void test03() {
    // test for clear the LinkedList
    struct LinkNode *oldLinkedListHeader = Init_LinkedList();
    Clear_LinkedList(oldLinkedListHeader);
    // 打印一下
    forEach_linkedList(oldLinkedListHeader);
}




int main() {

    //test01();

    //test02();

    test03();
    return 0;

}