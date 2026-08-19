#include "labelTranslation.h"

void printLabelTable(struct labelAddressTable * lat) {
    struct labelNode * temp = lat->head ;

    while(temp) {
        printf("%s %d\n",temp->labelName, temp->address);
        temp = temp->next ;
    }
}

int searchLabel(struct labelAddressTable * lat, char * labelName) {
    int labelSize = strlen(labelName);

    char * newLabelName = (char *) malloc(sizeof(char) * labelSize) ;
    strncpy(newLabelName,labelName,labelSize);
    newLabelName[labelSize-1] = '\0';

    struct labelNode * temp = lat->head ;
    while(temp != NULL) {
        if(strcmp(temp->labelName,newLabelName) == 0) {
            return temp->address ;
        }
        temp = temp->next ;
    }
    return -1 ;
}

void addLabel(struct labelAddressTable * lat, char * labelName, int address) {
    struct labelNode * node = ( struct labelNode *) malloc(sizeof(struct labelNode));

    int labelSize = strlen(labelName) -1;
    node->labelName = (char *) malloc(sizeof(char) * labelSize);
    strncpy(node->labelName,labelName,labelSize);
    node->labelName[labelSize-1] = '\0';

    node->address = address ;
    node->next = NULL ;

    struct labelNode * temp = lat->head ;
    if(temp == NULL){
		lat -> head = node;
		return;
	}

    while(temp->next) {
        temp = temp->next ;
    }
    temp->next = node ;
}