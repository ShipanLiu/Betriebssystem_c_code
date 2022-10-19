#include<stdio.h>
#include<stdlib.h>


// 先定义链表的node， 使用struct
struct LinkNode {
    int data;
    // 指向下一个node， 因此也应该是 struct LinkNode 类型
    struct LinkNode *next;  
};

// for linked list, you have to create some nodes and link them together
void test() {
    struct LinkNode node1 = {10, NULL};
    struct LinkNode node2 = {20, NULL};
    struct LinkNode node3 = {30, NULL};
    struct LinkNode node4 = {40, NULL};
    struct LinkNode node5 = {50, NULL};
    struct LinkNode node6 = {60, NULL};
    struct LinkNode node7 = {70, NULL};

    node1.next = &node2;
    node2.next = &node3;
    node3.next = &node4;
    node4.next = &node5;
    node5.next = &node6;
    node6.next = &node7;

    // 如何遍历这个链表， 通过定义一个新的指针（辅助指针变量）
    struct LinkNode *pCurrent = &node1;
    // 当pCurrent 指向NULL 的时候，说明已经到了最后一个节点
    while(pCurrent != NULL) {
        printf("%d-", pCurrent -> data);
        
        //pCurrent这个时候全权待变一个node， 现在 指针 要指向 下一个node 的地址
        pCurrent = pCurrent -> next;
    }

}


int main() {
    test();
    return 0;
}