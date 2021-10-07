#ifndef INTERCODE
#define INTERCODE
/*
中间代码定义如下
LABEL x :               定义标号x
FUNCTION f:             定义函数f
x := y                  赋值操作
x := y + z              加法操作
x := y - z              减法操作
x := y * z              乘法操作    
x := y / z              除法操作
x := y % z              模除操作
x := &y                 取y的地址赋给x
x := *y                 取以y值为地址的内存单元的内容赋给x
*x := y                 取y值赋给以x值为地址的内存单元
GOTO x                  无条件跳转至标号x
IF x [relop] y GOTO z   如果x与y满足[relop]关系则跳转至标号z
RETURN x                退出当前函数并返回x值
DEC x [size]            内存空间申请, 大小为4的倍数
ARG x                   传实参
x := CALL f             调用函数, 并将其返回值赋给x
PARAM x                 函数参数申明
READ x                  从控制台读取x的值 // 目前不用
WRITE x                 从控制台打印x的值 // 目前不用
*/
#include "node.h"
#include "list.h"



typedef struct ArgList_* ArgList;     // 函数实参链表
typedef struct Variable_* Variable;   // 变量
typedef struct Operand_* Operand;     // 操作数
typedef struct InterCode_* InterCode; // 中间代码节点
typedef struct CodeList_* CodeList;   // 中间代码链表


typedef struct Label_No_* Label_No;   // 记录Label的链表 与多余标签删除优化有关

struct Operand_{
    enum{
        OP_VARIABLE,  // 变量
        OP_CONSTANT,  // 常量
        OP_ADDRESS,   // 地址
        OP_LABEL,     // 标签
        OP_ARR_STRU,  // 数组 结构体-不用
        OP_TEMP       // 暂时变量
    } kind;
    union{
        int var_no;   // 变量序号
        int label_no; // 代码label序号
        int val;      // 用于常数
        int temp_no;  // 暂时变量序号  主要这些的作用就是作为唯一标示--每一个变量都是不同名的--从而解决作用域问题
    }u;
};

struct InterCode_{
    enum{
        IR_ASSIGN,  // x := y 
        IR_LABEL,   // LABEL x :
        IR_PLUS,    // x := y+z
        IR_MINUS,   // x := y-z
        IR_MUL,     // x := y*z
        IR_DIV,     // x := y/z
        IR_MOD,     // x : = y%z 
        IR_FUNC,    // FUNCTION f :
        IR_GOTO,    // GOTO x
        IR_IFGOTO,  // IF x [relop] y GOTO z 
        IR_RET,     // x = *y
        IR_DEC,     // DEC x [size] 内存空间申请-大小为4的倍数 用于数组
        IR_ARG,     // ARG x
        IR_CALL,    // x := CALL f
        IR_PARAM,   // PARAM x
        IR_READ,    // 不用
        IR_WRITE,   // 不用
        IR_RETURN,  // RETURN x
        IR_CHANGE_ADDR, // *x := y 取y值以赋给x值为地址的内存单元
        IR_GET_ADDR,     // x := &y 取y的地址赋给x  // 名字可能起的不是很好
        IR_ENDFUNC       // 函数声明结束 
        } kind;
    union{
        Operand op;
        char *func;
        struct{Operand right, left; } assign; // 赋值 x := y
        struct{Operand result, op1, op2; } binop; // 双目 x := y + z
        struct{Operand x, y, z; char *relop;} if_goto; // IF x [relop] y GOTO z
        struct{Operand x; int size;} dec;         // DEC x [size]
        struct{Operand result; char *func;} call; // x := CALL f
    }u;
};
// 中间代码链表
struct CodeList_{
    InterCode code;
    CodeList prev, next;
};

// 函数*实参*链表
struct ArgList_{
    Operand args; 
    ArgList next;
};

// 变量链表 
struct Variable_{
    char* name;
    Operand op;
    Variable next;
};

// 记录Label的链表 与多余标签删除优化有关
struct Label_No_
{
	int no;
	Label_No next;
};


int var_num,label_num,temp_num;  // 标号表示唯一标识符


Operand new_constant();          
Operand new_temp();
Operand new_label();
Operand new_fparam();
Operand op_from_var(char* var);   

InterCode new_InterCode(int kind);
CodeList new_CodeList(InterCode code);
// 合并两个CodeList
CodeList join(CodeList head, CodeList body);
CodeList translate_Operand(Operand op,int IR_KIND); // Operand

void tranlate_InterCode(treeNode*root, char*file);
void start_translate(treeNode* root); // 递归翻译
void insert_code(CodeList code); // 插入到总代码中

char *Operand_toString(Operand op);
char* InterCode_toString(InterCode code);

CodeList translate_Exp(treeNode *Exp, Operand place);
CodeList translate_AddExp(treeNode *AddExp, Operand place);
CodeList translate_MulExp(treeNode *MulExp, Operand place);
CodeList translate_Number(treeNode *Number, Operand place);
CodeList translate_PrimaryExp(treeNode *PrimaryExp,Operand place) ;
CodeList translate_UnaryExp(treeNode *UnaryExp, Operand place);
CodeList translate_Stmt(treeNode* Stmt, Operand label_1, Operand label_3);
CodeList translate_Block(treeNode *Block, Operand label_1, Operand label_3, int isFuncBlock);
CodeList translate_BlockItem_rep(treeNode *BlockItem_rep, Operand label_1, Operand label_3);
CodeList translate_BlockItem(treeNode *BlockItem, Operand label_1, Operand label_3);
CodeList translate_LVal(treeNode *LVal, Operand place, int isLeft);
//CodeList translate_Exp_rep(treeNode *Exp_rep);
CodeList translate_FuncDef(treeNode *FuncDef);
CodeList translate_FuncRParams(treeNode *FuncRParams, ArgList *arg_list);
CodeList translate_comExp_rep(treeNode *comExp_rep, ArgList *arg_list);
CodeList translate_Exp_rep(treeNode *Exp_rep, Operand place, int* index);
InterCode new_InterCode(int kind);
CodeList new_CodeList(InterCode code);
CodeList join(CodeList head, CodeList body);
CodeList translate_Operand(Operand op, int IR_KIND);
void insert_code(CodeList code);
Operand new_constant(int val);
Operand new_temp();
Operand new_label();
Operand new_var();
Operand op_from_var(char* var);
char *Operand_toString(Operand op);
char* InterCode_toString(InterCode code);
CodeList translate_InterCode(treeNode* root, char* file);
void start_translate(treeNode* root);
CodeList translate_CompUnit(treeNode *CompUnit);
CodeList translate_Decl(treeNode *Decl);
CodeList translate_VarDecl(treeNode *VarDecl);
CodeList translate_VarDef(treeNode *VarDef);
CodeList translate_ConstDef(treeNode *ConstDef);
CodeList translate_ConstInitVal(treeNode *ConstInitVal, Operand place);
CodeList translate_InitVal(treeNode *InitVal,Operand place, int arrayDim, int* arrayDimList);
treeNode* translate_InitVal_array(treeNode *r, Operand place, int arrayDim, int* arrayDimList, int index, int* numCount, int numCountLim, CodeList* code);
CodeList translate_LOrExp(treeNode *LOrExp, Operand label_true, Operand label_false);
CodeList translate_LAndExp(treeNode *LAndExp, Operand label_true, Operand label_false);
CodeList translate_EqExp_Calc(treeNode *EqExp, Operand place);
CodeList translate_RelExp_Calc(treeNode *RelExp, Operand place);
CodeList translate_EqExp(treeNode *EqExp, Operand label_true, Operand label_false);
CodeList translate_RelExp(treeNode *RelExp, Operand label_true, Operand label_false);
CodeList translate_Cond(treeNode *Condition, Operand label_true, Operand label_false);
CodeList translate_ConstExp(treeNode *ConstExp, Operand place);
CodeList translate_ComVarDef_rep(treeNode *ComVarDef_rep);
CodeList translate_ComConstDef_rep(treeNode *ComConstDef_rep);
CodeList translate_ConstDecl(treeNode *Decl);
void printCodeList(CodeList tmp);

void deleteCode(CodeList c);
void calcConst();


CodeList code_head, code_tail;  // 中间代码链表
Variable var_head,var_tail;     // 变量链表

#endif