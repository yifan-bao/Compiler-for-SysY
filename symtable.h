#ifndef _SYMTABLE_H
#define _SYMTABLE_H
#include "node.h"
#include "list.h"
#include "vector.h"
#include "intercode.h"

typedef struct Type_ *Type;
struct Type_
{
    enum
    {
        basic, 		// for int variables and function return type int
        array,		// for int array variables
        constant,	// for const int variables
        constArray,	// for const int array variables
        empty       // for function return type void and combine type error
    } kind;
    // for array & constArray
    int size;
};

struct paraItem //参数（变量、常量）
{
    char name[16];  //参数名
    List type;      //参数类型
    int *const_val; //常数组中的值，高维数组用一维数组存储
    int scope;      //作用域层数
    Operand op;     //对应的中间代码operand
};
typedef struct paraItem *Para;

struct funcItem //函数
{
    char name[16];
    int lineno;
    Type retType;       //返回类型
    int paraNum;        //参数的个数
    vector paraList;    //参数表
};
typedef struct funcItem *Func;

struct attrInfo
{
    int const_val; //常数表达式的值
};

int sdtParse(treeNode *r);
void initScopeTable();
void addScope();
void exitScope();
void printType();
void handleError(int no, int lineno);
int checkReserveNames(char* name);
Para querySymTable_para(char* name);
int insertSymTable_para(Para r, int lineno);
int sdtParseExp(treeNode *r, int isIdentConst, int flag, List list);
int sdtParseLVal(treeNode *r, int isIdentConst, int flag, List list);
int sdtParseNumber(treeNode *r);
int sdtParsePrimaryExp(treeNode *r, int isIdentConst, int flag, List list);
int sdtParseUnaryExp(treeNode *r, int isIdentConst, int flag, List list, int isCond);
int sdtParseMulExp(treeNode *r, int isIdentConst, int flag, List list, int isCond);
int sdtParseAddExp(treeNode *r, int isIdentConst, int flag, List list, int isCond);
int sdtParseConstExp(treeNode *r);
void sdtParseConstDecl(treeNode *r);
void sdtParseConstExp_rep(treeNode *r, List *type);
treeNode* getNextToken(treeNode *r);
int getArraySize(int* a, int index, int size);
void getArrayDimList(int *a, Para para);
treeNode* sdtParseConstInitVal_Array(treeNode *r, Para *para, int* a, int arrayDim, int index, int paraCount, int* numCount, int numCountLim);
void sdtParseConstInitVal(treeNode *r, Para *para);
void sdtParseConstDef(treeNode *r);
void sdtParseComConstDef_rep(treeNode* r);

void sdtParseVarDecl(treeNode *r);
void sdtParseVarDef(treeNode *r);
void sdtParseComVarDef_rep(treeNode *r);
void sdtParseInitVal(treeNode *r, Para *para);
treeNode* sdtParseInitVal_Array(treeNode *r, Para *para, int* a, int arrayDim, int index, int paraCount, int* numCount, int numCountLim);
void sdtParseDecl(treeNode *r);

Func querySymTable_func(char* name);
int insertSymTable_func(Func r, int lineno);

void sdtParseFuncDef(treeNode *r);
void sdtParseLbEXPRb_rep(treeNode *r, List *type);
vector sdtParseFuncFParams(treeNode *r);
void sdtParseComFuncFParam_rep(treeNode *r, vector paraList);
void sdtParseFuncFParam(treeNode *r, vector paraList);
void sdtParseBlock(treeNode *r, int flag, int noAdd);
void sdtParseBlockItem_rep(treeNode *r, int flag);
void sdtParseBlockItem(treeNode *r, int flag);
void sdtParseStmt(treeNode *r, int flag);

void sdtParseFuncRParams(treeNode* r, vector paraList);

int sdtParseCond(treeNode *r);
int sdtParseLOrExp(treeNode *r);
int sdtParseLAndExp(treeNode *r);
int sdtParseEqExp(treeNode *r);
int sdtParseRelExp(treeNode *r);

int para_exist(char *name, int scope);

#endif