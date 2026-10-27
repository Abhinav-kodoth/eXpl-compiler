#include "translate.h"

llnode* createLlNode(char* label, int address) {
    llnode* temp = (llnode*)malloc(sizeof(llnode));
    temp->label = strdup(label);
    temp->address = address;
    temp->next = NULL;
    return temp;
}

llnode* insert(char* label, int address, llnode* head) {
    llnode* x = createLlNode(label, address);
    if (!head) {
        return x;
    }
    llnode* temp = head;
    while (temp->next) {
        temp = temp->next;
    }
    temp->next = x;
    return head;
}

int getAddress(char* label, llnode* head) {
    llnode* temp = head;
    while (temp) {
        if (strcmp(temp->label, label) == 0) {
            return temp->address;
        }
        temp = temp->next;
    }
    return -1;
}
