#include "node.h"
extern int yylineno;
#define NEW(p) ((p) = malloc(sizeof *(p)))
#define NEW0(p) memset(NEW(p), 0, sizeof *(p))


/* new Node */

treeNode *newNodeOp(char* name)
{
	treeNode *node;
	NEW0(node);
	//node->lineNo=yylineno;
	strcpy(node->name,name);
	node->child=NULL;
	node->next=NULL;
	node->father=NULL;
	node->lineNo = yylineno;
	return node;
}

treeNode* newNodeString(char *name,char *value)
{
	treeNode *node;
	NEW0(node);
	//node->lineNo=yylineno;
	strcpy(node->name,name);
	strcpy(node->value,value);
	node->child=NULL;
	node->next=NULL;
	node->father=NULL;
	node->lineNo = yylineno;
	return node;
}

treeNode *newNodeInt(char* name, int int_val)
{
	treeNode *node;
	NEW0(node);
	//node->lineNo=yylineno;
	strcpy(node->name,name);
	node->child=NULL;
	node->next=NULL;
	node->int_val=int_val;
	node->father=NULL;
	node->lineNo = yylineno;
	return node;
}

treeNode *newNodeError() {
	treeNode *node;
	NEW0(node);
	strcpy(node->name,"ERROR");
	node->lineNo=yylineno;
	node->child=NULL;
	node->next=NULL;
	node->father=NULL;
	return node;
}

/*give father a child*/
void addChild(treeNode* parent,treeNode* child)
{
	if(child == NULL)
	{
		parent->child = NULL;
		return;
	}
	if(parent != NULL && child != NULL)
	{
		child->next = parent->child; // add to head of children list
		parent->child = child;
		child->father = parent;
		parent->lineNo = child->lineNo; // update the row of father node
	}
}


/* print the syntax tree */
void printTree(treeNode* r,int count)
{
	if(r==NULL)return;
	if(r->child==NULL)		//token
	{	
		int i=0;		
		for(;i < count;i++)	// retract 
		{
			printf("  ");
		}
		//not all nodes need to print value
		if(strcmp(r->name,"TYPE")==0||strcmp(r->name,"INTCONST")==0||strcmp(r->name,"ID")==0)
		{
			if(strcmp(r->name,"INTCONST")==0)
				printf("%s: %d\n",r->name,r->int_val);
			else
				printf("%s: %s\n",r->name,r->value);
		}
		else
		{
			printf("%s\n",r->name);
		}
	}
	else 				//non-terminal
	{
		int i=0;
		for(;i<count;i++)
		{
			printf("  ");
		}
		printf("%s(%d)\n",r->name,r->lineNo);
		treeNode *p=r->child;
		//traverse the child nodes
		while(p!=NULL){
			printTree(p,count+1);
			p=p->next;
		}
	}
	return;
}