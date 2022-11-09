/*
T00.25

Shipan Liu  108019201553
Minhua Liu  108020210282

*/


#include <stdio.h>
#include <stdlib.h>

// define a node struct
typedef struct node{
    int value;
    struct node *next;
}node;

// define the header
static node* header = NULL;


// !!! bibiothekfunction do not make it static 
// !!! (because the other Aufgabe may will not have the insertEle function) 

static int insertElement(int data) {
    // not negative
    if(data < 0) {
        return -1;
    }

    // if the list has not initialized yet
    if(header == NULL) {
		
		//!!!  here better to cast  (node*)malloc(.....)
		
        header = malloc(sizeof(node));
        // Fehlerbehandlung
        if(header == NULL) {
            return -1;
        }

        header->next = NULL;

        header->value = data;
        return data;
    }


    //if the list is already initialized
    // loop the list by using pointer
    node* pointer = header;

    //check duplicate
    while(1) {
        if(pointer->next != NULL) {
            if(pointer->value == data) {
                return -1;
            }
            pointer = pointer->next;
        } else {
			// pointer 没有 后继者， 现在就要查看 pointer 本身的值
            if(pointer->value == data) {
                return -1;
            }
            break;
        }
    }


    //now add new data in the list
	//!!! clear code :  (node*)malloc(...)
	//!!! you can use calloc, the value will be initialized 
	//!!! ==> elem *newElem = calloc(1, sizeof(elem));  1 代表 我只要 分配一个。  
    node* newItem = malloc(sizeof(node));

    //Fehlerbehandlung
    if(newItem == NULL) {
        return -1;
    }

    newItem->next = NULL;
    newItem->value = data;

    pointer->next = newItem;


    return data;

}

// printAll() is only for test
static void printAll() {
    if(header == NULL) {
        printf("empty");
    }
    node* pointer = header;
    int i = 1;
    while(pointer->next != NULL) {
        printf("%dst value: %d\n", i,pointer->value);
        pointer = pointer->next;
        i++;
    }
    printf("%dst value: %d\n", i,pointer->value);
}



static int removeElement(void) {

    //if the list has not initialized yet
    if(header == NULL) {
  /*I----> +--------------------------------------------------------------------+
         | Laut Aufgabenstellung soll keine Fehlermeldung ausgegeben werden   |
         | (-0.5): Die Funktionen insertElement() und removeElement() können |
         | als Bibliotheksfunktionen angesehen werden. Bei solchen Funktionen |
         | soll der Aufrufer entscheiden, ob er eine Fehlermeldung ausgeben   |
         | will oder nicht.                                                   |
         +-------------------------------------------------------------------*/
        // printf("list is not initialized or the list is empty\n");  // 这句话 不应该打印出来， 不能有fehlermeldungen 的出现。
        return -1;
    }

    // if only header exits in the list
    if(header->next == NULL) {
        int headerValue = header->value;
		//!!!  要先 free ， 因为free 要求一个 adresse
        free(header);  
        header = NULL;
        return headerValue;
    }


    //remove the youngest element
    node* pointer1 = header;
    node* pointer2 = header;
    while(pointer1->next != NULL) {
        pointer1 = pointer1->next;
    }

    while(pointer2->next != pointer1){
        pointer2 = pointer2->next;
    }

    int removedData = pointer1->value;
    pointer2->next = NULL;
    free(pointer1);
    return removedData;
}

int main(int argc, char* argv[]) {


    printf("insert 47: %d\n", insertElement(47));
	printf("insert 11: %d\n", insertElement(11));
	printf("insert 23: %d\n", insertElement(23));
	printf("insert 11: %d\n", insertElement(11));

	printf("remove: %d\n", removeElement());
	printf("remove: %d\n", removeElement());

	// TODO: add more tests


    printf("push 1: %d\n", insertElement(1));
    printf("push 2: %d\n", insertElement(2));
    printf("push 3: %d\n", insertElement(3));
    printf("push 3: %d\n", insertElement(3));
    printf("push 3: %d\n", insertElement(3));
    printf("push 6: %d\n", insertElement(6));
    printf("push 7: %d\n", insertElement(7));
    printf("push 8: %d\n", insertElement(8));

    printf("##############################\n");
    printAll();
    printf("##############################\n");

    printf("pop: %d\n", removeElement());
    printf("pop: %d\n", removeElement());
    printf("pop: %d\n", removeElement());
    printf("pop: %d\n", removeElement());
    printf("pop: %d\n", removeElement());
    printf("pop: %d\n", removeElement());
    // printf("pop: %d\n", removeElement());
    // printf("pop: %d\n", removeElement());

    printf("##############################\n");
    printAll();
    printf("##############################\n");

    exit(EXIT_SUCCESS);
}
