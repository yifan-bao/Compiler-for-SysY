//syntax tree 
#ifndef _NODE_H
#define _NODE_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    int lineNo;
    char name[16];
    char value[32];
    int int_val;
    struct Node *child;
    struct Node *next;
    struct Node *father; // used in sdt parse
}treeNode;
treeNode *newNodeOp(char* name);
treeNode *newNodeString(char* name, char* value);
treeNode *newNodeInt(char* name, int int_val);
treeNode *newNodeError();
void addChild(treeNode* parent, treeNode* child);
void printTree(treeNode* r, int count); // debug syntax tree

#endif

