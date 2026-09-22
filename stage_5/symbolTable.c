#include "symbolTable.h"
#include "exptree.h"

int bindAddress = STACK_BASE ;
int localBinding = -1;

struct Lsymbol * createLSTFromParams(struct ParamStruct * paramList) {
    struct Lsymbol * Lhead = NULL ;
    struct ParamStruct * temp = paramList ;
    int count = countParameterList(paramList);
    int i = 1;

    int bindingOffset = -3 ;

    while(temp) {
        Lhead = insertToLocalTable(Lhead,temp->name,temp->type);

        struct Lsymbol * newEntry = findLocalSymbol(Lhead,temp->name);
        if(newEntry != NULL) {
            newEntry->binding = bindingOffset - count + i;
        }
        temp = temp->next ;
        i++ ;
    }
    localBinding = 1;
    return Lhead; 
}

struct ParamStruct * createParamStructNode(char * name, int type) {
    struct ParamStruct * node = (struct ParamStruct *) malloc(sizeof(struct ParamStruct));

    node->name = name ;
    node->type = type ;
    node->next = NULL ;

    return node ;
}

struct ParamStruct * insertToParamList(struct ParamStruct * head,char * name, int type ) {
    struct ParamStruct * node = createParamStructNode(name,type);

    struct ParamStruct * temp = head ;
    if(temp == NULL) {
        head = node ;
        return head ;
    }
    while(temp->next) {
        temp = temp->next ;
    }
    temp->next = node ;

    return head ;
}

struct ParamStruct * findinParamList(struct ParamStruct * head, char * name) {
    struct ParamStruct * temp = head ;

    while(temp) {
        if(strcmp(temp->name,name) == 0) {
            return temp ;
        }
        temp = temp->next ;
    }

    return NULL ;
}

struct Gsymbol* createSymbolNode(char* name, int type, int size1, int flabel) {
    struct Gsymbol * node = (struct Gsymbol *) malloc(sizeof(struct Gsymbol));

    node->name = name ;
    node->type = type ;
    node->size1 = size1 ;
    node->next = NULL ;
    node->binding = bindAddress ;
    bindAddress += size1 ;
    node->flabel = flabel ;
    return node ;
}

struct Lsymbol *createLocalSymbolNode(char * name ,int type) {
    struct Lsymbol * node = (struct Lsymbol *) malloc(sizeof(struct Lsymbol));

    node->name = name ;
    node->type = type ;
    node->next = NULL ;
    node->binding = localBinding;
    localBinding++ ;
    return node ;
}

struct Lsymbol * findLocalSymbol(struct Lsymbol * head, char * name) {
    if(!head) {
        return NULL ;
    }
    struct Lsymbol * temp = head ;

    while(temp) {
        if(strcmp(temp->name,name) == 0) {
            return temp ;
        }
        temp = temp->next ;
    }
    return NULL ;
}


struct Gsymbol * find(struct Gsymbol * head, char * name) {
    if(!head) return NULL;

    struct Gsymbol * temp = head;
    while(temp) {
        if(strcmp(temp->name,name) == 0) {
            return temp ;
        }
        temp = temp->next ;
    }

    return NULL ;
}

struct Lsymbol * insertToLocalTable(struct Lsymbol * head, char * name, int type) {
    if(findLocalSymbol(head,name) != NULL) {
        printf("Redeclaration of variable\n");
        exit(1);
    }

    struct Lsymbol * temp = head ;
    struct Lsymbol * node = createLocalSymbolNode(name,type);

    if(temp == NULL) {
        head = node ;
        return head ;
    }

    while(temp->next) {
        temp = temp->next ;
    }
    temp->next = node ;
    return head ;
}


struct Gsymbol * insertTable(struct Gsymbol * head,char * name, int type, int size1,int flabel) {
    if(find(head,name) != NULL) {
        printf("Redeclaration of variable\n");
        exit(1);
    } 

    struct Gsymbol * temp = head ;
    struct Gsymbol * node = createSymbolNode(name,type,size1,flabel); 

    if(temp == NULL) {
        head = node ;
        return head ;
    } 
    
    while(temp->next) {
        temp = temp->next ;
    }
    temp->next = node ;
    return head ;
}

void printSymbolTable(struct Gsymbol * head) {
    struct Gsymbol * node = head ;

    while(node) {
        printf("Name: %s, Address: %d, Type: %d\n",node->name,node->binding,node->type);
        node = node->next ;
    }
}

int countParameterList(struct ParamStruct * paramlist) {
    struct ParamStruct * temp = paramlist ;
    int count = 0;

    while(temp) {
        count++ ;
        temp = temp->next ;
    }
    return count ;
}

void printLocalSymbolTable(struct Lsymbol * head) {
    struct Lsymbol * temp = head ;

    while(temp) {
        printf("Name: %s, Type: %d, Binding: %d\n",temp->name, temp->type,temp->binding);
        temp = temp->next ;
    }
}

void printtemp(struct ParamStruct * head) {
    struct ParamStruct * temp = head ;

    while(temp) {
        printf("Name: %s, Type: %d\n",temp->name, temp->type);
        temp = temp->next ;
    }
}

void insertParamListToGST(struct Gsymbol * head, char * name ,struct ParamStruct * paramList) {
    struct Gsymbol * temp = find(head,name);
    if(temp == NULL) {
        printf("Error while inserting paramList\n");
        exit(1);
    }
    temp->paramList = paramList ;
}

void compareParamList(struct Gsymbol * currentFunc,struct ParamStruct * currParamList) {
    struct ParamStruct * temp = currentFunc->paramList ;
    struct ParamStruct * compare = currParamList ;

    
    while(temp != NULL && compare != NULL ) {
        if((strcmp(temp->name,compare->name) != 0) || (temp->type != compare->type)) {
            printf("Type/Name mismatch in parameter for function definition\n");
            exit(1);
        }
        temp = temp->next ;
        compare = compare->next ;
    }

    if(temp != NULL || compare != NULL) {
        printf("Too few args or Too many args\n");
        exit(1);
    }
        
}

void checkDeclAndDefn(struct Gsymbol * currentFunc,int type) {
    int funcType = currentFunc->type ;

    if(funcType != type) {
        printf("Function Declaration return type and Definition return type does not match\n");
        exit(1);
    }
}