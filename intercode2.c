#include "intercode.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "node.h"
#include "symtable.h"
/* 
剩下的问题
3. break和continue语句如何与外面的while建立关系
*/
extern int scope;

/*
Exp -> AddExp. 
翻译Exp, 结果赋值给place, 返回计算三地址码链表 ok
*/
CodeList translate_Exp(treeNode *Exp, Operand place) {
    return translate_AddExp(Exp->child, place);  // 直接调用translate_AddExp
} 

/* 
AddExp -> MulExp | AddExp('+' | '-')MulExp. 
翻译AddExp, 结果赋值给place, 返回三地址码链表 ok
*/
CodeList translate_AddExp(treeNode *AddExp, Operand place)
{
    if(!strcmp(AddExp->child->name,"MulExp")) {
        return translate_MulExp(AddExp->child, place);  // 直接调用translate_MulExp, 
    }
    else {
        Operand t1 = new_temp();
        Operand t2 = new_temp();  // 两个临时变量
        treeNode* AddExp_1 = AddExp->child;   
        treeNode* MulExp = AddExp_1->next->next; // 两边表达式
        CodeList code1 = translate_AddExp(AddExp_1,t1);
        CodeList code2 = translate_MulExp(MulExp,t2); // 两边表达式翻译结果存在t1 和 t2中
        InterCode ic = malloc(sizeof(struct InterCode_)); // 三地址码节点
        if(!strcmp(AddExp_1->next->name, "PLUS")) {
            ic->kind = IR_PLUS;     // 加法类型
        }
        else { //"MINUS"
            ic->kind = IR_MINUS;    // 减法类型
        }
        ic->u.binop.op1 = t1;
        ic->u.binop.op2 = t2;       // 两个运算数
        ic->u.binop.result = place; // 结果存放位置 即place
        CodeList code3 = new_CodeList(ic);  // 三地址码链表
        return join(join(code1,code2),code3); // 返回连接后的三地址码链表
    }   
}

/*
UnaryExp | MulExp ('*' | '/' | '%') UnaryExp 
翻译MulExp, 结果赋值给place, 返回三地址码链表 ok
ok
*/
CodeList translate_MulExp(treeNode *MulExp, Operand place)
{
    if(!strcmp(MulExp->child->name,"UnaryExp")) {
        return translate_UnaryExp(MulExp->child,place);  // 直接调用UnaryExp
    }
    else {
        Operand t1 = new_temp();
        Operand t2 = new_temp();    // 两个临时变量
        treeNode* MulExp_1 = MulExp->child;
        treeNode* UnaryExp = MulExp_1->next->next;
        CodeList code1 = translate_MulExp(MulExp_1,t1);
        CodeList code2 = translate_UnaryExp(UnaryExp,t2); // 两边表达式
        InterCode ic = malloc(sizeof(struct InterCode_)); // 中间代码节点
        if(!strcmp(MulExp_1->next->name, "STAR")) {
            ic->kind = IR_MUL;  // 乘法类型
        }
        else if(!strcmp(MulExp_1->next->name, "DIV")) {
            ic->kind = IR_DIV;  // 除法类型
        }
        else if(!strcmp(MulExp_1->next->name, "PERCENT")) {
            ic->kind = IR_MOD;  // 模除类型
        }
        ic->u.binop.op1 = t1;   
        ic->u.binop.op2 = t2;   // 两个运算数
        ic->u.binop.result = place; // 存储的目标
        CodeList code3 = new_CodeList(ic); // 三地址码链表
        return join(join(code1,code2),code3); // 返回合并后的三地址码链表
    }      
}


/*
Number -> IntConst
计算Number的值, 结果赋给place, 返回三地址码链表 应该ok
*/
CodeList translate_Number(treeNode *Number, Operand place)
{
    int val = Number->child->int_val;    // 应该是直接用这个int_val吧
    fprintf(stderr, "found int val: %d\n", val);
    InterCode ic = new_InterCode(IR_ASSIGN); // 赋值类型
    ic->u.assign.left = place;      // 目标是place
    ic->u.assign.right = new_constant(val); // 值就是一个const值吧. const值不用地方保存?
    return new_CodeList(ic);        // 返回三地址码链表
}


/*
PrimaryExp -> LP Exp RP | LVal | Number
计算PrimaryExp的值, 结果赋给place, 返回三地址码链表 应该ok
*/
CodeList translate_PrimaryExp(treeNode *PrimaryExp,Operand place) {     
    if(!strcmp(PrimaryExp->child->name, "LP")) {
        return translate_Exp(PrimaryExp->child->next,place);  // 应该是直接返回即可--优先级是否要处理？
    }
    else if(!strcmp(PrimaryExp->child->name, "LVal")) { //LVal作右值，传个指针过去，再把指针指向的内容赋给place
        return translate_LVal(PrimaryExp->child, place, 0);      // 左值解析-有数组-重要
    }
    else {  // !strcpy(PrimaryExp->child->name,"Number")
        return translate_Number(PrimaryExp->child,place);
    }   
}


/*
UnaryExp -> PrimaryExp | ID LP FuncRParams RP | ID LP RP | UnaryOp UnaryExp
计算UnaryExp的值, 结果赋给place, 返回三地址码链表 ok
*/
CodeList translate_UnaryExp(treeNode *UnaryExp, Operand place) {
    if(!strcmp(UnaryExp->child->name,"ID")) {     
        printf("Function call found: %s\n", UnaryExp->child->value);    
        // 函数调用 UnaryExp -> ID LP FuncRParams RP
        if(!strcmp(UnaryExp->child->next->next->name, "FuncRParams")) { // 有实参
            ArgList argList = NULL;    // 实参链表
            CodeList code1 = translate_FuncRParams(UnaryExp->child->next->next,&argList); // 获取实参链表
            printf("Parse Rparam done\n");
            //无READ WRITE函数
            CodeList code2 = NULL;
            while(argList != NULL)  // 遍历实参链表-获取实参
            {
                InterCode ic = new_InterCode(IR_ARG);   // 实参指令的中间代码
                ic->u.op = argList->args;
                code2 = join(code2,new_CodeList(ic));   // 中间代码链表
                argList = argList->next;
            }
            InterCode ic = new_InterCode(IR_CALL);      // 函数调用中间代码 例如t3 := CALL add
            if(place != NULL) {
                ic->u.call.result = place;              // 函数调用结果给place
            }
            else {
                ic->u.call.result = new_temp();         // 一些NULL的情况还需要多检查一下 
            }
            ic->u.call.func = UnaryExp->child->value;   // value应该是他真实的函数名 直接用即可 语法树应该一直都在
            CodeList code3 = new_CodeList(ic);          
            return join(join(code1,code2),code3);
        } // ok
        else {
            // 函数调用 UnaryExp -> ID LP RP
            InterCode ic = new_InterCode(IR_CALL);
            if(place != NULL) {
                ic->u.call.result = place;  // 直接给返回值即可
            }
            else {
                ic->u.call.result = new_temp();
            }
            ic->u.call.func = UnaryExp->child->value;
            return new_CodeList(ic);
        }
    } // ok

    // UnaryExp -> UnaryOp UnaryExp 在翻译UnaryOp的情况
    if(!strcmp(UnaryExp->child->name,"UnaryOp")) {
        // 直接在这里处理掉UnaryOp
        if(!strcmp(UnaryExp->child->child->name,"PLUS")) {
            // 取正号
            return translate_UnaryExp(UnaryExp->child->next,place);  // 直接往下翻译即可
        }
        else if(!strcmp(UnaryExp->child->child->name,"MINUS")) {
            // 取负号 用0做减法q
            Operand t1 = new_temp();
            CodeList code1 = translate_UnaryExp(UnaryExp->child->next,t1); // 后面翻译结果赋值给t1
            InterCode ic = new_InterCode(IR_MINUS); // 减法
            ic->u.binop.op1 = new_constant(0);
            ic->u.binop.op2 = t1;   
            ic->u.binop.result = place; 
            CodeList code2 = new_CodeList(ic);
            return join(code1,code2);
        }
        else if(!strcmp(UnaryExp->child->child->name,"NOT")) {
            // 这个NOT虽然用在条件表达式中-但在文法里面其实和其他的逻辑运算都是分开的-只和表达式相关
            /// 表达式解析not
            //Operand label1 = new_label(); // label_true
            Operand label2 = new_label(); // label_false 
            InterCode ic = new_InterCode(IR_ASSIGN);
            ic->u.assign.left = place;
            ic->u.assign.right = new_constant(0); // 赋值0 
            CodeList code0 = new_CodeList(ic);
            
            // 条件解析NOT UnaryExp
            Operand t1 = new_temp();
            CodeList code11 = translate_UnaryExp(UnaryExp->child->next,t1);  // 计算求值
            InterCode ic1 = new_InterCode(IR_IFGOTO);   
            ic1->u.if_goto.x = t1;
            ic1->u.if_goto.y = new_constant(0);
            ic1->u.if_goto.z = label2;  // label_true -not-> label_false
            ic1->u.if_goto.relop = malloc(3);
            strcpy(ic1->u.if_goto.relop, "!=");  
            CodeList code12 = new_CodeList(ic1);
            //CodeList gotofalse = translate_Operand(label1, IR_GOTO);  // gotofalse -not-> goto true
            CodeList code1 = join(code11,code12);     
            // 跳转赋值相关
            //CodeList code2 = translate_Operand(label1,IR_LABEL);
            ic = new_InterCode(IR_ASSIGN);
            ic->u.assign.left = place;
            ic->u.assign.right = new_constant(1); // 赋值1
            //code2 = join(code2, new_CodeList(ic));
            CodeList code3 = translate_Operand(label2,IR_LABEL);
            return join(join(code0,code1),join(new_CodeList(ic),code3));     
        }
    }
    else if(!strcmp(UnaryExp->child->name,"PrimaryExp")) {
        //直接返回就行了把 UnaryExp -> PrimaryExp 
        return translate_PrimaryExp(UnaryExp->child,place);
    }
}

/*
Stmt -> LVal '=' Exp ';' | [Exp] ';' | Block
| 'if' '( Cond ')' Stmt [ 'else' Stmt ]
| 'while' '(' Cond ')' Stmt
| 'break' ';' | 'continue' ';'
| 'return' [Exp] ';'
Stmt语句的翻译  缺少break和continue语句 缺少多维数组处理(一维已经完成-重复一下即可)
*/
CodeList translate_Stmt(treeNode* Stmt, Operand label_1, Operand label_3)
{
    if(Stmt == NULL) 
        return NULL; 
    if(!strcmp(Stmt->child->name,"WHILE")) { 
        Operand label1 = new_label();
        Operand label2 = new_label();
        Operand label3 = new_label();
        CodeList code1 = translate_Cond(Stmt->child->next->next,label2,label3); // 应该没问题-直接用translate_Cond即可
        CodeList code2 = translate_Stmt(Stmt->child->next->next->next->next, label1, label3); //Stmt
        CodeList goto1 = translate_Operand(label1, IR_GOTO);  
        CodeList tmp = join(translate_Operand(label1, IR_LABEL),code1); 
        tmp = join(tmp,translate_Operand(label2,IR_LABEL));
        tmp = join(tmp,code2);
        tmp = join(tmp,goto1);
        tmp = join(tmp,translate_Operand(label3,IR_LABEL));
        return tmp;
        // ok
    }
    else if (!strcmp(Stmt->child->name, "BREAK")) {
        return translate_Operand(label_3, IR_GOTO);
    }
    else if (!strcmp(Stmt->child->name, "CONTINUE")) {
        return translate_Operand(label_1, IR_GOTO);
    }
    else if (!strcmp(Stmt->child->name, "RETURN")) {
        // 两种 一个是return Exp; 一个是直接返回 return ;
        if (!strcmp(Stmt->child->next->name, "Exp")) {
            Operand t1 = new_temp();
            CodeList code1 = translate_Exp(Stmt->child->next, t1);
            CodeList code2 = translate_Operand(t1, IR_RETURN);
            return join(code1, code2);
        }
        else {
            Operand t1 = new_constant(0);
            CodeList code = translate_Operand(t1, IR_RETURN);
            return code; // RETURN #0 代表void
        } 
    }
    else if(!strcmp(Stmt->child->name, "IF")) {
        if(Stmt->child->next->next->next->next->next == NULL) { // IF
            Operand label1 = new_label();
            Operand label2 = new_label();
            CodeList code1 = translate_Cond(Stmt->child->next->next,label1,label2);
            CodeList code2 = translate_Stmt(Stmt->child->next->next->next->next, label_1, label_3);
            CodeList clabel1 = translate_Operand(label1,IR_LABEL);
            CodeList clabel2 = translate_Operand(label2,IR_LABEL);
            return join(join(join(code1,clabel1),code2),clabel2);
            //  a > b 
            //  [IF a > b GOTO label1] [GOTO label2]  // translate_Cond
            //  [LABEL label1] [stmt] [LABEL label2]    // 后面应该正确-取决于translate_Cond的实现
            //  a > b && c > d
            //  [IF a > b GOTO label3] [GOTO label2]
            //  LABEL label3
            //  [IF c > d GOTO label1] [GOTO label2]
            //  [LABEL label1] [stmt] [LABEL label2]  // 应该没问题  
        } // ok
        else { // IF ELSE
            Operand label1 = new_label();
            Operand label2 = new_label();
            Operand label3 = new_label();
            CodeList code1 = translate_Cond(Stmt->child->next->next,label1,label2);
            CodeList code2 = translate_Stmt(Stmt->child->next->next->next->next, label_1, label_3);
            CodeList code3 = translate_Stmt(Stmt->child->next->next->next->next->next->next, label_1, label_3);
            CodeList clabel1 = translate_Operand(label1,IR_LABEL);
            CodeList clabel2 = translate_Operand(label2,IR_LABEL);
            CodeList clabel3 = translate_Operand(label3,IR_LABEL);
            CodeList goto3 = translate_Operand(label3,IR_GOTO);
            CodeList tmp = join(code1,translate_Operand(label1,IR_LABEL));
            tmp = join(tmp,code2);
            tmp = join(tmp,goto3);
            tmp = join(tmp,translate_Operand(label2,IR_LABEL));
            tmp = join(tmp,code3);
            tmp = join(tmp,translate_Operand(label3,IR_LABEL));
            return tmp;
        } // ok
    }
    else if(!strcmp(Stmt->child->name, "Block")) {
        return translate_Block(Stmt->child, label_1, label_3, 1);
    }
    else if(!strcmp(Stmt->child->name,"Exp")) {
        // Exp
        // exp的话应该要新建一个变量暂存一下的
        Operand t1 = new_temp();  // 应该是new temp
        return translate_Exp(Stmt->child,t1); //还是要改的
    } 
    else if(!strcmp(Stmt->child->name, "LVal")) {
        // Stmt -> LVal ASSIGN EXP; LVal -> ID Exp_rep; Exp_rep -> Exp_rep LB Exp RB | empty
        if(Stmt->child->child->next->child == NULL) {
            // 没有数组-单纯的变量 第一个Exp_rep的孩子是NULL-根据yacc上语法树构造-说明这个就是只有ID
            // Stmt -> LVal ASSIGN EXP; LVal -> ID Exp_rep; Exp_rep -> Exp_rep LB Exp RB | empty
            Operand var = op_from_var(Stmt->child->child->value); // 根据ID获取的变量
            Operand t1 = new_temp();
            CodeList code1 = translate_Exp(Stmt->child->next->next,t1); // exp计算结果
            InterCode ic = new_InterCode(IR_ASSIGN); // 变量赋值
            ic->u.assign.left = var;
            ic->u.assign.right = t1;
            CodeList code2 = new_CodeList(ic);
            return join(code1,code2);  // sysY语言的好像没有文法能够连续给表达式赋值, 例如 a = b = 2; 那就不用解析了
        }
        else {
            //计算LVal 获得地址
            Operand t1 = new_temp();
            CodeList code3 = translate_LVal(Stmt->child, t1, 1);
            // 计算表达式的值
            Operand t3 = new_temp();
            CodeList code4 = translate_Exp(Stmt->child->next->next,t3); //Exp计算
            InterCode ic = new_InterCode(IR_CHANGE_ADDR);  // *x := y 
            ic->u.assign.left = t1; 
            ic->u.assign.right = t3;
            CodeList code5 = new_CodeList(ic);  // 这里的赋值语句没有连续赋值的情况-就不返回了
            return join(join(code3, code4),code5);  
        } 
    }
    else //semi
        return NULL;
}

/* 
Block : LC BlockItem_rep RC
直接往下翻译即可 大致ok
*/
CodeList translate_Block(treeNode *Block, Operand label_1, Operand label_3, int isFuncBlock) {
    if(Block == NULL)
        return NULL;
    if(isFuncBlock) addScope();
    CodeList tmp = translate_BlockItem_rep(Block->child->next, label_1, label_3);
    if(isFuncBlock) exitScope();
    return tmp;
}

/* 
BlockItem_rep -> BlockItem_rep BlockItem  | empty ok
*/
CodeList translate_BlockItem_rep(treeNode *BlockItem_rep, Operand label_1, Operand label_3) {
    if(BlockItem_rep == NULL || BlockItem_rep->child == NULL)
        return NULL; 
    CodeList code1 = translate_BlockItem_rep(BlockItem_rep->child, label_1, label_3);
    CodeList code2 = translate_BlockItem(BlockItem_rep->child->next, label_1, label_3);
    return join(code1,code2);   // 先这样把-到时候再改
}   
/* 
BlockItem -> Decl | Stmt ok
*/
CodeList translate_BlockItem(treeNode *BlockItem, Operand label_1, Operand label_3) {
    if(!strcmp(BlockItem->child->name,"Decl")) {
        return translate_Decl(BlockItem->child);
    }
    else {
        // Stmt
        return translate_Stmt(BlockItem->child, label_1, label_3);
    }
}

/* 
LVal : ID Exp_rep 
Exp_rep : Exp_rep LB Exp RB | empty
*/
CodeList translate_Exp_rep(treeNode *Exp_rep, Operand place, int* index)
{
    if(Exp_rep == NULL || Exp_rep->child == NULL) return NULL;
    CodeList code0 = translate_Exp_rep(Exp_rep->child, place, index);
    Operand t1 = new_temp(), t2 = new_temp();
    CodeList code1 = translate_Exp(Exp_rep->child->next->next, t1);
    //place[*index] = t1
    InterCode ic = new_InterCode(IR_PLUS);
    ic->u.binop.op1 = place;
    ic->u.binop.op2 = new_constant(*index);
    ic->u.binop.result = t2;
    CodeList code2 = new_CodeList(ic);
    ++(*index);
    ic = new_InterCode(IR_CHANGE_ADDR);
    ic->u.assign.left = t2;
    ic->u.assign.right = t1;
    code2 = join(code2, new_CodeList(ic));
    return join(code0, join(code1, code2));
}


/* 
LVal : ID Exp_rep 
Exp_rep : Exp_rep LB Exp RB | empty
！！！注意：LVal既可以作为左值出现也可以作为右值出现，所以这里对于数组返回地址，此时要求下面的place参数为OP_ADDRESS类型
*/
CodeList translate_LVal(treeNode *LVal, Operand place, int isLeft) {
    // 普通变量的情况
    if(LVal->child->next->child == NULL) {
        Operand val = op_from_var(LVal->child->value);
        if(val->kind == OP_CONSTANT || val->kind == OP_VARIABLE)
        {
            InterCode ic = new_InterCode(IR_ASSIGN);
            ic->u.assign.left = place;
            ic->u.assign.right = val;
            return new_CodeList(ic);
        }
    } 
    // 特别注意：单独一个Ident也可能是指针
    // 数组/指针的情况, 以下注释中为方便理解，原数组记为a
    // 首先获取数组的地址，储存在baseAddr中, --> baseAddr = a
    Para item = querySymTable_para(LVal->child->value);
    Operand baseAddr = item->op;

    //预处理数组维数信息
    int arrayDim = listsize(item->type);
    printf("arrayDim: %d\n",arrayDim);
    int* arrayDimList = (int*)malloc(sizeof(int)*arrayDim);
    getArrayDimList(arrayDimList, item);
    printf("arrayDimList: {");
    int i;for(i=0;i<arrayDim;i++)printf("%d,",arrayDimList[i]);
    printf("}\n");

    //获取需要的数组下标，储存在数组中
    Operand b_addr = new_var();
    b_addr->kind = OP_ADDRESS;
    InterCode ic = new_InterCode(IR_DEC); // 数组空间声明语句
    ic->u.dec.x = b_addr;
    ic->u.dec.size =  arrayDim*sizeof(int); // 申明一个数组，大小为原数组的维度数，用于存放访问的index，以下记为b
    CodeList code0 = new_CodeList(ic);
    int accessDim = 0;  //实际访问的维数总数，比如一个二维数组a[2][2]，只访问a[1]，accessDim就为1
    code0 = join(code0, translate_Exp_rep(LVal->child->next, b_addr, &accessDim)); //获取index，存放在var中
    printf("accessDim=%d\n",accessDim);
    //计算最终地址
    Operand t1 = new_temp();
    Operand sum = new_temp(); //用于储存最终地址
    //set sum to 0
    ic=new_InterCode(IR_ASSIGN);
    ic->u.assign.left = sum;
    ic->u.assign.right = new_constant(0);
    code0 = join(code0, new_CodeList(ic));

    for(i=0;i<accessDim;i++)
    {
        //t1=b+4*i(即&b[i]);
        ic = new_InterCode(IR_PLUS);
        ic->u.binop.op1 = new_constant(4*i);
        ic->u.binop.op2 = b_addr;
        ic->u.binop.result = t1;
        code0=join(code0, new_CodeList(ic));
        //t1=*t1 此时t1=b[i]
        ic = new_InterCode(IR_RET);
        ic->u.assign.left = t1;
        ic->u.assign.right = t1;
        code0=join(code0, new_CodeList(ic));
        
        //rightSize是剩下维的大小之积，比如a[5][6][7], 当前i=0, 那么当前维的size是5，5右侧维数的大小之积为6*7=42, 所以rightSize=42
        int rightSize = getArraySize(arrayDimList, i+1, arrayDim); 
        //rightSize*=b[i]
        ic = new_InterCode(IR_MUL);
        ic->u.binop.op1=t1;
        ic->u.binop.op2=new_constant(rightSize);
        ic->u.binop.result=t1;
        code0=join(code0, new_CodeList(ic));
        //sum+=rightSize
        ic = new_InterCode(IR_PLUS);
        ic->u.binop.op1=sum;
        ic->u.binop.op2=t1;
        ic->u.binop.result=sum;
        code0=join(code0, new_CodeList(ic));
    }

    //place = a+sum*4 （即&a[sum]）
    ic=new_InterCode(IR_MUL);
    ic->u.binop.op1=sum;
    ic->u.binop.op2=new_constant(4);
    ic->u.binop.result=sum;
    code0=join(code0, new_CodeList(ic));
    ic=new_InterCode(IR_PLUS);
    ic->u.binop.op1=baseAddr;
    ic->u.binop.op2=sum;
    ic->u.binop.result=place;
    code0=join(code0, new_CodeList(ic));

    if(!isLeft && accessDim == arrayDim) //右值的话取一下指针中的值
    {
        ic=new_InterCode(IR_RET);
        ic->u.assign.left=place;
        ic->u.assign.right=place;
        code0=join(code0, new_CodeList(ic));
    } 
    free(arrayDimList);
    return code0;
}



/*
FuncDef : FuncType ID LP FuncFParams RP Block | FuncType ID LP RP Block	
FuncFParams : FuncFParam ComFuncFParam_rep
ComFuncFParam_rep : ComFuncFParam_rep COMMA FuncFParam
FuncFParam : FuncType ID | FuncType ID LB RB LbEXPRb_rep
LbEXPRb_rep : LbEXPRb_rep LB Exp RB
函数定义语句的翻译. 总体还是没问题, 但还是有部分要修改 基本ok-还需要再修改
*/
CodeList translate_FuncDef(treeNode *FuncDef)
{
    if(FuncDef == NULL) {
        return NULL;
    }
    InterCode ic = new_InterCode(IR_FUNC); // 函数中间名中间代码
    ic->u.func = FuncDef->child->next->value;    // 函数名 ID的value就是他的值
    printf("Funcdef: %s\n", ic->u.func);
    CodeList code1 = new_CodeList(ic);     // 函数名中间代码链表
    CodeList code2 = NULL;                 // 存block部分的中间代码
    addScope();
    if(!strcmp(FuncDef->child->next->next->next->name,"FuncFParams")) { // 解析形参
        // FuncType ID LP FuncFParams RP Block
        // 我们这里应该就只有int类型
        Func func = querySymTable_func(FuncDef->child->next->value);    // 根据函数名获得对应但函数
        vector paraList = func->paraList;    // 从而获得参数链表
        int i;
        for(i=0; i<paraList->size; i++)
        {
            Para param = (Para)getItem(paraList, i);
            InterCode paramCode = new_InterCode(IR_PARAM);
            Operand op;
            Type para_type = (Type)getFirst(param->type);
            op=new_var();
            paramCode->u.op = op;
            param->op = op; //存入符号表
            printf("name: %s\n",param->name);
            printf("scope: %d\n", param->scope);
            printf("current scope: %d\n", scope);
            insertSymTable_para(param, 0);
            CodeList tmp = new_CodeList(paramCode);
            join(code1,tmp);
        }

        // Block
        code2 =  translate_Block(FuncDef->child->next->next->next->next->next, NULL, NULL, 0); 
    }
    else {
        // FuncDef -> FuncType ID LP RP Block
        // Nothing

        // Block
        code2 =  translate_Block(FuncDef->child->next->next->next->next, NULL, NULL, 0); 
    }
    exitScope();
    return join(code1,code2); 
}


/*
FuncRParams : Exp comExp_rep; 
comExp_rep : comExp_rep COMMA Exp | empty;
实参的翻译, 添加到实参表, 并返回计算过程的三地址码链表 基本ok
*/
CodeList translate_FuncRParams(treeNode *FuncRParams, ArgList *arg_list) {
    if(FuncRParams==NULL) {
        return NULL;
    }
    Operand t1 = new_temp();   // 临时变量
    CodeList code1 = translate_Exp(FuncRParams->child,t1); // Exp求值后存到临时变量汇总
    ArgList newArgList = malloc(sizeof(struct ArgList_));  // 这里的ArgList就是链表的一个结点-代表了一个内容为arg的结构-而不是链表-注意和InterCode与CodeList关系区分
    newArgList->args = t1;          // 表达式计算的结果即为实参 
    newArgList->next = *arg_list;   // 链接
    *arg_list = newArgList;         // 头部!
    CodeList code2 = translate_comExp_rep(FuncRParams->child->next,arg_list); // 继续翻译下面的实参, 注意这里的arg_list为指针
    return join(code1,code2);
}
/* 
comExp_rep : comExp_rep COMMA Exp | empty
跟上 基本ok
*/
CodeList translate_comExp_rep(treeNode *comExp_rep, ArgList *arg_list) {
    if(comExp_rep == NULL || comExp_rep -> child == NULL) {
        return NULL;
    }
    CodeList code1 = translate_comExp_rep(comExp_rep->child,arg_list); // 继续递归翻译-直到遇到NULL. //目前还不确定NULL的地址码解析-应该ok
    Operand t1 = new_temp();      // 临时变量暂存表达式运算结果
    CodeList code2 = translate_Exp(comExp_rep->child->next->next,t1);  // 获取表达式运算结果
    ArgList newArgList = malloc(sizeof(struct ArgList_));              // 分配地址空间
    newArgList->args = t1;
    newArgList->next = *arg_list;
    *arg_list = newArgList;         // 头部!
    return join(code1,code2);       // 
}



/*
Utility. 根据类型新建一个三地址码节点, 返回三地址码节点 ok
*/
InterCode new_InterCode(int kind) 
{
    InterCode ic = (InterCode)malloc(sizeof(struct InterCode_));
    ic->kind = kind; 
    return ic;
}

/*
Utility. 从一个三地址码节点建立一个三地址码链表 ok
*/
CodeList new_CodeList(InterCode code)
{
    CodeList cl = (CodeList)malloc(sizeof(struct CodeList_));
    cl->code = code; 
    cl->next = cl->prev = NULL;  // 双向链表
    return cl;
}

/*
Utility. 链接两个三地址码链表 ok
*/
CodeList join(CodeList head, CodeList body) {
    if(head == NULL) {
        return body;
    }
    CodeList tmp = head;
    while(tmp->next != NULL) {  // 链表结尾是NULL
        tmp = tmp->next;
    }
    tmp->next = body;
    if(body != NULL) {
        body->prev = tmp;
    }
    return head;
}

/*
Utility. 将*单运算符*根据类型转化为三地址码链表
例如: translate_Operand(tmp, IR_RETURN). translate_Operand(label_false, IR_LABEL).
转化为三地址码(形式化结果) RETURN t3. LABEL label1 :. ok
*/
CodeList translate_Operand(Operand op, int IR_KIND) {
    InterCode tmp = new_InterCode(IR_KIND);
    tmp->u.op = op;
    CodeList res = new_CodeList(tmp);
    return res;
}


/*
Utility. 将三地址码插入全局链表. 全局变量: code_tail, code_head ok
*/
void insert_code(CodeList code) {
    if(code == NULL) {
        return ;
    }
    if(code_head==NULL) { //如果为NULL
        code_head = code;
        CodeList tmp = code;
        while(tmp->next != NULL)
        {
            tmp=tmp->next;
        }
        code_tail = tmp;
    }
    else { //往尾部插入
        code->prev = code_tail;
        code_tail->next = code;
        while(code_tail->next != NULL)
        {
            code_tail = code_tail->next;
        }
    }
}


/* 
Utility. 返回常量操作数. // 目前来看-应该叫字面量(立即数)操作数?
也有用在int的值中, 例如 ic->u.assign.right = new_constant(val); 应该ok
*/
Operand new_constant(int val) {
    Operand tmp = malloc(sizeof(struct Operand_));
    tmp->kind = OP_CONSTANT;
    tmp->u.val = val;
    return tmp;
}

/*
Utility. 返回临时变量操作数(全局唯一, count下标). ok
*/
Operand new_temp() {
    Operand tmp = malloc(sizeof(struct Operand_));
    tmp->kind = OP_TEMP;
    tmp->u.temp_no = temp_num;
    temp_num++;
    return tmp;
}

/*
Utility. 返回标签操作数(全局唯一, 代表一个标签). ok
*/
Operand new_label() {
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_LABEL;
    tmp->u.temp_no = label_num;
    label_num++;
    return tmp;
}

/*
Utility. 返回变量. ok
*/
Operand new_var() {
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_VARIABLE;
    tmp->u.var_no = var_num++;
    return tmp;
}

/*
Utility. 返回函数形参. ok
*/
Operand new_fparam() {
    Operand tmp = (Operand)malloc(sizeof(struct Operand_));
    tmp->kind = OP_VARIABLE;
    //tmp->u.a 需要事后自己修改
    return tmp;
}

/*
Utility. 返回变量类型的Operand. 变量: 函数形参, 局部变量v开头. 
查看变量链表-返回对应的操作数. var是变量名-可以用原本代码中的变量名?可能需要解决名字冲突问题?
大致ok--还需要解决一些问题
*/
Operand op_from_var(char* var) {
    // if(var_head == NULL) {  // 变量链表为空
    //     // 若为空 直接加
    //     var_head = malloc(sizeof(struct Variable_));
    //     var_head->name = var;       // 变量名
    //     Operand tmp = malloc(sizeof(struct Operand_));
    //     tmp->kind = OP_VARIABLE;
    //     tmp->u.var_no = var_num;    // 变量操作数
    //     var_num++;
    //     var_head->op = tmp;
    //     var_tail = var_head;
    //     return tmp;
    // }
    // else {
    //     // 遍历 查找 进行名字比较
    //     Variable vtmp = var_head;
    //     while(vtmp != NULL) {
    //         if(!(strcmp(vtmp->name,var))) {
    //             return vtmp->op;    // 找到直接返回即可
    //         }
    //         vtmp = vtmp->next;
    //     } 
    //     // 增加变量节点内容并返回
    //     Operand tmp = malloc(sizeof(struct Operand_));
    //     tmp->kind = OP_VARIABLE;
    //     tmp->u.var_no = var_num;
    //     var_num++;
    //     Variable newVar = malloc(sizeof(struct Variable_));
    //     newVar->name = var;
    //     newVar->op = tmp;
    //     var_tail->next = newVar;
    //     var_tail = newVar;
    //     return tmp;
    // }
    Para tmp = querySymTable_para(var);
    if(tmp&&tmp->op!=NULL) return tmp->op; //已经在符号表中，直接返回
    return new_var();
}

/*
Utility. 运算符转化为字符串. 返回转化后的字符串 大致ok-要理解修改
*/
char *Operand_toString(Operand op) {
    char msg[64] = "";
    if(op->kind == OP_CONSTANT) {
        sprintf(msg,"#%d",op->u.val); // 常量(目前应该叫立即数). 例如 #0
    }
    else if(op->kind == OP_LABEL) {
        sprintf(msg,"label%d",op->u.label_no); // 标签. 例如 label1. 全局唯一
    }
    else if(op->kind == OP_VARIABLE || op->kind == OP_ARR_STRU || op->kind == OP_ADDRESS) {
        sprintf(msg, "v%d", op->u.var_no);
    }
    else if(op->kind==OP_TEMP) {
        sprintf(msg,"t%d", op->u.temp_no);
    }
    char *ret = malloc(strlen(msg)+1);         // 返回空间指针
    strcpy(ret, msg);
    return ret; ///就是最终的内容把～～
}

/*
Utility. 将一个三地址码节点翻译为一条三地址码指令. 返回三地址码指令字符串 基本ok 还需要再了解下数组 地址 指针相关
*/
char* InterCode_toString(InterCode code) {
    int max_size = 64;              // 最大字节数
    char *msg = malloc(max_size);   // 空间分配 64 字节
    memset(msg,0,max_size);
    printf("%d\n", code->kind);
    if(code->kind == IR_LABEL) {    // LABEL代码
        char *x = Operand_toString(code->u.op);
        sprintf(msg, "LABEL %s :",x); // 例如 LABEL label1
    }
    else if(code->kind == IR_FUNC) { // FUNCTION代码
        char *f = code->u.func;      // code->u.func应该是函数名. 在别的地方check一下
        sprintf(msg, "FUNCTION %s :", f); // 例如 FUNCTION main
    }
    // 这几部分都和地址指针数组有关--下次再看看
    else if(code->kind == IR_ASSIGN) {
        Operand left = code->u.assign.left;    // 左边的运算数
        Operand right = code->u.assign.right;  // 右边的运算数
        if(right==NULL){
            printf("omitted\n");
            sprintf(msg, "\n");
            return msg;
        }
        if(code->u.assign.left != NULL) {      
            char *x = Operand_toString(left);  // 例如v1
            char *y = Operand_toString(right); // 例如t1
            // if(left->kind==OP_ADDRESS && right->kind!=OP_ADDRESS) {
            //     sprintf(msg,"%s := &%s",x,y);  // 例如 v1 = &t1 产生来源: 大概是
            // }
            // else if(left->kind!=OP_ADDRESS && right->kind==OP_ADDRESS) {
            //     sprintf(msg,"%s := *%s",x,y);  // 例如 v1 = *t1 产生来源: 有数组赋值语句-也就是将数组某个下标的值给某个变量
            // }
            // else {
            //     sprintf(msg,"%s := %s",x,y);   // 普通赋值运算, 例如v1 = t1
            // } 
            sprintf(msg,"%s := %s",x,y);
        }
    }
    else if(code->kind == IR_CHANGE_ADDR) {    // 这几部分都和地址指针数组有关--下次再看看
        if(code->u.assign.left != NULL) {
            char *x = Operand_toString(code->u.assign.left);
            char *y = Operand_toString(code->u.assign.right);
            sprintf(msg, "*%s := %s",x,y);     // 例如 *v1 = t1 
        }
    }
    else if(code->kind == IR_PLUS) {
        if(code->u.binop.result != NULL) {
            char *x = Operand_toString(code->u.binop.result);
            char *y = Operand_toString(code->u.binop.op1);
            char *z = Operand_toString(code->u.binop.op2); 
            sprintf(msg, "%s := %s + %s",x,y,z); // 例如 t3 = t2 + t1
        }
    }
    else if(code->kind == IR_MINUS) {
        if(code->u.binop.result != NULL) {
            char *x = Operand_toString(code->u.binop.result);
            char *y = Operand_toString(code->u.binop.op1);
            char *z = Operand_toString(code->u.binop.op2);
            sprintf(msg, "%s := %s - %s",x,y,z); // 例如 t3 = t2 - t1
        }
    }
    else if(code->kind == IR_MUL) {
        if(code->u.binop.result != NULL) {
            char *x = Operand_toString(code->u.binop.result); 
            char *y = Operand_toString(code->u.binop.op1);
            char *z = Operand_toString(code->u.binop.op2);
            ////
            sprintf(msg,"%s := %s * %s",x,y,z); // 例如 t3 = t2 * t1
        }
    }
    else if(code->kind == IR_DIV) {
        if(code->u.binop.result != NULL) {
            char *x = Operand_toString(code->u.binop.result);
            char *y = Operand_toString(code->u.binop.op1);
            char *z = Operand_toString(code->u.binop.op2);
            sprintf(msg,"%s := %s / %s", x,y,z); // 例如 t3 = t2 / t1
        }
    }
    else if(code->kind == IR_MOD) {
        if(code->u.binop.result != NULL) {
            char *x = Operand_toString(code->u.binop.result);
            char *y = Operand_toString(code->u.binop.op1);
            char *z = Operand_toString(code->u.binop.op2);
            sprintf(msg,"%s := %s %% %s",x,y,z); // 例如 t3 = t2 % t1
        }
    }
    else if(code->kind == IR_GOTO) {
        char *x = Operand_toString(code->u.op);
        sprintf(msg,"GOTO %s",x);               // 例如 GOTO label4
    }
    else if(code->kind == IR_IFGOTO) {          // 例如 IF v2 > v1 GOTO label3
        char *x = Operand_toString(code->u.if_goto.x);
        char *y = Operand_toString(code->u.if_goto.y);
        char *z = Operand_toString(code->u.if_goto.z);
        sprintf(msg,"IF %s %s %s GOTO %s",x,code->u.if_goto.relop,y,z); // 应该可以直接用
    }
    else if(code->kind == IR_RETURN) {
        char *x = Operand_toString(code->u.op); // 例如RETURN t8
        sprintf(msg,"RETURN %s",x);
    }
    else if(code->kind == IR_RET) {
        Operand left = code->u.assign.left;    // 左边的运算数
        Operand right = code->u.assign.right;  // 右边的运算数
        if(code->u.assign.left != NULL) {      
            char *x = Operand_toString(left);  // 例如v1
            char *y = Operand_toString(right); // 例如t1
            sprintf(msg, "%s := *%s", x, y);
        }
    }
    else if(code->kind == IR_DEC) {
        char *x = Operand_toString(code->u.dec.x);
        sprintf(msg,"DEC %s %d",x,code->u.dec.size); // 申请内存空间--数组相关--下次看. 例如DEC v2 8
    }
    else if(code->kind == IR_ARG) {
        char *x = Operand_toString(code->u.op);
        sprintf(msg,"ARG %s",x);        // 例如ARG v2. // ARG &v2 会发生吗? 
    }
    else if(code->kind == IR_CALL) {
        char *x = Operand_toString(code->u.call.result);
        char *f = code->u.call.func;
        sprintf(msg,"%s := CALL %s",x,f); // 例如 x := CALL f
    }
    else if(code->kind == IR_PARAM) {
        char *x = Operand_toString(code->u.op);
        sprintf(msg,"PARAM %s",x);        //  例如 PARAM v1
    }
    // else if(code->kind == IR_READ) {   // 目前不用READ WRITE
    //     char *x = Operand_toString(code->u.op);
    //     sprintf(msg,"READ %s",x);
    // } 
    // else if(code->kind == IR_WRITE) {
    //     char *x = Operand_toString(code->u.op);
    //     sprintf(msg,"WRITE %s",x);
    // }
    else if(code->kind == IR_GET_ADDR) {
        char *x = Operand_toString(code->u.assign.left);
        char *y = Operand_toString(code->u.assign.right);
        sprintf(msg,"%s := &%s",x,y);   // 数组相关--下次修改在看
    }
    else if(code->kind == IR_ENDFUNC) {
        sprintf(msg, "ENDFUNC");
    }
    char *ret = malloc(strlen(msg) + 1);
    strcpy(ret, msg);
    free(msg);
    return ret;
}



/*
Start. 翻译中间代码, 并将结果写入文件 ok
*/
CodeList translate_InterCode(treeNode* root, char* file)
{
    FILE *f;
    if(file != NULL) {
        f = fopen(file, "w");
    }
    else {
        f = fopen("output.ir", "w");
    }
    label_num = var_num = temp_num = 1; // 各种类型运算符(标签、变量、临时变量)的 起始序号
    code_head = code_tail = NULL;       // 中间代码链表的头和尾
    var_tail = var_head = NULL;         // 变量链表的头和尾 //以上均为全局变量
    start_translate(root);              // 翻译
    printCodeList(code_head);
    printf("---------------\n");
    CodeList code = code_head;          // 中间代码头
    while(code != NULL)                 // 依次遍历-解析为字符串-输出
    {   
        fprintf(f,"%s\n",InterCode_toString(code->code));
        code = code->next;      
    }
    fclose(f);
    return code_head;
}


/*
Utility. 递归翻译, 得到中间代码链表 ok
*/
void start_translate(treeNode* root)
{   
    if(root==NULL){
        return ;
    }
    if(!strcmp(root->name, "CompUnit")) {    // 只有一个根的话-直接从根开始递归翻译就行了 应该
        fprintf(stderr,"start_translate\n");
        initScopeTable();
        insert_code(translate_CompUnit(root));
    }
}


/*
CompUnit : CompUnit Decl | CompUnit FuncDef | Decl | FuncDef  
程序编译单元 ok
*/
CodeList translate_CompUnit(treeNode *CompUnit) {
    if(!strcmp(CompUnit->child->name,"CompUnit")) {
        CodeList code1 = translate_CompUnit(CompUnit->child);
        CodeList code2 = NULL, code3 = NULL;
        if(!strcmp(CompUnit->child->next->name,"Decl")){
            code3 = translate_Decl(CompUnit->child->next);
        }
        else { // (!strcmp(CompUnit->child->name,"FuncDef"))
            code2 = translate_FuncDef(CompUnit->child->next);
            code3 = join(code2, new_CodeList(new_InterCode(IR_ENDFUNC)));
        }
        return join(code1,code3);
    }
    else if(!strcmp(CompUnit->child->name,"Decl")) {
        return translate_Decl(CompUnit->child);
    }
    else { // if(!strcmp(CompUnit->child->name,"FuncDef"))
        return join(translate_FuncDef(CompUnit->child), new_CodeList(new_InterCode(IR_ENDFUNC)));
    }
}


// 需要重度修改的: translate_Decl, translate_VarDecl, translate_VarDef, translate_ConstDef, translate_ConstInitVal, translate_InitVal
/*************************************************************/
/*Decl : ConstDecl | VarDecl 
*/
CodeList translate_Decl(treeNode *Decl) {
    if(Decl == NULL)
        return NULL; //应该不会
    if(!strcmp(Decl->child->name,"ConstDecl")) {
        CodeList code1 = translate_ConstDecl(Decl->child);
        return code1;
    }
    else if(!strcmp(Decl->child->name,"VarDecl")) {
        // 先不考虑高维数组
        // 变量单纯的定义没有用
        // 遍历翻译所有的VarDef
        CodeList code1 = translate_VarDecl(Decl->child);
        return code1;
    }
}

/*
    ConstDecl : CONST FuncType ConstDef ComConstDef_rep SEMI
    ComConstDef_rep	: ComConstDef_rep COMMA ConstDef | empty
*/
CodeList translate_ConstDecl(treeNode *ConstDecl) {
    fprintf(stderr, "%s\n", ConstDecl->name);
    if(ConstDecl == NULL)
        return NULL;
    if(!strcmp(ConstDecl->child->next->next->name, "ConstDef")) {
        CodeList code1 = translate_ConstDef(ConstDecl->child->next->next);
        CodeList code2 = translate_ComConstDef_rep(ConstDecl->child->next->next->next);
        return join(code1, code2);
    }
}

/*
    ComConstDef_rep	: ComConstDef_rep COMMA ConstDef | empty
*/
CodeList translate_ComConstDef_rep(treeNode *ComConstDef_rep) {
    if(ComConstDef_rep == NULL)
        return NULL;
    if(ComConstDef_rep->child == NULL) 
        return NULL;
    CodeList code1 = translate_ComConstDef_rep(ComConstDef_rep->child);
    CodeList code2 = translate_ConstDef(ComConstDef_rep->child->next->next);
    return join(code1, code2);
}


CodeList translate_ConstExp(treeNode *ConstExp, Operand place) {
    return translate_AddExp(ConstExp->child,place);
}

/* 
ConstDef : ID ConstExp_rep ASSIGNOP ConstInitVal
ConstExp_rep : ConstExp_rep LB ConstExp RB | empty 
*/
CodeList translate_ConstDef(treeNode *ConstDef) {
    if(ConstDef == NULL)
        return NULL;
    
    // 不能转化成变量, 因为转换成变量后符号表中没有存常量的初值
    Para item;
    NEW0(item);
    item->type = newList();
    item->scope = scope;
    strcpy(item->name, ConstDef->child->value);
    if(ConstDef->child->next->child == NULL) { // ConstExp_rep的孩子是NULL 说明是单变量
        Type newType;
        NEW0(newType);
        newType->kind = constant;
        addFirst(item->type, newType);
        sdtParseConstInitVal(ConstDef->child->next->next->next, &item);
        Operand op = new_var();
        item->op = op;
        // sdtParseConstInitVal(VarDef->child->next->next->next, &item);
        printf("scope=%d\n",scope);
        insertSymTable_para(item, 0);
        printf("constant/variable found: %s\n", item->name);
        printf("res = %d\n", para_exist(item->name, scope));
        return translate_InitVal(ConstDef->child->next->next->next, op, 0, NULL); // 赋值给op
    }
    else {  //处理const array和var array
        //首先解析出数组的维数，并插入符号表，这里和符号表不同，不需要插入初值
        printf("translate_VarDef: const/varArray found: %s\n", item->name);
        sdtParseConstExp_rep(ConstDef->child->next, &item->type);
        //printType(item->type);
        sdtParseConstInitVal(ConstDef->child->next->next->next, &item);
        Operand op = new_var();
        op->kind = OP_ADDRESS;
        item->op = op;
        insertSymTable_para(item, 0);

        printf("scope=%d\n",scope);
        //预处理数组维数信息
        int arrayDim = listsize(item->type);
        printf("arrayDim: %d\n",arrayDim);
        int* arrayDimList = (int*)malloc(sizeof(int)*arrayDim);
        getArrayDimList(arrayDimList, item);
        printf("arrayDimList: {");
        int i;for(i=0;i<arrayDim;i++)printf("%d,",arrayDimList[i]);
        printf("}\n");
        int arraySize = getArraySize(arrayDimList, 0, arrayDim);
        printf("arraySize: %d\n", arraySize);

        //生成数组定义代码
        op->kind = OP_ADDRESS;
        InterCode ic = new_InterCode(IR_DEC); // 数组空间声明语句
        ic->u.dec.x = op;
        ic->u.dec.size =  arraySize*sizeof(int); // 数组空间大小 translate_ConstExp 反正要获得空间大小!!要修改
        CodeList code0 = new_CodeList(ic);
        CodeList code1 = translate_InitVal(ConstDef->child->next->next->next, op, arrayDim, arrayDimList); // 赋值给arr
        free(arrayDimList);
        return join(code0, code1);
    }  
}

/* 
ConstInitVal : ConstExp | LC ConstInitVal ComConstInitVal_rep RC | LC RC 
ComConstInitVal_rep : ComConstInitVal_rep COMMA ConstInitVal | empty
ConstExp : AddExp
*/
CodeList translate_ConstInitVal(treeNode *ConstInitVal, Operand place) {
    // 先不考虑数组
    if(!strcmp(ConstInitVal->child->name,"ConstExp")) {
        return translate_AddExp(ConstInitVal->child->child,place); // AddExp
    }
    else if(!strcmp(ConstInitVal->child->name,"LC") && !strcmp(ConstInitVal->child->next->name,"RC")) {
        return NULL; // 直接返回空吧
    }
    // 数组
}

/*
VarDecl : FuncType VarDef ComVarDef_rep SEMI
ComVarDef_rep : ComVarDef_rep COMMA VarDef | empty
ok
*/
CodeList translate_VarDecl(treeNode *VarDecl) {
    fprintf(stderr, "%s\n", VarDecl->name);
    // 遍历翻译所有的Vardef
    if (VarDecl == NULL) 
        return NULL;
    CodeList code1 = translate_VarDef(VarDecl->child->next);
    CodeList code2 = translate_ComVarDef_rep(VarDecl->child->next->next);
    return join(code1, code2);
}

/*
    ComVarDef_rep : ComVarDef_rep COMMA VarDef | empty
*/
CodeList translate_ComVarDef_rep(treeNode *ComVarDef_rep){
    if(ComVarDef_rep == NULL || ComVarDef_rep -> child == NULL)
        return NULL;
    CodeList code1 = translate_ComVarDef_rep(ComVarDef_rep->child);
    CodeList code2 = translate_VarDef(ComVarDef_rep->child->next->next);
    return join(code1, code2);
}


/*
ConstDef : ID ConstExp_rep ASSIGNOP ConstInitVal
VarDef : ID ConstExp_rep | ID ConstExp_rep ASSIGNOP InitVal
ConstExp_rep : ConstExp_rep LB ConstExp RB | empty
大致ok  补充上多维数组的情况
*/
CodeList translate_VarDef(treeNode *VarDef) {
    if(VarDef == NULL) 
        return NULL;
    // 变量的普通定义不用翻译 定义初始化要翻译出来 数组要翻译出来
    Para item;
    NEW0(item);
    item->type = newList();
    item->scope = scope;
    strcpy(item->name, VarDef->child->value);
    if(VarDef->child->next->child == NULL) { // ConstExp_rep的孩子是NULL 说明是单变量
        Type newType;
        NEW0(newType);
        newType->kind = basic;
        addFirst(item->type, newType);
        Operand op = new_var();
        item->op = op;
        // sdtParseConstInitVal(VarDef->child->next->next->next, &item);
        printf("scope=%d\n",scope);
        insertSymTable_para(item, 0);
        printf("constant/variable found: %s\n", item->name);
        printf("res = %d\n", para_exist(item->name, scope));
        if(VarDef->child->next->next != NULL) { // ASSIGN 存在
            return translate_InitVal(VarDef->child->next->next->next, op, 0, NULL); // 赋值给op    
        }
        else 
            return NULL;  //纯变量定义无需生成中间代码
    }
    else {  //处理const array和var array
        //首先解析出数组的维数，并插入符号表，这里和符号表不同，不需要插入初值
        printf("translate_VarDef: const/varArray found: %s\n", item->name);
        sdtParseConstExp_rep(VarDef->child->next, &item->type);
        printType(item->type);
        Operand op = new_var();
        op->kind = OP_ADDRESS;
        item->op = op;
        insertSymTable_para(item, 0);

        printf("scope=%d\n",scope);
        //预处理数组维数信息
        int arrayDim = listsize(item->type);
        printf("arrayDim: %d\n",arrayDim);
        int* arrayDimList = (int*)malloc(sizeof(int)*arrayDim);
        getArrayDimList(arrayDimList, item);
        printf("arrayDimList: {");
        int i;for(i=0;i<arrayDim;i++)printf("%d,",arrayDimList[i]);
        printf("}\n");
        int arraySize = getArraySize(arrayDimList, 0, arrayDim);
        printf("arraySize: %d\n", arraySize);

        //生成数组定义代码
        op->kind = OP_ADDRESS;
        InterCode ic = new_InterCode(IR_DEC); // 数组空间声明语句
        ic->u.dec.x = op;
        ic->u.dec.size =  arraySize*sizeof(int); // 数组空间大小 translate_ConstExp 反正要获得空间大小!!要修改
        CodeList code0 = new_CodeList(ic);

        if(VarDef->child->next->next != NULL) { // ASSIGN 存在
            CodeList code1 = translate_InitVal(VarDef->child->next->next->next, op, arrayDim, arrayDimList); // 赋值给arr
            free(arrayDimList);
            return join(code0, code1);
        }
        else
        {
            free(arrayDimList);
            return code0;
        }
    }  
}

/*
Utility. Print the code list. FOR DEBUG
*/
void printCodeList(CodeList tmp)
{
    if(tmp==NULL)
    {
        fprintf(stderr, "CodeList is NULL!\n");
        return;
    }
    while(tmp)
    {
        InterCode p = tmp->code;
        switch(p->kind)
        {
            case IR_ASSIGN: fprintf(stderr,"IR_ASSIGN\n");break;
            case IR_LABEL: fprintf(stderr,"IR_LABEL\n");break;
            case IR_PLUS: fprintf(stderr,"IR_PLUS\n");break;
            case IR_MINUS: fprintf(stderr,"IR_MINUS\n");break;
            case IR_MUL: fprintf(stderr,"IR_MUL\n");break;
            case IR_DIV: fprintf(stderr,"IR_DIV\n");break;
            case IR_MOD: fprintf(stderr,"IR_MOD\n");break;
            case IR_FUNC: fprintf(stderr,"IR_FUNC\n");break;
            case IR_GOTO: fprintf(stderr,"IR_GOTO\n");break;
            case IR_IFGOTO: fprintf(stderr,"IR_IFGOTO\n");break;
            case IR_RET: fprintf(stderr,"IR_RET\n");break;
            case IR_DEC: fprintf(stderr,"IR_DEC\n");break;
            case IR_ARG: fprintf(stderr,"IR_ARG\n");break;
            case IR_CALL: fprintf(stderr,"IR_CALL\n");break;
            case IR_PARAM: fprintf(stderr,"IR_PARAM\n");break;
            case IR_READ: fprintf(stderr,"IR_READ\n");break;
            case IR_WRITE: fprintf(stderr,"IR_WRITE\n");break;
            case IR_RETURN: fprintf(stderr,"IR_RETURN\n");break;
            case IR_GET_ADDR: fprintf(stderr,"IR_GET_ADDR\n");break;
            case IR_CHANGE_ADDR: fprintf(stderr,"IR_CHANGE_ADDR\n");break;
        }
        tmp=tmp->next;
    }
}

/*
InitVal : Exp | LC InitVal ComInitVal_rep RC | LC RC
ComInitVal_rep : ComInitVal_rep COMMA InitVal | empty
*/
CodeList translate_InitVal(treeNode *InitVal,Operand place, int arrayDim, int* arrayDimList) {
    // 数组赋值先不管
    if(!strcmp(InitVal->child->name,"Exp")||!strcmp(InitVal->child->name,"ConstExp")) {
        return translate_Exp(InitVal->child,place);
    }
    if(!strcmp(InitVal->child->name,"LC") && !strcmp(InitVal->child->next->name,"RC")) {
        // InitVal -> LC RC 不做事情
        return NULL; 
    }
    else {
        // InitVal : LC InitVal ComInitVal_rep RC
        CodeList code = NULL;
        int numCount = 0;
        translate_InitVal_array(InitVal->child, place, arrayDim, arrayDimList, 0, &numCount, getArraySize(arrayDimList, 0, arrayDim), &code);
        return code;
    }
}

treeNode* translate_InitVal_array(treeNode *r, Operand place, int arrayDim, int* arrayDimList, int index, int* numCount, int numCountLim, CodeList* code) {
    printf("sdtParseConstInitVal_Array(arrayDim=%d, index=%d, numCount=%d, numCountLim=%d)\n",
        arrayDim, index, *numCount, numCountLim);
    if(r == NULL) return NULL;
    while(1)
    {
        if(r->next!=NULL)
        {
            r=r->next;
        }
        else
        {
            while(r)
            {
                r=r->father;
                if(r->next!=NULL)
                {
                    r=r->next;
                    break;
                }
            }
        }
        if(r==NULL)return NULL;
        r=getNextToken(r);
        printf("nextToken: %s\n",r->name);
        if(strcmp(r->name, "LC")==0)
        {
            if((*numCount)%getArraySize(arrayDimList, index+1, arrayDim)!=0) //如果当前维度没有填满，需要隐式初始化
            {
                int dim = getArraySize(arrayDimList,index+1,arrayDim);
                (*numCount)+=dim-(*numCount)%dim;
            }
            r = translate_InitVal_array(r, place, arrayDim, arrayDimList, index+1, numCount, *numCount+getArraySize(arrayDimList,index+1,arrayDim), code);
        }
        else if(strcmp(r->name, "RC")==0)
        {
            (*numCount)=numCountLim;
            return r;
        }
        else if(strcmp(r->name, "ConstExp") == 0 || strcmp(r->name, "Exp") == 0)
        {
            Operand t1 = new_temp(); //存放Exp的值
            Operand t2 = new_temp();
            *code = join(*code, translate_Exp(r, t1)); //计算Exp的值
            //a[*numCount] = val(Exp)
            InterCode ic = new_InterCode(IR_PLUS);
            ic->u.binop.op1 = place;
            ic->u.binop.op2 = new_constant(*numCount*4);
            ic->u.binop.result = t2;
            *code = join(*code, new_CodeList(ic));
            ic=new_InterCode(IR_CHANGE_ADDR);
            ic->u.assign.left=t2;
            ic->u.assign.right=t1;
            *code = join(*code, new_CodeList(ic));
            *numCount=*numCount+1;
        }
    }
}



/*
LOrExp -> LAndExp | LOrExp '||' LAndExp 
例子 a + b || c + d. 变换后是. [IF a+b != #0 GOTO label1] [GOTO label3] // LABEL label3 // [IF c+d != #0 GOTO label1] [GOTO label2] 
ok
*/
CodeList translate_LOrExp(treeNode *LOrExp, Operand label_true, Operand label_false) {
    if(!strcmp(LOrExp->child->name,"LAndExp")) {
        // LOrExp -> LAndExp
        return translate_LAndExp(LOrExp->child,label_true,label_false); // 可能是单一的无逻辑表达式-非0即真
    }
    else {
        // LOrExp -> LOrExp '||' LAndExp
        Operand label1 = new_label();
        CodeList code1 = translate_LOrExp(LOrExp->child, label_true,label1); // 左边部分的结果
        CodeList code2 = translate_LAndExp(LOrExp->child->next->next,label_true,label_false); // 右边部分的结果
        CodeList clabel1 = translate_Operand(label1,IR_LABEL); // [LABEL label1] 
        return join(join(code1,clabel1),code2); 
    }
}

/* 
LAndExp -> EqExp | LAndExp '&&' EqExp 
例子 a + b && c + d. 变换后是. [IF a+b != #0 GOTO label3] [GOTO label2] // LABEL label3 // [IF c+d != #0 GOTO label1] [GOTO label2] 
ok
*/
CodeList translate_LAndExp(treeNode *LAndExp, Operand label_true, Operand label_false) {
    if(!strcmp(LAndExp->child->name,"EqExp")) {
        // LAndExp -> EqExp 
        return translate_EqExp(LAndExp->child,label_true,label_false);
    }
    else {
        //LAndExp -> LAndExp '&&' EqExp
        Operand label1 = new_label();
        CodeList code1 = translate_LAndExp(LAndExp->child, label1,label_false);
        CodeList code2 = translate_EqExp(LAndExp->child->next->next,label_true,label_false);
        CodeList clabel1 = translate_Operand(label1,IR_LABEL); 
        return join(join(code1,clabel1),code2);  
    }   
}


/* 
EqExp -> RelExp | EqExp ('==' | '!=') RelExp 
用于设置运算结果 0 或 1的表达式结果 功能和例子见translate_EqExp注释 ok
*/
CodeList translate_EqExp_Calc(treeNode *EqExp, Operand place) {
    if(!strcmp(EqExp->child->name,"RelExp")) {
        // Operand t1 = new_temp();
        // return translate_RelExp_Calc(EqExp->child,t1);
        return translate_RelExp_Calc(EqExp->child,place);//是直接给place
    }
    else {
        Operand label1 = new_label();
        Operand label2 = new_label();
        InterCode ic = new_InterCode(IR_ASSIGN);
        ic->u.assign.left = place;
        ic->u.assign.right = new_constant(0);
        CodeList code0 = new_CodeList(ic);
        CodeList code1 = translate_EqExp(EqExp,label1,label2);
        CodeList code2 = translate_Operand(label1,IR_LABEL);
        ic = new_InterCode(IR_ASSIGN);
        ic->u.assign.left = place;
        ic->u.assign.right = new_constant(1);
        code2 = join(code2,new_CodeList(ic));
        CodeList code3 = translate_Operand(label2,IR_LABEL);
        return join(join(code0,code1),join(code2,code3));
    }
}

/* 
RelExp -> AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp 
功能和例子见translate_EqExp注释 ok
*/
CodeList translate_RelExp_Calc(treeNode *RelExp, Operand place) {
    if(!strcmp(RelExp->child->name,"AddExp")) {
        // Operand t1 = new_temp();
        // return translate_AddExp(RelExp->child,t1);
        return translate_AddExp(RelExp->child,place);
    }
    else {
        Operand label1 = new_label();
        Operand label2 = new_label();
        InterCode ic = new_InterCode(IR_ASSIGN);
        ic->u.assign.left = place;
        ic->u.assign.right = new_constant(0);
        CodeList code0 = new_CodeList(ic);
        CodeList code1 = translate_RelExp(RelExp,label1,label2);
        CodeList code2 = translate_Operand(label1,IR_LABEL);
        ic = new_InterCode(IR_ASSIGN);
        ic->u.assign.left = place;
        ic->u.assign.right = new_constant(1);
        code2 = join(code2,new_CodeList(ic));
        CodeList code3 = translate_Operand(label2,IR_LABEL);
        return join(join(code0,code1),join(code2,code3));
    }
}

/*
EqExp -> RelExp | EqExp ('==' | '!=') RelExp
函数调用思考例子
a + b == c > d // EqExp -> EqExp == RelExp 
分别调用translate_EqExp_Calc和translate_RelExp_Calc
左边有EqExp->RelExp->AddExp直到计算结果
右边有RelExp1 -> RelExp2 > AddExp, 调用translateRelExp(RelExp1-本身)
继续下去-计算出了c, d, 给出了跳转情况-最终RelExp1的值为1或者-1 
应该ok
*/
CodeList translate_EqExp(treeNode *EqExp, Operand label_true, Operand label_false) {
    if(!strcmp(EqExp->child->name,"RelExp")) {
        // EqExp -> RelExp 
        return translate_RelExp(EqExp->child, label_true, label_false);   //应该是这样把--自己还要思考一下label_true和label_false的情况
    }
    else {
        // EqExp -> EqExp ('==' | '!=') RelExp
        Operand t1 = new_temp();
        Operand t2 = new_temp();
        CodeList code1 = translate_EqExp_Calc(EqExp->child,t1);  // 之所以这样是因为EqExp也可以是一个逻辑表达式比如 EqExp -> EqExp == RelExp -> RelExp == RelExp -> RelExp < AddExp == RelExp -- 逻辑表达式的结果要么0要么1-如果直接调用自身-结果的值可能不对        
        CodeList code2 = translate_RelExp_Calc(EqExp->child->next->next,t2); //非常绕-可能出错-应该正确的
        InterCode ic = new_InterCode(IR_IFGOTO);
        ic->u.if_goto.x = t1;
        ic->u.if_goto.y = t2;
        ic->u.if_goto.z = label_true;
        ic->u.if_goto.relop = EqExp->child->next->value; // 符号 !=和 == 改成value存入了
        CodeList code3 = new_CodeList(ic);
        CodeList gotofalse = translate_Operand(label_false,IR_GOTO);
        return join(join(join(code1,code2),code3),gotofalse);
    }
}

/*
RelExp -> AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp ok
*/
CodeList translate_RelExp(treeNode *RelExp, Operand label_true, Operand label_false) {
    if(!strcmp(RelExp->child->name,"AddExp")) {
        // 关系表达式下的算数表达式的结果-非0即真！-从而完成跳转注意和translate_RelExp_Calc遇到AddExp时候的区别-后者是表达式计算
        Operand t1 = new_temp();
        CodeList code1 = translate_AddExp(RelExp->child,t1);
        InterCode ic = new_InterCode(IR_IFGOTO);
        ic->u.if_goto.x = t1;
        ic->u.if_goto.y = new_constant(0);
        ic->u.if_goto.z = label_true;
        ic->u.if_goto.relop = malloc(3);
        strcpy(ic->u.if_goto.relop,"!=");
        CodeList code2 = new_CodeList(ic);
        CodeList gotofalse = translate_Operand(label_false,IR_GOTO);
        return join(join(code1,code2),gotofalse);
    }
    else {
        // RelExp -> AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
        Operand t1 = new_temp();
        Operand t2 = new_temp();
        CodeList code1 = translate_RelExp_Calc(RelExp->child,t1);
        CodeList code2 = translate_AddExp(RelExp->child->next->next,t2);
        InterCode ic = new_InterCode(IR_IFGOTO);
        ic->u.if_goto.x = t1;
        ic->u.if_goto.y = t2;
        ic->u.if_goto.z = label_true;
        ic->u.if_goto.relop = RelExp->child->next->value; //
        CodeList code3 = new_CodeList(ic);
        CodeList gotofalse = translate_Operand(label_false,IR_GOTO);
        return join(join(join(code1,code2),code3),gotofalse);
    }
}


/* 
Cond -> LOrExp
直接往下调用LOrExp进一步翻译即可 ok
*/
CodeList translate_Cond(treeNode *Condition, Operand label_true, Operand label_false) {
    if(Condition == NULL)
        return NULL; 
    // Cond
    if(!strcmp(Condition->name,"Cond")) {   
        return translate_LOrExp(Condition->child,label_true,label_false);
    }
}



// /*
// 删除未被指向的label
// */
// void rmLabel()
// {
//     Label_No head = malloc(sizeof(struct Label_No_));
//     head->no = -1;
//     head->next = NULL;
//     Label_No tail = head;
//     InterCode  p;
//     CodeList tmp = code_head;
//     while(tmp != NULL) {
//         p = tmp->code;
//         if(p->kind == IR_GOTO)  // 记录能直接goto到的标签
//         {
//             Label_No temp = malloc(sizeof(struct Label_No_));
//             temp->no = p->u.op->u.label_no;  // label标号
//             temp->next = NULL;
//             tail->next = temp; 
//             tail = temp;   // temp添加到标签链表末尾
//         } 
//         else if(p->kind == IR_IFGOTO)  // 记录通过if IF x [relop] y GOTO z 到的标签
//         {
//             Label_No temp = malloc(sizeof(struct Label_No_));
//             temp->no = p->u.if_goto.z->u.label_no;  // label标号
//             temp->next = NULL;
//             tail->next = temp;
//             tail = temp;
//         }
//         tmp = tmp->next;
//     }

//     tmp = code_head;  // 重新遍历一遍IR链表
//     while(tmp!=NULL)
//     {
//         p = tmp->code;
//         if(p->kind == IR_LABEL)  // 找LABEL类型ir 然后查找之前记录的能goto到的label, 如果不能找到 说明是多余的标签, 需要删除
//         {
//             int no = p->u.op->u.label_no; // 获取label标号
//             Label_No ln = head;
//             while(ln != NULL)  // 遍历能被goto到的标签
//             {
//                 if(ln->no == no) break;
//                 ln = ln->next;
//             }
//             CodeList temp = tmp;
//             tmp = tmp->next;
//             if(ln == NULL) {
//                 deleteCode(temp);  // 遍历完没找到 则直接删除
//             }
//         }
//         else
//             tmp = tmp->next; // 其他情况直接往下
//     }
//     tail = NULL;
//     while(head != NULL)
//     {
//         Label_No ln = head;
//         head = head->next;
//         free(ln); // 清理内存
//     }
// }


// // 算数计算相关的内容 相同的右值? 应该是和临时变量相关
// // 具体内容待完成后补充
// void sameRight()
// {
//     CodeList tmp = code_head;
     
//     InterCode p, p1;
//     CodeList p_CL, p1_CL;
//     while(tmp != NULL)
//     {
//         p_CL = tmp;
//         p = p_CL->code;
//         if(p->kind == IR_PLUS || p->kind == IR_MINUS || p->kind == IR_MUL || p->kind == IR_DIV || p->kind == IR_MOD)
//         {
//             if(p->u.binop.result->kind==OP_TEMP)
//             {
//                 Operand r = p->u.binop.result;
//                 Operand op1 = p->u.binop.op1;
//                 Operand op2 = p->u.binop.op2;
//                 p1_CL = p_CL->next;
//                 p1 = p_CL->code;
//                 while(p1_CL!=NULL && p1->kind!=IR_RETURN && p1->kind != IR_GOTO && p1->kind != IR_IFGOTO && p1->kind != IR_CALL && p1->kind != IR_FUNC)
//                 {
//                     // READ先不看 if(p->kind==IR_READ)
//                     if((p1->kind==IR_CALL||p1->kind==IR_ASSIGN) && (p1->u.assign.left->kind==OP_ADDRESS || p1->u.assign.left->kind==OP_ARR_STRU || opEqual(p1->u.assign.left, op1) || opEqual(p1->u.assign.left,op2)))break;
//                     if((p1->kind==IR_PLUS || p1->kind == IR_MINUS || p1->kind == IR_MUL || p1->kind == IR_DIV || p1->kind == IR_MOD) && (p1->u.binop.result->kind == OP_ADDRESS || p1->u.binop.result->kind == OP_ARR_STRU || opEqual(p1->u.assign.left,op1) || opEqual(p1->u.assign.left,op2)))break;
//                     if(p1->kind == p->kind && p1->u.binop.result->kind == OP_TEMP && opEqual(p1->u.binop.op1,op1) && opEqual(p1->u.binop.op2,op2))
//                     {
//                         p1->u.binop.result->u.var_no = r->u.var_no; // 变量序号-应该有序号的吧
//                         // CodeList temp = p1_CL;
//                         p1_CL = p1_CL->next;
//                         p1 = p1_CL->code;
//                         // delteCode(temp);   增加删除中间代码的语句--直接从CodeList中删除
//                         continue;
//                     }
//                     p1_CL = p1_CL->next;
//                     p1 = p1_CL->code;
//                 }
//             }
//         }
//         tmp = tmp->next;
//     }
// }


// /*
// 找出常量并直接计算结果。
// 对中间代码双向链表进行遍历。对于计算类型的操作, 判断左右侧是否为常量。如果均是常量, 则将该代码替换为lhs, rhs计算结果的常量; ok
// */
// void calcConst()
// {
//     CodeList tmp = code_head;
//     InterCode p;
//     while(tmp != NULL)
//     {
//         p = tmp->code;
//         if(p->kind == IR_PLUS || p->kind == IR_MINUS || p->kind == IR_MUL || p->kind == IR_DIV || p->kind == IR_MUL)
//         {
//             if(p->u.binop.result->kind == OP_TEMP && p->u.binop.op1->kind == OP_CONSTANT && p->u.binop.op2->kind == OP_CONSTANT)
//             { 
//                 int c1 = p->u.binop.op1->u.val;  // 应该常量值有直接存吧 不确定-实现的时候是否已经计算了常量-感觉还是放到优化里面合理-应该都可以
//                 int c2 = p->u.binop.op2->u.val;
//                 int r = 0;
//                 switch(p->kind) {
//                     case IR_PLUS  : r = c1 + c2; break;
//                     case IR_MINUS : r = c1 - c2; break;
//                     case IR_MUL   : r = c1 * c2; break;
//                     case IR_DIV   : r = c1 / c2; break;
//                     case IR_MOD   : r = c1 % c2; break;
//                     default: break;
//                 }
//                 p->u.binop.result->kind = OP_CONSTANT;
//                 p->u.binop.result->u.val = r; // 应该直接int存就行了吧
//                 CodeList temp = tmp;
//                 tmp = tmp->next;
//                 deleteCode(temp);  
//                 continue;
//             }
//         }
//         tmp = tmp->next;
//     }
// }



// // IF跳转语句的优化
// void optimizeIF()
// {
//     CodeList tmp = code_head;
//     InterCode p;

//     CodeList p1_CL, p2_CL, p3_CL; 
//     InterCode p1, p2, p3;  
//     while(tmp != NULL)
//     {   
//         p = tmp->code;
//         if(p->kind == IR_IFGOTO)
//         {
//             p1 = p;
//             p2_CL = tmp->next;
//             p2 = p2_CL->code;
//             if(p2 == NULL) continue;
//             p3_CL = p2_CL->next;
//             p3 = p3_CL->next;
//             if(p3 == NULL) continue;
//             if(p2->kind == IR_GOTO && p3->kind == IR_LABEL && p1->u.if_goto.z == p3->u.op)
//             {
//                 p1->u.if_goto.z = p2->u.op; // z就是label吧
//                 // deleteCode(p2_CL);
//                 if(strcmp(p1->u.if_goto.relop,"==") == 0)  // 忘记了是存的符号表示还是名字表示了
//                 {
//                     p1->u.if_goto.relop = malloc(4);  // 要修改 -- 下次再看
//                     memset(p1->u.if_goto.relop,0,4);
//                     strcpy(p1->u.if_goto.relop,"!=");
//                 }
//                 else if(strcmp(p1->u.if_goto.relop,"!=") == 0)
//                 {
//                     p1->u.if_goto.relop = malloc(4);  // 要修改 -- 下次再看
//                     memset(p1->u.if_goto.relop,0,4);
//                     strcpy(p1->u.if_goto.relop,"==");
//                 }
//                 else if (strcmp(p1->u.if_goto.relop,"<"))
//                 {
//                     p1->u.if_goto.relop = malloc(4);
//                     memset(p1->u.if_goto.relop,0,4);
//                     strcpy(p1->u.if_goto.relop,">=");
//                 }
//                 else if(strcmp(p1->u.if_goto.relop,">")==0)
//                 {
//                     p1->u.if_goto = malloc(4);
//                     memset(p1->u.if_goto.relop,0,4);
//                     strcpy(p1->u.if_goto.relop,"<=");
//                 }
//                 else if(strcmp(p1->u.if_goto.relop,"<=") == 0)
//                 {
//                     p1->u.if_goto.relop == malloc(4);
//                     memset(p1->u.if_goto.relop,0,4);
//                     strcpy(p1->u.if_goto.relop,">");
//                 }
//             }
//         }
//         else if(p->kind == IR_GOTO)
//         {
//             p1_CL = tmp;
//             p1 = p;
//             p2_CL = tmp->next;
//             p2 = p2_CL->code;
//             if(p2 != NULL && p2->kind == IR_LABEL && p1->u.op == p2->u.op)
//             {
//                 tmp = tmp->next;
//                 // deleteCode(p1_CL);  //
//             }
//         }
//         tmp = tmp->next;
//     }
// }


// // 判断操作数是否相同
// int opEqual(Operand op1, Operand op2)
// {
//     if(op1 == NULL || op2 == NULL)
//         return 0;
//     if(op1->kind == OP_TEMP || op1->kind==OP_VARIABLE || op1->kind == OP_CONSTANT); //ok
//     else return 0;
//     if(op1 == op2)return 1;
//     if(op1->kind == OP_TEMP && op2->kind == OP_TEMP && op1->u.var_no == op2->u.var_no)
//         return 1;
    
//     if(op1->kind == OP_VARIABLE && op2->kind == OP_VARIABLE && (op1->u.val == op2->u.val )) // 应该有存值把
//         return 1;
    
//     if(op1->kind == OP_CONSTANT && op2->kind == OP_CONSTANT)
//     {
//         if (op1->u.val == op2->u.val)  // 常量值应该是存了的
//             return 1;   
//     }
//     return 0;
// }



// // 死代码删除？
// void rddCode()
// {
//     CodeList top = code_tail;     //?
//     CodeList bottom = code_tail;  //?
//     InterCode top_code;
//     InterCode bottom_code;
//     // 自底向上    
//     while(1) 
//     {
//         // find a block
//         bottom = top->prev;
//         if(bottom == NULL)
//             break;
//         top = bottom->prev;
//         if(top == NULL) 
//             break; // ????

//         while(top != NULL)
//         {
//             top_code =top->code;
//             if(top_code->kind == IR_RETURN || top_code->kind == IR_GOTO || top_code->kind==IR_IFGOTO || top_code->kind == IR_CALL)
//             {
//                 top = top->prev;
//                 top_code = top->code;
//                 break;
//             }
//             else if(top_code->kind == IR_LABEL || top_code->kind == IR_FUNC)
//                 break;
//             top = top->prev;
//         }
//         if(top == NULL)
//             top = code_head;
//         if(bottom == top)
//             continue;

//         // deal with this block
//         while(bottom != NULL && bottom != top->prev)
//         {
//             Operand noAct = NULL;
//             CodeList p_CL = bottom;
//             InterCode p = p_CL->code;
//             int flag = 0;
//             if(p->kind == IR_ASSIGN)
//             {
//                 if(p->u.assign.left->kind == OP_VARIABLE && !opEqual(p->u.assign.left,p->u.assign.right))
//                 {
//                     noAct = p->u.assign.left;
//                     flag = 1;
//                 }
//             }
//             else if(p->kind == IR_PLUS || p->kind == IR_MINUS || p->kind == IR_MUL || p->kind == IR_DIV || p->kind == IR_MOD)
//             {
//                 if(p->u.binop.result->kind == OP_VARIABLE && !opEqual(p->u.binop.result,p->u.binop.op1) && !opEqual(p->u.binop.result,p->u.binop.op2))
//                 {
//                     noAct = p->u.binop.result;
//                     flag = 1;
//                 }
//             }
//             if(flag)
//             {
//                 p_CL = p_CL->prev;
//                 p = p_CL->code;
//                 while(p_CL != NULL && p_CL != top->prev)
//                 {
//                     if(p->kind == IR_ASSIGN || p->kind == IR_CALL)
//                     {
//                         if(opEqual(noAct, p->u.assign.right) || p->u.assign.left->kind == OP_ARR_STRU)break; ///应该是这个把?
//                         if(p->u.assign.right->kind == OP_ARR_STRU || p->u.assign.right->kind == OP_ADDRESS)break; // 不确定
//                         if(opEqual(noAct,p->u.assign.left))
//                         {
//                             CodeList temp = p_CL;
//                             p_CL = p_CL->prev;
//                             p = p_CL->code;
//                             // deleteCode(temp);
//                             continue;
//                         }
//                     }
//                     else if(p->kind == IR_PLUS || p->kind == IR_MINUS || p->kind == IR_DIV || p->kind == IR_MUL || p->kind == IR_MOD)
//                     {
//                         if(opEqual(noAct,p->u.binop.op1) || opEqual(noAct,p->u.binop.op2))
//                             break;
//                         if(p->u.binop.result->kind == OP_ARR_STRU)break;
//                         if(p->u.binop.op1->kind == OP_ARR_STRU || p->u.binop.op1->kind == OP_ADDRESS)
//                             break;
//                         if(p->u.binop.op2->kind == OP_ARR_STRU || p->u.binop.op2->kind == OP_ADDRESS)
//                             break;
//                         if(opEqual(noAct, p->u.binop.result))
//                         {
//                             CodeList temp = p_CL;
//                             p_CL = p_CL->prev;
//                             p = p_CL->code;
//                             // deleteCode(temp)
//                             continue;
//                         }
//                     }
//                     // read 和 write就先不弄了
//                     // 还有个不清晰的
//                     p_CL = p_CL->prev;
//                     p = p_CL->code;
//                 }
//             }
//             bottom = bottom->prev;
//         }
//     }
// }
// // 都是不完善的--还需要修改

// /* 
// 在双向中间代码链表中删除指定中间代码链表节点 ok
// */
// void deleteCode(CodeList c)
// {
//     if(c == code_head && c == code_tail)
//     {
//         code_head = NULL;
//         code_tail = NULL;
//     }
//     else if(c == code_head)
//     {
//         if(c->next != NULL)
//         {
//             c->next->prev = NULL;
//         }
//         code_head = c->next;
//     }
//     else if(c == code_tail)
//     {
//         if(c->prev != NULL)
//         {
//             c->prev->next = NULL;
//         }
//         code_tail = c->prev;
//     }
//     else
//     {
//         if(c->next != NULL)
//         {
//             c->next->prev = c->prev;
//         }
//         if(c->prev != NULL)
//         {
//             c->prev->next = c->next;
//         }
//     }
//     free(c->code);
//     free(c);
// }
// //还需要再检查一下-链表结构也有点忘记了