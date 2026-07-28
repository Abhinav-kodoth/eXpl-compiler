typedef struct tnode {
  char *symbol;
  struct tnode *left, *right;
} tnode;

// prints the prefix expressions
void infixtoprefix(struct tnode* root);

/*Create  tnode*/
struct tnode* createTree(char *symbol, struct tnode* left, struct tnode* right);