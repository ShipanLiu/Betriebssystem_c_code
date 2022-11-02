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

static int insertElement(int data) {
    // not negative
    if(data < 0) {
        return -1;
    }

    // if the list has not initialized yet
    if(header == NULL) {
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
            if(pointer->value == data) {
                return -1;
            }
            break;
        }
    }


    //now add new data in the list
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
        printf("list is not initialized or the list is empty\n");
        return -1;
    }

    // if only header exits in the list
    if(header->next == NULL) {
        int headerValue = header->value;
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












