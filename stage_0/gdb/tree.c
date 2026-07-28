#include <string.h>

#include <stdlib.h>
#include "tree.h"

struct tnode* createTree(char *symbol, struct tnode* left, struct tnode* right) {
  struct tnode *k = (struct tnode*)malloc(sizeof(struct tnode));
  k->symbol = strdup(symbol);
  k->left = left;
  k->right = right;
  return k;

}

void infixtoprefix (struct tnode* root) {
  if (root != NULL) {
    printf("%s ", root->symbol);
    infixtoprefix(root->left);
    infixtoprefix(root->right);
  }
}