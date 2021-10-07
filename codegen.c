#include "codegen.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "intercode.h"
#define isConst(op) (op->kind==OP_CONSTANT)
#define opVal(op) (op->u.val)
asmCodeList target_codelist = NULL;

int isText = -1; //-1: 初始 0：在data段中 1：在text段中
asmCodeList new_asmCodeList(char* code)
{
    asmCodeList cl = (asmCodeList)malloc(sizeof(struct asmCodeList_));
    cl->code = code; 
    cl->next = cl->prev = NULL;  // 双向链表
    return cl;
}

// 合并两个CodeList
asmCodeList asm_join(asmCodeList head, asmCodeList body)
{
    if(head == NULL) {
        return body;
    }
    asmCodeList tmp = head;
    while(tmp->next != NULL) {  // 链表结尾是NULL
        tmp = tmp->next;
    }
    tmp->next = body;
    if(body != NULL) {
        body->prev = tmp;
    }
    return head;
}

CodeList currentCode = NULL;  // 记录主循环中当前遍历到的中间代码
void codegen(CodeList cl){
    if(cl==NULL)
    {
        fprintf(stderr, "CodeList is NULL!\n");
        return;
    }
    asm_join(target_codelist, gen_header());
    while(cl)
    {
        currentCode = cl;  // 
        InterCode p = cl->code;
        if(p->kind == IR_FUNC){
            target_codelist = asm_join(target_codelist, gen_ir_func(p));
            cl=cl->next;
            while(cl)
            {
                if(cl->code->kind == IR_ENDFUNC) break;
                switch (cl->code->kind)
                {
                    case IR_ASSIGN:  // x := y 
                        gen_ir_assign(ir); break;
                    case IR_LABEL:   // LABEL x :
                        gen_ir_label(ir);break;
                    case IR_PLUS:   // x := y+z
                        gen_ir_plus(ir, "ADD");break;
                    case IR_MINUS:   // x := y-z
                        gen_ir_plus(ir, "SUB");break;
                    case IR_MUL:     // x := y*z
                        gen_ir_plus(ir, "MUL");break;
                    case IR_DIV:     // x := y/z
                        gen_ir_plus(ir, "DIV");break;
                    case IR_MOD:     // x : = y%z 
                        gen_ir_plus(ir, "MOD");break; // arm无mod-需要修改
                    case IR_FUNC:    // FUNCTION f :
                        gen_ir_func(ir);break;
                    case IR_GOTO:    // GOTO x
                        gen_ir_goto(ir);break;
                    case IR_IFGOTO:  // IF x [relop] y GOTO z 
                        gen_ir_ifgoto(ir);break;
                    case IR_RET:     // x = *y
                        // 不太确定
                        break;
                    case IR_DEC:     // DEC x [size] 内存空间申请-大小为4的倍数 用于数组
                        //不太确定
                        gen_ir_dec(ir);break;
                    case IR_ARG:     // ARG x
                        gen_ir_arg(ir);break;
                    case IR_CALL:    // x := CALL f
                        gen_ir_call(ir);break;
                    case IR_PARAM:   // PARAM x
                        gen_ir_params(ir);break;
                    case IR_READ:    // 不用
                        // gen_ir_read(ir);
                        break;
                    case IR_WRITE:  // 不用
                        // gen_ir_write(ir);
                        break;
                    case IR_RETURN:  // RETURN x
                        gen_ir_return(ir);break;
                    case IR_CHANGE_ADDR: // *x := y 取y值以赋给x值为地址的内存单元
                        // 不确定
                        break;
                    case IR_GET_ADDR:     // x := &y 取y的地址赋给x  // 名字可能起的不是很好
                        // 不确定
                        break;
                    default:
                        printf("Error: Unknown Kind to ARM32\n");
                        break;
                }
                cl=cl->next;
            }
            cl=cl->next;
        }
    }
}

asmCodeList gen_header()
{
    return asm_join(
        asm_join(
            new_asmCodeList(".macro mov32, reg, val"),
            new_asmCodeList("\tmovw \\reg, #:lower16:\\val")
        ),
        asm_join(
            new_asmCodeList("\tmovt \\reg, #:upper16:\\val"),
            asm_join(
                new_asmCodeList(".endm"),  
                new_asmCodeList("\n")
            )
        )
    );
}

asmCodeList gen_ir_func(InterCode ir)
{
    CodeList res = NULL;
    if(isText != 1){
        res = asm_join(res, new_asmCodeList(".text"));
        char* msg = malloc(64);
        sprintf(msg, ".global %s", ir->u.func);
        res = asm_join(res, new_asmCodeList(msg));
        msg = malloc(64);
        sprintf(msg, ".type\t %s, %%function", ir->u.func);
        res = asm_join(res, new_asmCodeList(msg));
        msg = malloc(64);
        sprintf(msg, "%s:", ir->u.func);
        res = asm_join(res, new_asmCodeList(msg));
    }
    else if(isText == 0){
        int frameSize = getframeSize(); //获取局部变量以及需要用到的参数的值(小于等于4个-如果多余-直接用fp去往前找) 
        stackSize = frameSize; // 这个全局变量-其他地方可能会用到
        char* msg = malloc(64);
        //sprintf(msg, "%s:\n\tSUB sp, sp, #4\n\tSTR $fp, 0(sp)\n\tmov $fp, $sp\n\tsubu $sp, $sp, %d\n",interCode->u.sinop.op->u.value,stackSize);
        sprintf(msg, "%s:\n\tSUB sp, sp, #4\n\tSTR fp, [sp, #0]\n\tMOV fp, sp,\n\tSUB sp, sp, #%d\n", ir->u.func,frameSize);
        // SUB sp, sp, #4
        // STR fp, [sp, #0]  -- 存老的fp -- lr存储这里是在CALL解析的时候做
        // MOV fp, sp  -- fp指向栈底
        // SUB sp, sp, #stacksize -- 函数的变量空间
        spOffset = 0;
        curParam = 0;
        res = new_asmCodeList(msg);
    }
    isText = 1;
    return res;
}

// 获取一个函数的栈帧大小
int getframeSize()
{
    CodeList cl = currentCode;  // 遍历函数部分中间代码-记录变量数
    //从function开始往下
    int countParam = 0;
    int countVarTmp = 0;
    while(cl)
    {
        InterCode p = cl->code;
        if(p->kind == IR_PARAM)
        {
            ++countParam;
        }
        if(p->kind == IR_ASSIGN || p->kind == IR_PLUS || p->kind == IR_MINUS || 
            p->kind == IR_MUL || p->kind == IR_DIV || p->kind==IR_MOD) // 可能还有address的kind以及数组的空间申请
        {  
            //左边变量名在中间代码里面应该没有重复的-所以直接加上去了-区别就是没有和源代码对应
            ++countVarTmp;
        }
        if(p->kind == IR_DEC)
        {
            countVarTmp += p->u.dec.size;  //这里数组空间申请可能还是不太清楚
        }
        if(p->kind == IR_ENDFUNC)
            break;
        cl = cl->next;
    }
    int frameSize = 4 * (countParam - 4) + 4 * countVarTmp;
    return frameSize;
}

// 对应的就是 DEC x [size] 申请数组空间 也不确定
asmCodeList gen_ir_dec(InterCode ir){
    Var_t *arrayHead = malloc(sizeof(Var_t));  
    spOffset -= 4;
    arrayHead->offset = spOffset;
    spOffset -= ir->u.dec.size;
    char *arrayName = malloc(32);
    memset(arrayName, 0, sizeof(arrayName));
    if(ir->u.dec.x->kind == OP_VARIABLE)
    {
        sprintf(arrayName, "v%d", ir->u.dec.x->u.var_no);
        arrayHead->name = arrayName;
    }
    else if (ir->u.dec.x->kind == OP_TEMP)
    {   
        sprintf(arrayName, "t%d", ir->u.dec.x->u.temp_no);        
        arrayHead->name = arrayName;        
    }
    addVar(arrayHead);
    char msg = malloc(64);
    sprintf(msg, "\tADD r4, fp, #%d\n\tSTR r4, [fp, #%d]", spOffset, arrayHead->offset);
    // 这里也不太确定--写一个字节系的是什么呢？
    return new_asmCodeList(msg);
}

/*char* trans_operand(Operand op)
{
    char msg[64] = "";
    if(op->kind == OP_CONSTANT) {
        sprintf(msg,"#%d",op->u.val); // 常量(目前应该叫立即数). 例如 #0
    }
    else if(op->kind == OP_LABEL) {
        sprintf(msg,"label%d",op->u.label_no); // 标签. 例如 label1. 全局唯一
    }
    else{
        int regid = getreg(op);  //分配寄存器
        sprintf(msg, "r%d", regid);
    }
    char *ret = malloc(strlen(msg)+1);         // 返回空间指针
    strcpy(ret, msg);
    return ret; 
}*/
int getreg(Operand op)
{
    return 1;
}

asmCodeList load_to_reg(Operand op, int regid)
{
    return NULL;
}

// ok
asmCodeList gen_ir_label(InterCode ir)
{
    char* msg = (char*)malloc(64);
    sprintf(msg, "label%d:", ir->u.op->u.label_no);
    return new_asmCodeList(msg);
}


asmCodeList gen_ir_assign(InterCode ir)
{
    if(isConst(ir->u.assign.right))
    {
        int reg_left = getreg(ir->u.assign.left);
        char* msg = (char*)malloc(64);
        sprintf(msg, "\tMOV r%d, #%d", reg_left, opVal(ir->u.assign.right));
        return new_asmCodeList(msg);
    } 
    else 
    {
        int reg_left = getreg(ir->u.assign.left);
        int reg_right = getreg(ir->u.assign.right);
        asmCodeList load_right = load_to_reg(ir->u.assign.right, reg_right);
        char* msg = (char*)malloc(64);
        sprintf(msg, "\tMOV r%d, r%d", reg_left, reg_right);
        asmCodeList mov_value = new_asmCodeList(msg);
        //asmCodeList store_left = store_to_reg(ir->u.assign.left, reg_left);
        return asm_join(load_right, mov_value);
    }
}

// 将变量对应的寄存器值保存到栈上 ok
asmCodeList swReg(int index){
    char* msg = malloc(64);
    memset(msg, 0, 64);
    Var_t *var = regs[index].var;
    sprintf(msg, "\tSTR %s, [fp, #%d]\n", regName[index], var->offset);
    return new_asmCodeList(msg);
}
// 将栈上值加载到寄存器上 ok
asmCodeList lwReg(int index, Var_t *var) {
    char* msg = malloc(64);
    memset(msg, 0, 64);
    regs[index].var = var;
    sprintf(msg, "\tLDR %s, [fp, #%d]\n", regName[index], var->offset);
    return new_asmCodeList(msg);
}

// x := CALL f ok
asmCodeList gen_ir_call(InterCode ir)
{
    char* msg = malloc(64);
    asmCodeList res;
    sprintf(msg, "\tSUB sp, sp, #4\n");    // SUB sp, sp, #4 -- 开出一个字节的空间
    asmCodeList sub_sp = new_asmCodeList(msg);
    sprintf(msg, "\tSTR, lr, [sp, #0]\n");  // STR LR, [sp, #0] -- 保存返回地址
    asmCodeList str_lr = new_asmCodeList(msg);
    res = asm_join(sub_sp, str_lr);

    ir->u.call.func;  
    Operand op = ir->u.call.result;    // 函数返回值接受对象
    int x = getreg(op);  // 大概是寄存器返回的序号

    // BL f
    // MOV reg(x), r0
    sprintf(msg, "\tBL %s\n\tMOV %s, r0\n", ir->u.call.func, regName[x]); /// 注意x的返回情况
    asmCodeList bl_mov = new_asmCodeList(msg);
    res = asm_join(res, bl_mov);
    asmCodeList swreg =  swReg(x);
    res = asm_join(res, swreg);

    sprintf(msg, "\tLDR lr, [sp, #0]\n");  // LDR, lr, [sp, #0] -- 加载原来的lr
    asmCodeList ldr_lr = new_asmCodeList(msg);
    res = asm_join(res, ldr_lr);    

    sprintf(msg, "\tADD sp, sp, #4\n");    // ADD, sp, sp, #4 -- 回收lr的空间
    asmCodeList add_sp = new_asmCodeList(msg);
    res = asm_join(res, add_sp);
    curArg = 0; // arg数清0

    return res;
}

// ARG x ok
asmCodeList gen_ir_arg(InterCode ir)
{
    char* msg = malloc(64);
    memset(msg, 0, 64);
    Operand op = ir->u.op;
    Var_t *arg = NULL;
    if(op->kind == OP_TEMP){
        char argName[20];
        memset(argName, 0, 20);
        sprintf(argName, "t%d", op->u.var_no);
        arg = findVar(argName);
    } else if(op->kind == OP_VARIABLE){
        char argName[20];
        memset(argName, 0, 20);
        sprintf(argName, "v%d", op->u.var_no);
        arg = findVar(argName);
        // 这部分之后可能会会统一
    } 
    if(arg == NULL)
        exit(-1);
    
    /// 实参传递 - 小于4个直接放到r0到r3寄存器中
    /// 多余4个放到栈上 -- 这里我也不清楚栈的变化情况？属于当前函数的栈的-变量部分?
    if(curArg < 4){
        sprintf(msg, "\tLDR r%d, [fp, #%d]\n", curArg, arg->offset); // LDR r0-r3, [fp, #offset] // 应该就是根据fp的offset定位变量位置
    }
    else { 
        sprintf(msg, "\tLDR r4, [fp, #%d]\n\tSUB sp, sp, #4\n\tSTR r4, [sp, #0]\n", arg->offset);
        // LDR r4, [fp, #offset]
        // SUB sp, sp, #4 
        // STR r4, [sp, #0]
        // 原来的mips版本如下-比较疑惑-最后应该是sw才对
        // sprintf(str, "\tlw $s0, %d($fp)\n\tsubu $sp, $sp, 4\n\tlw $s0, 0($sp)\n", arg->offset);
    }
    ++curArg;
    if(currentCode->next==NULL || currentCode->next->code->kind != IR_ARG) // 实参传递结束
    {
        curArg = 0;  
        return NULL;
    }
    return new_asmCodeList(msg);
}

// RETURN x ok
asmCodeList gen_ir_return(InterCode ir)
{
    char* msg = malloc(64);
    memset(msg, 0, 64);
    Operand op = ir->u.op;  // RETURN x
    if(op->kind!=OP_CONSTANT){
        int x = getreg(op);
        // MOV r0, reg(x)  -- 返回值存r0上
        // ADD sp, sp, #stacksize  -- stacksize的计算和传递!!未完善 -- 这里回收变量栈空间
        // LDR fp, [sp, #0] -- 加载老的fp指针
        // ADD sp, sp, #4 -- 回收空间
        // MOV pc, lr -- 指令跳转
        // lr寄存器的空间的回收在gen_ir_call中进行
        sprintf(msg, "\tMOV r0, %s\n\tADD sp, sp, #%d\n\tLDR fp, [sp, #0]\n\tADD sp, sp, #4\n\tMOV pc, lr\n", regName[x], stackSize); // stackSize空间传递未完善
    } 
    else {
        // 常数的输出 --感觉还是有错的
        // MOV r0, #constVal
        // ADD sp, sp, #stacksize
        // LDR fp, [sp, #0]
        // ADD sp, sp, #4
        // MOV pc, lr
        sprintf(msg, "\tmov r0, #%d\n\tADD sp, sp, #%d\n\tLDR fp, [sp, #0]\n\tADD sp, sp, #4\n\tMOV pc, lr\n", op->u.val, stackSize);
    }
    return new_asmCodeList(msg);
}

// PARAM x ok
asmCodeList gen_ir_params(InterCode ir)
{
    char* msg = malloc(64);
    Var_t* param = malloc(sizeof(Var_t));
    param->name = malloc(20);  // 注意在变量很大的情况下可能会超出-要改进
    sprintf(param->name, "v%d",ir->u.op->u.var_no); // 应该是参数也是按变量来的
    spOffset -= 4;  // 应该就是对fp的偏移才对
    param->offset = spOffset;
    addVar(param);

    // 对于前4个参数 存入a0~a3寄存器中 - 这里就是将寄存器里的参数放到本函数栈上 - 所以是STR
    if(curParam < 4){
        sprintf(msg, "\tSTR r%d, [fp, #%d]\n", curParam, param->offset); // 存储r0-r3-并且优先处理寄存器上参数
        // STR r0-r3, [fp, #offset]
    }
    // 对于其他参数 传递是放到栈上 - 这里就是从栈上拿出来放到寄存器上 - 再放回到本函数栈上
    else {
        sprintf(msg, "\tLDR r0, [fp, #%d]\n\tSTR r0, [fp, #%d]\n", (curParam-2)*4, param->offset); 
        // LDR r0, [fp, #从fp找参数的偏移]
        // STR r0, [fp, #到本栈变量区的偏移]
    }
    asmCodeList result = new_asmCodeList(msg);
    ++curParam;
    return result;
}

// 可能对应的是 IR_GET_ADDR ?
asmCodeList gen_ir_address(InterCode ir)
{
    Operand leftOp = ir->u.assign.left;
    Operand rightOp = ir->u.assign.right;
    Var_t *arrayHead = NULL;
    if(rightOp->kind == OP_TEMP) {
        char *arrayName = malloc(32);
        memset(arrayName, 0, sizeof(arrayName));
        sprintf(arrayName, "t%d", rightOp->u.var_no);
        arrayHead = findVar(arrayName);
    } else if(rightOp->kind == OP_VARIABLE) {
        char *arrayName = malloc(32);
        memset(arrayName, 0, sizeof(arrayName));
        sprintf(arrayName, "v%d", rightOp->u.var_no);
        arrayHead = findVar(arrayName);
    }
    if (arrayHead == NULL){
        exit(-1);
    }
    int x = getreg(leftOp);
    char msg = malloc(64);
    memset(msg, 0, 64);
    sprintf(msg, "\tLDR %s, [fp, #%d]\n",regName(x),arrayHead->offset);
    asmCodeList swregCode = swReg(x);
    asmCodeList result = asm_join(new_asmCodeList(msg), swregCode);
    return result/
}

// 删除函数变量表 ok
void delVars(){
    Var_t *ptr = varList;
    while(ptr != NULL) {
        varList = varList->next;
        free(ptr);
        ptr = varList;
    }
}

// 添加到函数变量表 ok
void addVar(Var_t * var)
{
    if(var==NULL)
        exit(-1);
    var->next = NULL;
    if(var_tail == NULL){
        varList = var;
    } else {
        Var_t *ptr = varList;
        while(ptr->next != NULL)
            ptr = ptr->next;
        ptr->next = var;
    }
}

// 在Var_t中遍历查找相同的名字-返回对应的变量指针 ok
Var_t* findVar(char* name) {
    Var_t *ptr = varList;
    while(ptr != NULL){
        if(strcmp(ptr->name, name)==0) {
            break;
        } else{
            ptr = ptr->next;
        }
    }
    return ptr;
}

// 初始化寄存器 ok
void initRegs(){
    int i = 0;
    for(i = 0; i < 16;i++) // 一共16个寄存器
    {
        regs[i].name = regName[i];
        regs[i].var = NULL;
    }
}

asmCodeList gen_ir_plus(InterCode ir, char* op_type)
{
    int reg_left = getreg(ir->u.binop.result);
    if(isConst(ir->u.binop.op1) && isConst(ir->u.binop.op2))
    {
        char* msg = (char*)malloc(64);
        sprintf(msg, "\t%s r%d, #%d, #%d", op_type, reg_left, opVal(ir->u.binop.op1), opVal(ir->u.binop.op2));
        return new_asmCodeList(msg);
    } 
    else if(isConst(ir->u.binop.op1)) 
    {
        int reg_op2 = getreg(ir->u.binop.op2);
        asmCodeList load_reg_op2 = load_to_reg(ir->u.binop.op2, reg_op2);
        char* msg = (char*)malloc(64);
        sprintf(msg, "\t%s r%d, #%d, r%d", op_type, reg_left, opVal(ir->u.binop.op1), reg_op2);
        return asm_join(load_reg_op2, new_asmCodeList(msg));
    }
    else if(isConst(ir->u.binop.op2)) 
    {
        int reg_op1 = getreg(ir->u.binop.op1);
        asmCodeList load_reg_op1 = load_to_reg(ir->u.binop.op1, reg_op1);
        char* msg = (char*)malloc(64);
        sprintf(msg, "\t%s r%d, r%d, #%d", op_type, reg_left, reg_op1, opVal(ir->u.binop.op2));
        return asm_join(load_reg_op1, new_asmCodeList(msg));
    }
    else 
    {
        int reg_op1 = getreg(ir->u.binop.op1);
        int reg_op2 = getreg(ir->u.binop.op2);
        asmCodeList load_reg_op1 = load_to_reg(ir->u.binop.op1, reg_op1);
        asmCodeList load_reg_op2 = load_to_reg(ir->u.binop.op2, reg_op2);
        char* msg = (char*)malloc(64);
        sprintf(msg, "\t%s r%d, r%d, r%d", op_type, reg_left, reg_op1, reg_op2);
        asmCodeList calc_value = new_asmCodeList(msg);
        return asm_join(load_reg_op1, asm_join(load_reg_op2, calc_value));
    }
}

asmCodeList gen_ir_goto(InterCode ir)
{
    char* msg = (char*)malloc(64);
    sprintf(msg, "\tB label%d", ir->u.op->u.label_no);
    return new_asmCodeList(msg);
}

asmCodeList gen_ir_ifgoto(InterCode ir)
{
    int label_id = ir->u.if_goto.z->u.label_no;
    char* relop = ir->u.if_goto.relop;
    char* cmpmsg = (char*)malloc(64);
    char* msg = (char*)malloc(64);
    memset(cmpmsg, 0, sizeof(cmpmsg));
    memset(msg, 0, sizeof(msg));
    char* code_op;
    if(!strcmp(relop, ">"))
    {
        code_op = "bgt";
    }
    else if(!strcmp(relop, ">="))
    {
        code_op = "bge";
    }
    else if(!strcmp(relop, "<"))
    {
        code_op = "blt";
    }
    else if(!strcmp(relop, "<="))
    {
        code_op = "ble";
    }
    else if(!strcmp(relop, "=="))
    {
        code_op = "beq";
    }
    else if(!strcmp(relop, "!="))
    {
        code_op = "bne";
    }

    if(isConst(ir->u.if_goto.x) && isConst(ir->u.if_goto.y))
    {
        sprintf(cmpmsg, "\tcmp #%d, #%d", opVal(ir->u.if_goto.x), opVal(ir->u.if_goto.y));
        sprintf(msg, "\t%s label%d", code_op, label_id);
        return asm_join(new_asmCodeList(cmpmsg), new_asmCodeList(msg));
    }
    else if(isConst(ir->u.if_goto.x))
    {
        int reg_y = getreg(ir->u.if_goto.y);
        asmCodeList load_reg_y = load_to_reg(ir->u.if_goto.y, reg_y);
        sprintf(cmpmsg, "\tcmp #%d, r%d", opVal(ir->u.if_goto.x), reg_y);
        sprintf(msg, "\t%s label%d", code_op, label_id);
        return asm_join(new_asmCodeList(cmpmsg), new_asmCodeList(msg));
    }
    else if(isConst(ir->u.if_goto.y))
    {
        int reg_x = getreg(ir->u.if_goto.x);
        asmCodeList load_reg_x = load_to_reg(ir->u.if_goto.x, reg_x);
        sprintf(cmpmsg, "\tcmp r%d, #%d", reg_x, opVal(ir->u.if_goto.y));
        sprintf(msg, "\t%s label%d", code_op, label_id);
        return asm_join(new_asmCodeList(cmpmsg), new_asmCodeList(msg));
    }
    else
    {
        int reg_x = getreg(ir->u.if_goto.x);
        int reg_y = getreg(ir->u.if_goto.y);
        asmCodeList load_reg_x = load_to_reg(ir->u.if_goto.x, reg_x);
        asmCodeList load_reg_y = load_to_reg(ir->u.if_goto.y, reg_y);
        sprintf(cmpmsg, "\tcmp r%d, r%d", reg_x, reg_y);
        sprintf(msg, "\t%s label%d", code_op, label_id);
        return asm_join(new_asmCodeList(cmpmsg), new_asmCodeList(msg));
    }
}


Register_ regs[16];
Var_t *varList = NULL;
int curReg = 0;
int spOffset = 0;
int curParam = 0;
int curArg = 0;
int stackSize = 100;  //栈的变量区大小

char* regName[] = {
    "r0","r1","r2","r3","r4","r5","r6","r7","r8","r9","r10","fp","r12",
    "sp","lr", "pc"
}