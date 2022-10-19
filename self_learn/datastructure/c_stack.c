/*
c语言中实现 stack 的程序， 元素是 现先进后出

*/

#include <stdio.h>
#include <stdlib.h>


// first init the Node type

typedef struct Node {
    int data;
    struct Node* next;
}Node;

Node* initStack() {
    // in each empty stack, there is always a head node;
    Node* S = (Node*)malloc(sizeof(Node));
    S->data = 0;
    S->next = NULL;
    return S;
}

// 我把代表 Stack 的 header S传进来， 当作是 Stack
int isEmpty(Node* S) {
    if(S->data == 0 || S->next == NULL) {
        return 1;
    }else {
        return 0;
    }
}

void push(Node* S, int content) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->data = content;

    // node->next  现在指向  原来 S->next的指向
    node->next = S->next;
    // 插进来 之后， 就相当于插到 S 这个header 后面了, s->next 改变.
    S->next = node;
    //S->data记录的是 现在 栈里面实际上有多少插进来的。刚开始 是 0， 现在是 1；
    S->data++;    

}
int pop(Node* S) {
    Node* node = S->next;
    if(isEmpty(S)) {
        printf("no pop, empty!!!");
        return -1;
    } else {
        S->next = node->next;
        S->data--;
        int popNodeData = node->data;
        free(node);
        printf("you poped %d\n", popNodeData);
        return popNodeData;
    }
} 

// 遍历栈
void loop(Node* S) {
    Node* node = S->next;
    while(node) {
        printf("%d\n", node->data);
        node = node->next;
    }
    printf("NULL");
}

int main() {
    Node* S = initStack();
    push(S, 1);
    push(S, 2);
    push(S, 3);
    pop(S);
    pop(S);
    pop(S);
    loop(S);

    return 0;
}

 

