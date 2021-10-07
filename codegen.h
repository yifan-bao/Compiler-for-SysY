#ifndef CODEGEN
#define CODEGEN

#include "intercode.h"

typedef struct asmCodeList_ *asmCodeList;

struct asmCodeList_{
    char* code;
    asmCodeList prev, next;
};

asmCodeList new_asmCodeList(char* code);
// 合并两个CodeList
asmCodeList asm_join(asmCodeList head, asmCodeList body);
void codegen(CodeList cl);
asmCodeList gen_header();
asmCodeList gen_ir_func(InterCode ir);
int getreg(Operand op);
asmCodeList load_to_reg(Operand op, int regid);
asmCodeList gen_ir_label(InterCode ir);
asmCodeList gen_ir_assign(InterCode ir);
asmCodeList gen_ir_plus(InterCode ir, char* op_type);
asmCodeList gen_ir_goto(InterCode ir);
asmCodeList gen_ir_ifgoto(InterCode ir);

asmCodeList gen_ir_call(InterCode ir);
asmCodeList gen_ir_func(InterCode ir);
asmCodeList gen_ir_arg(InterCode ir);
asmCodeList gen_ir_params(InterCode ir);
asmCodeList gen_ir_return(InterCode ir);
asmCodeList gen_ir_dec(InterCode ir);
asmCodeList gen_ir_address(InterCode ir);


void initRegs();
asmCodeList swReg(int index);
asmCodeList lwReg(int index, Var_t *var);
void delVars();
void addVars(Var_t *var);
Var_t* findVar(char *name);

// 应该是一个函数内部的变量链表 应该是所有变量都存储到栈空间
typedef struct Var_t {
    char *name;   // 变量名字 
    int reg;      // 应该是对应的寄存器的序号
    int offset;   // 应该是相对于"fp"的偏移量
    struct Var_t *next; 
} Var_t;

typedef struct Register_ {
    char *name;     // 寄存器的名字
    Var_t *var;     // 应该是寄存器存储的变量的名字
} Register_;


extern Register_ regs[]; // 寄存器数组
extern char* regName[];  // 寄存器名字映射数组-目前有名字的就 r11-fp, r13-sp, r14-lr, r15-pc
extern Var_t *varList;   // 函数中的变量链表
extern int curReg;       //
extern int spOffset;     // 还不太确定-应该是相对fp的偏移-可能名字有问题
extern int curParam;     // 
extern int curArg;       // 

#endif

/*
栈管理
参数传入
前4个参数, 存入r0到r3寄存器中。
对于其他参数, 按顺序存入栈中, 在函数中, 利用fp拿出实参存入栈中作为临时变量。
*/