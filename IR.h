// simple intermediate code representation -- linear structure (learning from NJU, not the final result)
// we can design the intermediate code by hand and use hierarchical representation for optimization, hence better performance
#ifndef _IR_H_
#define _IR_H_

#include <assert.h>
#include <stdio.h>
typedef struct Operand_* Operand;
typedef struct InterCode_* InterCode;
typedef struct Label_No_* Label_No;

typedef struct Operand_ {
    // operand kind: temporary variable, variable, constant, virtual address, label, function?, true address?
    enum { TEMPVAR, VARIABLE, CONSTANT, VADDRESS, LABEL, FUNCTION_, TADDRESS } kind; 
    union {
        int var_no;   // variable number ?
        char* value;  // element width ?? for variable kind
        Operand addr; // address 
    } u;
    Operand next;
} Operand_;

typedef struct InterCode_{
    // intercode kind: assgin, add, etc...
	enum { ASSIGN_N,ADD_N,SUB_N,MUL_N,DIV_N,RETURN_N,LABEL_N,GOTO_N,IFGOTO_N,READ_N,WRITE_N,CALL_N,ARG_N,FUNCTION_N,PARAM_N,DEC_N,ADDRESS_N } kind;
    // operands for different operations
    union {
        struct { Operand op; } sinop; // single 
        struct { Operand left, right; } assign;  
        struct { Operand op1, op2, result; } binop; // binary 
        struct { Operand x; Operand y; Operand label; char *op; } triop; // triple
        struct { Operand op; int size; }dec;
    } u;
    InterCode pre;  // doubly linked list code是线性结构的-每个三地址码用链表按顺序连接？
    InterCode next;
} InterCode_;

// Label for three address code ? Mark the place for jump
typedef struct Label_No_
{
    int no;
    Label_No next;
} Label_No_;

void insertIRCode(InterCode c);    
void deleteIRCode(InterCode c);     
void printCode(char* fileName);

extern struct IRList_ IRList;


#endif