#include "node.h"
#include "list.h"
#include "vector.h"
#include "symtable.h"
#include <string.h>
#define HT_SIZE 233
#define HT_SEED 32
#define NEW(p) ((p) = malloc(sizeof *(p)))
#define NEW0(p) memset(NEW(p), 0, sizeof *(p))
List funcTable[HT_SIZE], paraTable[HT_SIZE];
const char *RESERVE_WORDS[] = {(const char *)"int\0", (const char *)"void\0", (const char *)"NULL\0"};
int scope = 0, whileCnt = 0;
vector scopeTable;
int errorcnt = 0;

void handleError(int no, int lineno)
{
    errorcnt ++;
    printf("Error %d on Line %d: ", no, lineno);
    switch (no)
    {
    case 1: { printf("storage size of this variable isn't known. \n"); break; }
    case 2: { printf("division by zero. \n"); break; }
    case 3: { printf("function cannot be called in a constant expression. \n"); break;}
    case 4: { printf("size of array is negative. \n"); break; }
    case 5: { printf("excess elements in scalar initializer. \n"); break; }
    case 6: { printf("invalid initializer. \n"); break; }
    case 7: { printf("the variable has't been defined. \n"); break; }
    case 8: { printf("too many braces around scalar initializer. \n"); break; }
    case 9: { printf("const type variable is required here. \n"); break; }
    case 10: { printf("excess elements in scalar initializer. \n"); break; }
    case 11: { printf("an identifier can not be the same as reserve names. \n"); break; }
    case 12: { printf("repeated definition. \n"); break; }
    case 13: { printf("the continue/break statement is not in while loop. \n"); break; }
    case 14: { printf("NOT '!' operation can only appear in condition expressions. \n"); break; }
    case 15: { printf("the actual parameter type is different from the formal parameter type defined by the function. \n"); break; }
    case 16: { printf("function parameter type cannot be void. \n"); break; }
    case 17: { printf("the return type is different from the return type defined by the function. \n"); break; }
    case 18: { printf("when lvalue is passed as a function argument, the reference of the array variable is illegal. \n"); break; }
    case 19: { printf("lvalue subscript out of bounds. \n"); break; }
    case 20: { printf("lvalue passed as a function argument here should be an array/constarray. \n"); break; }
    case 21: { printf("lvalue in assignment statements can only be a non-array variable, or locate an array element. \n"); break; }
    case 22: { printf("too many function arguments. \n"); break; }
    case 23: { printf("too many function arguments. \n"); break; }
    case 24: { printf("the function has't been defined. \n"); break; }
    case 25: { printf("missing function arguments. \n"); break; }
    default: { printf("undefined error. \n"); break; }
    }
}

void printType(List type)
{
    if (listsize(type) == 0)
    {
        printf("List is empty\n");
        return;
    }
    int blanknum = 0, j;
    ListItr i = getGListItr(type, 0);
    while (hasNext(i))
    {
        for (j = 0; j < blanknum; j++)
            printf(" ");
        Type tmp = nextItem(i);
        switch (tmp->kind)
        {
        case constant:
            printf("constant\n");
            break;
        case basic:
            printf("basic\n");
            break;
        case array:
            printf("array: %d\n", tmp->size);
            break;
        case constArray:
            printf("constArray: %d\n", tmp->size);
            break;
        }
        blanknum += 2;
    }
}

int myHash(char *key) //自定义hash函数
{
    int res = 0, p = 1, i;
    for (i = 0; i < strlen(key); i++)
    {
        res = (res + key[i] * p % HT_SIZE) % HT_SIZE;
        p = p * HT_SEED % HT_SIZE;
    }
    return res;
}

int isConst(List type)
{
    ListItr i = getGListItr(type, 0);
    Type tmp = nextItem(i);
    if (tmp->kind == constant)
        return 1;
    if (tmp->kind == constArray)
        return 1;
    return 0;
}

Type isNotArray(List type)
{
    ListItr i = getGListItr(type, 0);
    Type tmp = nextItem(i);
    if (tmp->kind == constant)
    {
        Type res;
        NEW0(res);
        res->kind = constant;
        return res;
    }
    if (tmp->kind == basic)
    {
        Type res;
        NEW0(res);
        res->kind = basic;
        return res;
    }
    return NULL;
}

int cmpType(List type1, List type2) // 判断两个类型是否完全一致，如const int-const int，int[2][3]-int[2][3]
{
    int res = 0;
    if (listsize(type1) != listsize(type2))
    {
        return 1;
    }
    int blanknum = 0, k;
    ListItr i = newListItr(type1, 0);
    ListItr j = newListItr(type2, 0);
    while (hasNext(i))
    {
        Type tmp1 = nextItem(i);
        Type tmp2 = nextItem(j);
        if (tmp1->kind != tmp2->kind)
        {
            res = 1;
            break;
        }
        if (tmp1->kind == array && tmp1->size != tmp2->size)
        {
            res = 1;
            break;
        }
        if (tmp1->kind == constArray && tmp1->size != tmp2->size)
        {
            res = 1;
            break;
        }
    }
    destroyListItr(&i);
    destroyListItr(&j);
    return res;
}

int cmpSingleTypeConst(Type t1, Type t2) // 后者可以是const的Type比较，没有比size
{
    switch (t1->kind)
    {
    case basic:
        if (t2->kind == basic || t2->kind == constant)
            return 0;
        break;
    case array:
        if (t2->kind == array || t2->kind == constArray)
            return 0;
        break;
    default:
        if (t2->kind == t1->kind)
            return 0;
        break;
    }
    return 1;
}

int cmpTypeConst(List type, List constType) // 后者可以是const的比较，如int=const int，int[2][3]=const int[2][3]
{
    int res = 0;
    if (listsize(type) != listsize(constType))
    {
        return 1;
    }
    ListItr i = newListItr(type, 0);
    ListItr j = newListItr(constType, 0);
    // ListItr i = getGListItr(type, 0);
    // ListItr j = getGListItr(constType, 0);
    while (hasNext(i))
    {
        Type tmp1 = nextItem(i);
        Type tmp2 = nextItem(j);
        if (cmpSingleTypeConst(tmp1, tmp2) > 0)
        {
            res = 1;
            break;
        }
        if (tmp1->kind == array && tmp1->size != tmp2->size)
        {
            res = 1;
            break;
        }
        if (tmp1->kind == constArray && tmp1->size != tmp2->size)
        {
            res = 1;
            break;
        }
    }
    return res;
}

int cmpTypeFunc(List funcType, List type) // 前者为函数形参，后者为函数实参，判断能否传入，如int-int，int[]-int[3]
{
    int res = 0;
    if (listsize(funcType) != listsize(type))
    {
        return 1;
    }
    int blanknum = 0, k;
    ListItr i = newListItr(funcType, 0);
    ListItr j = newListItr(type, 0);
    int flag = 0;
    while (hasNext(i))
    {
        Type tmp1 = nextItem(i);
        Type tmp2 = nextItem(j);
        if (cmpSingleTypeConst(tmp1, tmp2) > 0)
        {
            res = 1;
            break;
        }
        if (flag == 0)
            flag = 1;
        else
        { // 函数传参为数组时 从第二项才开始比较size
            if (tmp1->kind == array && tmp1->size != tmp2->size)
            {
                res = 1;
                break;
            }
            if (tmp1->kind == constArray && tmp1->size != tmp2->size)
            {
                res = 1;
                break;
            }
        }
    }
    destroyListItr(&i);
    destroyListItr(&j);
    return res;
}

List combineType(List defType, List callType)
{
    if (defType == NULL)
        return NULL;
    List resType = newList();
    Type def = isNotArray(defType);
    if (def != NULL) // 数值 其实用不到这部分
    {
        if (callType->size == 0)
        {
            handleError(30, 0);
            return defType;
        }
        int res = cmpSingleTypeConst((Type)getFirst(callType), (Type)getFirst(defType));
        if (res)
        {
            def->kind = empty;
            def->size = 0;
        } // error
        addFirst(resType, def);
    }
    else // array
    {
        if (callType->size == 0)
            return defType;
        // 举个例子 deftype是a[4][2][3]  list : 4 -> 2 -> 3
        // calltype a[1][2]   list : 1 -> 2
        // 从第一项开始判断 1合法 2不合法 返回错误类型empty
        // calltype a[1][1]   list : 1 -> 1
        // 判断 1 合法 1 合法 下一项没了 deftype还剩3 所以返回类型是 list : 3 检验到
        // 合法的时候再新建一个List返回
        if (listsize(defType) < listsize(callType))
        {
            Type error;
            NEW0(error);
            error->kind = empty;
            error->size = 1; // error
            addFirst(resType, error);
            return resType;
        }
        ListItr i = newListItr(defType, 0);
        ListItr j = newListItr(callType, 0);
        int count = listsize(defType);
        while (hasNext(j))
        {
            count--;
            Type tmp1 = nextItem(i); // def
            Type tmp2 = nextItem(j); // call
            if (tmp1->size <= tmp2->size)
            {
                Type error;
                NEW0(error);
                error->kind = empty;
                error->size = 2; // error
                addFirst(resType, error);
                destroyListItr(&i);
                destroyListItr(&j);
                return resType;
            }
        }
        if (count == 0)
        {
            Type tmp;
            NEW0(tmp);
            if (isConst(defType))
                tmp->kind = constant;
            else
                tmp->kind = basic;
            addFirst(resType, tmp);
        }
        else
        {
            while (hasNext(i))
            {
                Type tmp = nextItem(i);
                Type copyTmp;
                NEW0(copyTmp);
                copyTmp->size = tmp->size;
                copyTmp->kind = tmp->kind;
                addLast(resType, copyTmp);
            }
        }
        destroyListItr(&i);
        destroyListItr(&j);
    }
    return resType;
}

void initScopeTable()
{
    scope = 0;
    scopeTable = newVector();
    vector scope0 = newVector();
    addItem(scopeTable, scope0);
}

void insertScopeItem(listNode r)
{
    vector scope = (vector)(getLastItem(scopeTable));
    addItem(scope, r);
}

void addScope()
{
    scope++;
    vector newScope = newVector();
    addItem(scopeTable, newScope);
}

void exitScope()
{
    vector lastScope = (vector)(getLastItem(scopeTable));
    for (int i = 0; i < lastScope->size; i++)
    {
        listNode item = (listNode)getItem(lastScope, i);
        removeNode(item);
        item = NULL;
    }
    destoryVector(lastScope);
    removeLastItem(scopeTable);
    scope--;
}

void clearScope0()
{
    if (scope != 0)
    {
        printf("scope is not zero!\n");
        return;
    }
    vector lastScope = (vector)(getLastItem(scopeTable));
    for (int i = 0; i < lastScope->size; i++)
    {
        listNode item = (listNode)getItem(lastScope, i);
        removeNode(item);
        item = NULL;
    }
    destoryVector(lastScope);
    removeLastItem(scopeTable);
    for (int i = 0; i < HT_SIZE; i++)
    {
        if (paraTable[i] != NULL)
        {
            free(paraTable[i]->first);
            free(paraTable[i]);
            paraTable[i] = NULL;
        }
    }
}

void initSymTable()
{
    int i;
    for (i = 0; i < HT_SIZE; i++)
    {
        funcTable[i] = paraTable[i] = NULL;
    }
}

// int insertSymTable(treeNode *root)
// {
//     int pos = myHash(root->name); //hash表的下标
//     int flag = checkConflict(pos, root->name);
//     if (flag) //有冲突
//     {
//         return -1;
//     }
//     if (hashTable[pos] == NULL)
//     {
//         hashTable[pos] = newList();
//         addFirst(hashTable[pos], );
//     }
// }

int checkReserveNames(char *name)
{
    int i;
    for (i = 0; i < sizeof(RESERVE_WORDS) / sizeof(const char *); i++)
    {
        if (strcmp(name, RESERVE_WORDS[i]) == 0)
        {
            return 1;
        }
    }
    return 0;
}

Para querySymTable_para(char *name)
{
    int pos = myHash(name);
    List list = paraTable[pos];
    if (list == NULL || listsize(list) == 0)
        return NULL;
    ListItr i = getGListItr(list, 0);
    while (hasNext(i))
    {
        Para tmp = (Para)(nextItem(i));
        if (strcmp(tmp->name, name) == 0)
        {
            return tmp;
        }
    }
    return NULL;
}

int para_exist(char *name, int scope)
{
    int t = checkReserveNames(name);
    if (t != 0)
    {
        return 1; // handleError(11, 0);
    }
    Para query = querySymTable_para(name);
    if (query != NULL && query->scope == scope)
    {
        return 2; // handleError(12, 0);
    }
    return 0;
}

int insertSymTable_para(Para r, int lineno)
{
    int t = para_exist(r->name, r->scope);
    if (t == 1)
    {
        handleError(11, lineno);
        return 1;
    }
    if (t == 2)
    {
        handleError(12, lineno);
        return 1;
    }
    int pos = myHash(r->name);
    if (paraTable[pos] == NULL)
    {
        paraTable[pos] = newList();
    }
    addFirst(paraTable[pos], r);
    listNode n = getFirstNode(paraTable[pos]);
    insertScopeItem(n);
    return 0;
}

int sdtParseExp(treeNode *r, int isIdentConst, int flag, List list)
{
    return sdtParseAddExp(r->child, isIdentConst, flag, list, 0);
}

void sdtParseExp_rep(treeNode *r, List *type) // Exp_rep LB Exp RB | empty
{
    if (r->child == NULL) //Exp_rep -> empty
    {
        return;
    }
    else if (r->child != NULL) //Exp_rep -> Exp_rep LB Exp RB
    {
        treeNode *Exp = r->child->next->next;
        Type newType;
        NEW0(newType);
        newType->kind = array;
        newType->size = sdtParseExp(Exp, 0, 0, NULL);
        printf("%d\n", newType->size);
        if (newType->size < 0) //数组的维度必须是正数
        {
            handleError(4, r->lineNo);
            newType->size = 0;
        }
        addFirst(*type, newType);
        sdtParseExp_rep(r->child, type);
    }
}

int sdtParseLVal(treeNode *r, int isIdentConst, int flag, List list) // LVal : ID Exp_rep
{
    //查询符号表，若isIdentConst = 1，则Ident必须为Constant，否则可以是任意（在符号表中存在即可）
    // 若flag=1，说明是形参-数组的情况
    int res = 0;
    Para p = querySymTable_para(r->child->value);
    if (p == NULL)
    {
        handleError(7, r->lineNo); // not exist
        return 0;
    }
    List ptype = p->type;
    if (!isConst(ptype) && isIdentConst == 1) // 必须是const但ID不是const
    {
        handleError(9, r->lineNo);
    }
    else
    {
        if (flag == 0) // 数值
        {
            if (isNotArray(ptype) != NULL)
                return p->const_val[0];
            List type = newList();
            sdtParseExp_rep(r->child->next, &type);
            if (listsize(type) != listsize(ptype)) // LVal如果是取数组必须取到一个元素
            {
                handleError(21, r->lineNo);
            }
            ListItr i = newListItr(ptype, 0);
            ListItr j = newListItr(type, 0);
            int index = 0, factor = 1, flag1 = 0;
            while (hasNext(j))
            {
                Type tmp1 = nextItem(i); // def
                Type tmp2 = nextItem(j); // call
                if (tmp1->size <= tmp2->size)
                {
                    if (tmp1->size != -1 || flag1 != 0)
                    {
                        handleError(19, r->lineNo);
                        destroyListItr(&i);
                        destroyListItr(&j);
                        return 0;
                    }
                    else
                    {
                        flag1 = 2;
                    }
                }
                if (flag1 == 0)
                    flag1 = 1;
                else
                    factor *= tmp1->size;
                index = index + factor * tmp2->size;
            }
            destroyListItr(&i);
            destroyListItr(&j);
            if (flag1 != 2)
                res = p->const_val[index];
            else
                res = 0;
        }
        else // 形参-数组
        {
            if (isNotArray(ptype) != NULL)
            {
                handleError(20, r->lineNo); // 应该输入数组，但输入的是数值
            }
            List type = newList();
            sdtParseExp_rep(r->child->next, &type);
            List callType = combineType(ptype, type);
            ListItr i = getGListItr(callType, 0);
            Type tmp = nextItem(i);
            if (tmp->kind == empty)
            {
                handleError(18, r->lineNo); // 调用不合法，例如int a[2]调用的时候使用了a[2]
                res = 1;
            }
            *list = *callType;
        }
    }
    return res;
}

int sdtParseNumber(treeNode *r)
{
    return r->child->int_val;
}

int sdtParsePrimaryExp(treeNode *r, int isIdentConst, int flag, List list)
{
    if (strcmp(r->child->name, "LP") == 0) //PrimaryExp -> '(' Exp ')'
    {
        return sdtParseExp(r->child->next, isIdentConst, flag, list);
    }
    else if (strcmp(r->child->name, "LVal") == 0) //PrimaryExp -> LVal
    {
        return sdtParseLVal(r->child, isIdentConst, flag, list);
    }
    else //PrimaryExp -> Number
    {
        return sdtParseNumber(r->child);
    }
}

int sdtParseUnaryExp(treeNode *r, int isIdentConst, int flag, List list, int isCond)
{
    if (strcmp(r->child->name, "PrimaryExp") == 0) //UnaryExp -> PrimaryExp
    {
        return sdtParsePrimaryExp(r->child, isIdentConst, flag, list);
    }
    else if (strcmp(r->child->name, "UnaryOp") == 0) //UnaryExp -> UnaryOp UnaryExp
    {
        int a = sdtParseUnaryExp(r->child->next, isIdentConst, 0, NULL, isCond);
        if (strcmp(r->child->child->name, "PLUS") == 0)
        {
            return a;
        }
        else if (strcmp(r->child->child->name, "MINUS") == 0)
        {
            return -a;
        }
        else
        {
            if (isCond == 0)
                handleError(14, r->lineNo);
            return (a == 0);
        }
    }
    else //UnaryExp -> Ident '(' [FuncRParams] ')'
    {
        if (isIdentConst) //常量表达式里不可能出现函数，因为函数的返回值是变量
        {
            handleError(3, r->lineNo);
        }
        else
        {
            //判断Ident是函数，并且返回值是int变量，否则报错
            Func f = querySymTable_func(r->child->value);
            if (f == NULL)
            {
                handleError(24, r->lineNo);
                return 0;
            }
            // 解析FuncRParams
            if (r->child->next->next->next == NULL)
            {
                if (f->paraNum != 0)
                    handleError(25, r->lineNo);
            }
            else if (f->paraNum == 0)
                handleError(22, r->lineNo);
            else
            {
                sdtParseFuncRParams(r->child->next->next, f->paraList);
            }
        }
    }
}

int sdtParseMulExp(treeNode *r, int isIdentConst, int flag, List list, int isCond)
{
    if (strcmp(r->child->name, "UnaryExp") == 0) //MulExp -> UnaryExp
    {
        return sdtParseUnaryExp(r->child, isIdentConst, flag, list, isCond);
    }
    else //MulExp -> MulExp ('*'|'/'|'%') UnaryExp
    {
        int a = sdtParseMulExp(r->child, isIdentConst, 0, NULL, isCond);
        int b = sdtParseUnaryExp(r->child->next->next, isIdentConst, 0, NULL, isCond); // 应该是这样？
        if (strcmp(r->child->next->name, "STAR") == 0)                                 //乘法
        {
            return a * b;
        }
        else
        {
            if (b == 0) //除运算和模运算，被除数不能是0
            {
                handleError(2, r->lineNo);
            }
            else if (strcmp(r->child->next->name, "DIV") == 0)
            {
                return a / b;
            }
            else
            {
                return a % b;
            }
        }
    }
}

int sdtParseAddExp(treeNode *r, int isIdentConst, int flag, List list, int isCond)
{
    if (strcmp(r->child->name, "MulExp") == 0) //AddExp -> MulExp
    {
        return sdtParseMulExp(r->child, isIdentConst, flag, list, isCond);
    }
    else //AddExp -> AddExp ('+'|'-') MulExp
    {
        int a = sdtParseAddExp(r->child, isIdentConst, 0, NULL, isCond);
        int b = sdtParseMulExp(r->child->next->next, isIdentConst, 0, NULL, isCond);
        if (strcmp(r->child->child->name, "PLUS") == 0)
        {
            return a + b;
        }
        else
        {
            return a - b;
        }
    }
}

int sdtParseConstExp(treeNode *r)
{
    return sdtParseAddExp(r->child, 1, 0, NULL, 0); //ConstExp -> AddExp
}

void sdtParseComConstDef_rep(treeNode *r) //ComConstDef_rep	-> ComConstDef_rep COMMA ConstDef | empty
{
    if (r == NULL || r->child == NULL)
        return;
    sdtParseComConstDef_rep(r->child);
    sdtParseConstDef(r->child->next->next);
}

void sdtParseConstDecl(treeNode *r) //ConstDecl : CONST FuncType ConstDef ComConstDef_rep SEMI
{
    if (r == NULL)
        return;
    if (strcmp(r->name, "ConstDecl") == 0)
    {
        if (strcmp(r->child->next->child->name, "VOID") == 0) //常量类型不能是void
        {
            handleError(1, r->lineNo);
            return;
        }
        sdtParseConstDef(r->child->next->next);
        sdtParseComConstDef_rep(r->child->next->next->next);
    }
}

void sdtParseConstExp_rep(treeNode *r, List *type)
{
    if (r->child == NULL) //constExp_rep -> empty
    {
        return;
    }
    else if (r->child != NULL) //constExp_rep -> constExp_rep LB ConstExp RB
    {
        treeNode *constExp = r->child->next->next;
        Type newType;
        NEW0(newType);
        newType->kind = constArray;
        newType->size = sdtParseConstExp(constExp);
        printf("%d\n", newType->size);
        if (newType->size <= 0) //数组的维度必须是正数
        {
            handleError(4, r->lineNo);
            newType->size = 1;
        }
        addFirst(*type, newType);
        sdtParseConstExp_rep(r->child, type);
    }
}

//返回r之后最近的一个'{', '}', ConstExp或者Exp
treeNode *getNextToken(treeNode *r)
{
    if (r == NULL)
        return NULL;
    //r=r->next;
    while (r != NULL)
    {
        if (strcmp(r->name, "LC") == 0 || strcmp(r->name, "RC") == 0 || strcmp(r->name, "ConstExp") == 0 || strcmp(r->name, "Exp") == 0)
        {
            return r;
        }
        else if (strcmp(r->name, "ConstInitVal") == 0 || strcmp(r->name, "ComConstInitVal_rep") == 0 || strcmp(r->name, "InitVal") == 0 || strcmp(r->name, "ComInitVal_rep") == 0)
        {
            if (r->child != NULL)
                return getNextToken(r->child);
        }
        if (r->next != NULL)
        {
            r = r->next;
        }
        else
        {
            while (r)
            {
                r = r->father;
                if (r->next != NULL)
                {
                    r = r->next;
                    break;
                }
            }
        }
    }
}

int getArraySize(int *a, int index, int size)
{
    int arraySize = 1;
    for (; index < size; index++)
        arraySize *= a[index];
    return arraySize;
}

void getArrayDimList(int *a, Para para)
{
    ListItr i = getGListItr(para->type, 0);
    int size = 0;
    while (hasNext(i))
    {
        a[size++] = ((Type)(nextItem(i)))->size;
    }
}

treeNode *sdtParseConstInitVal_Array(treeNode *r, Para *para, int *a, int arrayDim, int index, int paraCount, int *numCount, int numCountLim)
{
    /*
        ConstInitVal : ConstExp | LC ConstInitVal ComConstInitVal_rep RC | LC RC
        ComConstInitVal_rep : ComConstInitVal_rep COMMA ConstInitVal | empty
    */
    printf("sdtParseConstInitVal_Array(arrayDim=%d, index=%d, paraCount=%d, numCount=%d, numCountLim=%d\n",
           arrayDim, index, paraCount, *numCount, numCountLim);
    if (r == NULL)
        return NULL;
    while (1)
    {
        if (r->next != NULL)
        {
            r = r->next;
        }
        else
        {
            while (r)
            {
                r = r->father;
                if (r->next != NULL)
                {
                    r = r->next;
                    break;
                }
            }
        }
        if (r == NULL)
            return NULL;
        r = getNextToken(r);
        printf("nextToken: %s\n", r->name);
        if (strcmp(r->name, "LC") == 0)
        {
            if (index + 1 >= arrayDim) //大括号嵌套层数超过数组维数
            {
                handleError(8, r->lineNo);
            }
            if ((*numCount) % getArraySize(a, index + 1, arrayDim) != 0) //如果当前维度没有填满，需要隐式初始化
            {
                int dim = getArraySize(a, index + 1, arrayDim);
                (*numCount) += dim - (*numCount) % dim;
            }
            if ((*numCount) >= numCountLim) //同一层大括号的数量超过了这一层的维数
            {
                handleError(10, r->lineNo);
            }
            r = sdtParseConstInitVal_Array(r, para, a, arrayDim, index + 1, paraCount + 1, numCount, *numCount + getArraySize(a, index + 1, arrayDim));
        }
        else if (strcmp(r->name, "RC") == 0)
        {
            if ((*numCount) % getArraySize(a, index, arrayDim) != 0) //当前维没填满
            {
                (*numCount) = numCountLim;
            }
            (*numCount) = numCountLim;
            return r;
        }
        else if (strcmp(r->name, "ConstExp") == 0)
        {
            int res = sdtParseConstExp(r);
            printf("res=%d\n", res);
            if (*numCount >= numCountLim) //常量数量超过当前维限制
            {
                handleError(10, r->lineNo);
            }
            (*para)->const_val[*numCount] = res;
            *numCount = *numCount + 1;
        }
    }
}

void sdtParseConstInitVal(treeNode *r, Para *para)
{
    /*
        ConstInitVal : ConstExp | LC ConstInitVal ComConstInitVal_rep RC | LC RC
        ComConstInitVal_rep : ComConstInitVal_rep COMMA ConstInitVal | empty
    */
    Type t = getFirst((*para)->type);
    if (t->kind == constant) //常量的初始化必须是一个单一int值
    {
        if (strcmp(r->child->name, "ConstExp") == 0)
        {
            (*para)->const_val = (int *)malloc(sizeof(int));
            (*para)->const_val[0] = sdtParseConstExp(r->child);
        }
        else
        {
            handleError(5, r->lineNo);
        }
    }
    else // constArray
    {
        int arrayDim = listsize((*para)->type);
        printf("arrayDim: %d\n", arrayDim);
        int *arrayDimList = (int *)malloc(sizeof(int) * arrayDim);
        getArrayDimList(arrayDimList, *para);
        printf("arrayDimList: {");
        int i;
        for (i = 0; i < arrayDim; i++)
            printf("%d,", arrayDimList[i]);
        printf("}\n");
        int arraySize = getArraySize(arrayDimList, 0, arrayDim);
        printf("arraySize: %d\n", arraySize);
        (*para)->const_val = (int *)malloc(sizeof(int) * arraySize);
        memset((*para)->const_val, 0, sizeof((*para)->const_val));
        if (strcmp(r->child->next->name, "RC") == 0) //仅一个{}，全部初始化为0，直接返回
        {
            printf("All zero\n");
            return;
        }
        else if (strcmp(r->child->name, "ConstExp") == 0)
        {
            handleError(6, r->lineNo);
        }
        else
        {
            int tmp = 0;
            sdtParseConstInitVal_Array(r->child, para, arrayDimList, arrayDim, 0, 1, &tmp, arraySize);
            int i;
            printf("{");
            for (i = 0; i < arraySize; i++)
                printf("%d ", (*para)->const_val[i]);
            printf("}\n");
        }
        free(arrayDimList);
    }
}

void sdtParseConstDef(treeNode *r)
{
    if (r == NULL)
        return;
    if (strcmp(r->name, "ConstDef") == 0) //ConstDef -> ID ConstExp_rep ASSIGNOP ConstInitVal
    {
        Para item;
        NEW0(item);
        item->type = newList();
        item->scope = scope;
        strcpy(item->name, r->child->value);
        if (r->child->next->child == NULL) //ID is a constant
        {
            Type newType;
            NEW0(newType);
            newType->kind = constant;
            addFirst(item->type, newType);
            sdtParseConstInitVal(r->child->next->next->next, &item);
            int tmp = insertSymTable_para(item, r->lineNo);
            if (!tmp)
                printf("constant: %d\n", item->const_val[0]);
        }
        else // ID is a const array
        {
            //printf("constDef: constArray\n");
            sdtParseConstExp_rep(r->child->next, &item->type);
            //printType(item->type);
            sdtParseConstInitVal(r->child->next->next->next, &item);
            int tmp = insertSymTable_para(item, r->lineNo);
            if (!tmp)
            {
                printf("constDef: constArray\n");
                printType(item->type);
            }
        }
    }
}

void sdtParseCompUnit(treeNode *r)
{
    if (strcmp(r->child->name, "Decl") == 0) // Decl
    {
        sdtParseDecl(r->child);
    }
    else if (strcmp(r->child->name, "FuncDef") == 0) // FuncDef
    {
        sdtParseFuncDef(r->child);
    }
    else if (strcmp(r->child->next->name, "Decl") == 0) // CompUnit Decl
    {
        sdtParseCompUnit(r->child);
        sdtParseDecl(r->child->next);
    }
    else if (strcmp(r->child->next->name, "FuncDef") == 0) // CompUnit FuncDef
    {
        sdtParseCompUnit(r->child);
        sdtParseFuncDef(r->child->next);
    }
}

void sdtParseDecl(treeNode *r) // Decl -> ConstDecl | VarDecl
{
    if (r == NULL)
        return;
    if (strcmp(r->name, "Decl") == 0)
    {
        if (strcmp(r->child->name, "ConstDecl") == 0)
        {
            sdtParseConstDecl(r->child);
        }
        else if (strcmp(r->child->name, "VarDecl") == 0)
        {
            sdtParseVarDecl(r->child);
        }
    }
}

void sdtParseVarDecl(treeNode *r) // VarDecl -> FuncType(not void) VarDef VarDef_rep SEMI
{
    if (r == NULL)
        return;
    if (strcmp(r->name, "VarDecl") == 0)
    {
        if (strcmp(r->child->child->name, "VOID") == 0) //变量类型不能是void
        {
            handleError(1, r->lineNo);
            return;
        }
        sdtParseVarDef(r->child->next);
        sdtParseComVarDef_rep(r->child->next->next);
    }
}

void sdtParseVarDef(treeNode *r) // VarDef -> ID ConstExp_rep | ID ConstExp_rep ASSIGNOP InitVal
{
    if (r == NULL)
        return;
    if (strcmp(r->name, "VarDef") == 0)
    {
        Para item;
        NEW0(item);
        item->type = newList();
        item->scope = scope;
        strcpy(item->name, r->child->value);
        if (r->child->next->child == NULL) //ID is a variable
        {
            Type newType;
            NEW0(newType);
            newType->kind = basic;
            addFirst(item->type, newType);
            if (r->child->next->next != NULL)
            {
                sdtParseInitVal(r->child->next->next->next, &item);
            }
            else
            {
                item->const_val = (int *)malloc(sizeof(int));
                item->const_val[0] = 0;
            }
            int tmp = insertSymTable_para(item, r->lineNo);
            if (!tmp)
                printf("basic: %d\n", item->const_val[0]);
        }
        else // ID is an array
        {
            //printf("VarDef: VarArray\n");
            sdtParseConstExp_rep(r->child->next, &item->type);
            //printType(item->type);
            if (r->child->next->next != NULL)
            {
                sdtParseInitVal(r->child->next->next->next, &item);
            }
            else
            {
                sdtParseInitVal(NULL, &item);
            }
            int tmp = insertSymTable_para(item, r->lineNo);
            if (!tmp)
            {
                printf("VarDef: VarArray\n");
                printType(item->type);
            }
        }
    }
}

void sdtParseComVarDef_rep(treeNode *r) // ComVarDef_rep -> ComVarDef_rep COMMA VarDef | empty
{
    if (r == NULL || r->child == NULL)
        return;
    sdtParseComVarDef_rep(r->child);
    sdtParseVarDef(r->child->next->next);
}

void sdtParseInitVal(treeNode *r, Para *para) // InitVal -> Exp | LC InitVal ComInitVal_rep RC | LC RC
{                                             // ComInitVal_rep -> ComInitVal_rep COMMA InitVal | empty
    Type t = getFirst((*para)->type);
    if (t->kind == basic)
    {
        if (strcmp(r->child->name, "Exp") == 0)
        {
            (*para)->const_val = (int *)malloc(sizeof(int));
            (*para)->const_val[0] = sdtParseExp(r->child, 0, 0, NULL);
        }
        else
        {
            handleError(5, r->lineNo);
        }
    }
    else // array
    {
        int arrayDim = listsize((*para)->type);
        printf("arrayDim: %d\n", arrayDim);
        int *arrayDimList = (int *)malloc(sizeof(int) * arrayDim);
        getArrayDimList(arrayDimList, *para);
        printf("arrayDimList: {");
        int i;
        for (i = 0; i < arrayDim; i++)
            printf("%d,", arrayDimList[i]);
        printf("}\n");
        int arraySize = getArraySize(arrayDimList, 0, arrayDim);
        printf("arraySize: %d\n", arraySize);
        (*para)->const_val = (int *)malloc(sizeof(int) * arraySize);
        memset((*para)->const_val, 0, sizeof((*para)->const_val));
        if (r == NULL || strcmp(r->child->next->name, "RC") == 0) //仅一个{}，全部初始化为0，直接返回
        {
            printf("All zero\n");
            return;
        }
        else if (strcmp(r->child->name, "Exp") == 0)
        {
            handleError(6, r->lineNo);
        }
        else
        {
            int tmp = 0;
            sdtParseInitVal_Array(r->child, para, arrayDimList, arrayDim, 0, 1, &tmp, arraySize);
            int i;
            printf("{");
            for (i = 0; i < arraySize; i++)
                printf("%d ", (*para)->const_val[i]);
            printf("}\n");
        }
    }
}

treeNode *sdtParseInitVal_Array(treeNode *r, Para *para, int *a, int arrayDim, int index, int paraCount, int *numCount, int numCountLim)
{
    // InitVal -> Exp | LC InitVal ComInitVal_rep RC | LC RC
    // ComInitVal_rep -> ComInitVal_rep COMMA InitVal | empty
    printf("sdtParseInitVal_Array(arrayDim=%d, index=%d, paraCount=%d, numCount=%d, numCountLim=%d\n",
           arrayDim, index, paraCount, *numCount, numCountLim);
    if (r == NULL)
        return NULL;
    while (1)
    {
        if (r->next != NULL)
        {
            r = r->next;
        }
        else
        {
            while (r)
            {
                r = r->father;
                if (r->next != NULL)
                {
                    r = r->next;
                    break;
                }
            }
        }
        if (r == NULL)
            return NULL;
        r = getNextToken(r);
        printf("nextToken: %s\n", r->name);
        if (strcmp(r->name, "LC") == 0)
        {
            if (index + 1 >= arrayDim) //大括号嵌套层数超过数组维数
            {
                handleError(8, r->lineNo);
            }
            if ((*numCount) % getArraySize(a, index + 1, arrayDim) != 0) //如果当前维度没有填满，需要隐式初始化
            {
                int dim = getArraySize(a, index + 1, arrayDim);
                (*numCount) += dim - (*numCount) % dim;
            }
            if ((*numCount) >= numCountLim) //同一层大括号的数量超过了这一层的维数
            {
                handleError(10, r->lineNo);
            }
            r = sdtParseInitVal_Array(r, para, a, arrayDim, index + 1, paraCount + 1, numCount, *numCount + getArraySize(a, index + 1, arrayDim));
        }
        else if (strcmp(r->name, "RC") == 0)
        {
            if ((*numCount) % getArraySize(a, index, arrayDim) != 0) //当前维没填满
            {
                (*numCount) = numCountLim;
            }
            (*numCount) = numCountLim;
            return r;
        }
        else if (strcmp(r->name, "ConstExp") == 0 || strcmp(r->name, "Exp") == 0)
        {
            int res = sdtParseExp(r, 0, 0, NULL);
            printf("res=%d\n", res);
            if (*numCount >= numCountLim) //常量数量超过当前维限制
            {
                handleError(10, r->lineNo);
            }
            (*para)->const_val[*numCount] = res;
            *numCount = *numCount + 1;
        }
    }
}

void sdtParseFuncDef(treeNode *r) // FuncDef -> FuncType ID LP FuncFParams RP Block
{
    Func f;
    NEW0(f);
    strcpy(f->name, r->child->next->value);
    Type newType;
    NEW0(newType);
    addScope(); // 函数定义的参数表需要在符号表里面
    if (strcmp(r->child->next->next->next->name, "FuncFParams") == 0)
    { // FuncDef -> FuncType ID LP FuncFParams RP Block
        vector list = sdtParseFuncFParams(r->child->next->next->next);
        f->paraList = list;
        f->paraNum = list->size;
        if (strcmp(r->child->child->name, "VOID") == 0)
        {
            newType->kind = empty;
            f->retType = newType;
            insertSymTable_func(f, r->lineNo); // 函数内部可以调用自己 所以要在block前面插入符号表
            sdtParseBlock(r->child->next->next->next->next->next, 2, 1);
        }
        else
        {
            newType->kind = basic;
            f->retType = newType;
            insertSymTable_func(f, r->lineNo);
            sdtParseBlock(r->child->next->next->next->next->next, 1, 1);
        }
    }
    else // FuncType ID LP RP Block
    {
        f->paraList = NULL;
        f->paraNum = 0;
        if (strcmp(r->child->child->name, "VOID") == 0)
        {
            newType->kind = empty;
            f->retType = newType;
            insertSymTable_func(f, r->lineNo);
            sdtParseBlock(r->child->next->next->next->next, 2, 1);
        }
        else
        {
            newType->kind = basic;
            f->retType = newType;
            insertSymTable_func(f, r->lineNo);
            sdtParseBlock(r->child->next->next->next->next, 1, 1);
        }
    }
    exitScope();
    //insertSymTable_func(f);
}

vector sdtParseFuncFParams(treeNode *r) // FuncFParams -> FuncFParam ComFuncFParam_rep
{
    vector paraList = newVector();
    sdtParseFuncFParam(r->child, paraList);
    sdtParseComFuncFParam_rep(r->child->next, paraList);
    return paraList;
}

void sdtParseComFuncFParam_rep(treeNode *r, vector paraList) // ComFuncFParam_rep : ComFuncFParam_rep COMMA FuncFParam | empty
{
    if (r == NULL || r->child == NULL)
        return;
    sdtParseComFuncFParam_rep(r->child, paraList);
    sdtParseFuncFParam(r->child->next->next, paraList);
}

void sdtParseFuncFParam(treeNode *r, vector paraList) // FuncFParam -> FuncType (not void) ID
{                                                     // | FuncType (not void) ID LB RB LbEXPRb_rep
    Para item;
    NEW0(item);
    item->type = newList();
    item->scope = scope;
    strcpy(item->name, r->child->next->value);
    if (strcmp(r->child->child->name, "VOID") == 0)
    {
        handleError(16, r->lineNo);
    }
    else if (r->child->next->next != NULL)
    {
        // array 加入paraList和scope
        sdtParseLbEXPRb_rep(r->child->next->next->next->next, &item->type);
        Type newType;
        NEW0(newType);
        newType->kind = array;
        newType->size = -1; // []内任意size都可以
        addFirst(item->type, newType);
        int tmp = insertSymTable_para(item, r->lineNo);
        if (!tmp)
        {
            addItem(paraList, item);
            printf("function array param: %s\n", item->name);
        }
    }
    else
    {
        // not array 加入paraList和scope
        Type newType;
        NEW0(newType);
        newType->kind = basic;
        addFirst(item->type, newType);
        item->const_val = (int *)malloc(sizeof(int));
        item->const_val[0] = 0;
        int tmp = insertSymTable_para(item, r->lineNo);
        if (!tmp)
        {
            addItem(paraList, item);
            printf("function int param: %s\n", item->name);
        }
    }
}

void sdtParseLbEXPRb_rep(treeNode *r, List *type) // -> LbEXPRb_rep LB Exp RB | empty
{
    if (r == NULL || r->child == NULL)
        return;
    treeNode *Exp = r->child->next->next;
    Type newType;
    NEW0(newType);
    newType->kind = array;
    newType->size = sdtParseExp(Exp, 0, 0, NULL);
    if (newType->size < 0) //数组的维度必须是正数
    {
        handleError(4, r->lineNo);
        newType->size = 0;
    }
    addFirst(*type, newType);
    sdtParseLbEXPRb_rep(r->child, type);
}

void sdtParseBlock(treeNode *r, int flag, int noAdd) // Block -> LC BlockItem_rep RC
{                                                    // flag = 1: return int | flag = 2: return void | flag = 0: no return
    if (!noAdd)
        addScope();
    sdtParseBlockItem_rep(r->child->next, flag);
    if (!noAdd)
        exitScope();
}

void sdtParseBlockItem_rep(treeNode *r, int flag) // BlockItem_rep : BlockItem_rep BlockItem | empty
{
    if (r == NULL || r->child == NULL)
        return;
    sdtParseBlockItem_rep(r->child, flag);
    sdtParseBlockItem(r->child->next, flag);
}

void sdtParseBlockItem(treeNode *r, int flag) // BlockItem -> Decl | Stmt
{                                             // flag = 1: return int | flag = 2: return void | flag = 0: no return
    if (strcmp(r->child->name, "Decl") == 0)
    {
        return sdtParseDecl(r->child);
    }
    else if (strcmp(r->child->name, "Stmt") == 0)
    {
        return sdtParseStmt(r->child, flag);
    }
}

void sdtParseStmt(treeNode *r, int flag)
{                                             // flag = 1: return int | flag = 2: return void | flag = 0: no return
    if (strcmp(r->child->name, "Block") == 0) // Block
    {
        sdtParseBlock(r->child, flag, 0);
    }
    else if (strcmp(r->child->name, "LVal") == 0) // LVal ASSIGNOP Exp SEMI
    {
        sdtParseLVal(r->child, 0, 0, NULL);
        sdtParseExp(r->child->next->next, 0, 0, NULL);
    }
    else if (strcmp(r->child->name, "Exp") == 0) // Exp SEMI
    {
        sdtParseExp(r->child, 0, 0, NULL);
    }
    else if (strcmp(r->child->name, "SEMI") == 0) // SEMI
    {
        return;
    }
    else if (strcmp(r->child->name, "IF") == 0) // IF LP Cond RP Stmt [ELSE Stmt]
    {
        sdtParseCond(r->child->next->next);
        sdtParseStmt(r->child->next->next->next->next, flag);
        if (r->child->next->next->next->next->next != NULL)
        {
            sdtParseStmt(r->child->next->next->next->next->next->next, flag);
        }
    }
    else if (strcmp(r->child->name, "WHILE") == 0) // WHILE LP Cond RP Stmt
    {
        whileCnt++;
        sdtParseCond(r->child->next->next);
        sdtParseStmt(r->child->next->next->next->next, flag);
        whileCnt--;
    }
    else if (strcmp(r->child->name, "BREAK") == 0) // BREAK SEMI
    {
        if (whileCnt <= 0)
        {
            handleError(13, r->lineNo);
        }
        return;
    }
    else if (strcmp(r->child->name, "CONTINUE") == 0) // CONTINUE SEMI
    {
        if (whileCnt <= 0)
        {
            handleError(13, r->lineNo);
        }
        return;
    }
    else if (strcmp(r->child->name, "RETURN") == 0) // RETURN SEMI | RETURN Exp SEMI
    {
        if (strcmp(r->child->next->name, "SEMI") == 0)
        {
            if (flag != 2)
                handleError(17, r->lineNo);
        }
        else
        {
            if (flag != 1)
                handleError(17, r->lineNo);
        }
    }
}

void sdtParseExpRParams(treeNode *r, vector v, int i) // addExp
{
    if (v->size <= i)
    {
        handleError(23, r->lineNo);
    }
    Para p = getItem(v, i);
    List type = p->type;
    if (isNotArray(type) != NULL)
    {
        // number 如果addExp在解析过程中发现不是number会自行报错
        sdtParseAddExp(r->child, 0, 0, NULL, 0);
    }
    else
    {
        // array
        List calltype;
        NEW0(calltype);
        sdtParseAddExp(r->child, 0, 1, calltype, 0);
        int t = cmpTypeFunc(type, calltype);
        if (t)
        {
            handleError(15, r->lineNo);
        }
    }
}

void sdtParseComExp_repRParams(treeNode *r, vector v, int j) // comExp_rep COMMA Exp | empty
{
    if (r == NULL || r->child == NULL)
    {
        if (j != 0)
            handleError(25, r->lineNo);
        return;
    }
    else if (j == 0)
    {
        handleError(22, r->lineNo);
        return;
    }
    sdtParseComExp_repRParams(r->child, v, j - 1);
    sdtParseExpRParams(r->child->next->next, v, j);
}

void sdtParseFuncRParams(treeNode *r, vector paraList) // Exp comExp_rep
{
    if (r == NULL || r->child == NULL)
    {
        if (paraList == NULL || paraList->size == 0)
            return;
        else
            handleError(25, r->lineNo);
        return;
    }
    sdtParseExpRParams(r->child, paraList, 0);
    sdtParseComExp_repRParams(r->child->next, paraList, paraList->size - 1);
    return;
}

int sdtParseCond(treeNode *r) // LOrExp
{
    return sdtParseLOrExp(r->child);
}

int sdtParseLOrExp(treeNode *r) // LAndExp | LOrExp OR LAndExp
{
    if (r->child->next == NULL)
    {
        return sdtParseLAndExp(r->child);
    }
    else
    {
        int i = sdtParseLOrExp(r->child);
        int j = sdtParseLAndExp(r->child->next->next);
        return i || j;
    }
}

int sdtParseLAndExp(treeNode *r) // EqExp | LAndExp AND EqExp
{
    if (r->child->next == NULL)
    {
        return sdtParseEqExp(r->child);
    }
    else
    {
        int i = sdtParseLAndExp(r->child);
        int j = sdtParseEqExp(r->child->next->next);
        return i && j;
    }
}

int sdtParseEqExp(treeNode *r) // RelExp | EqExp EQ RelExp | EqExp NEQ RelExp
{
    if (r->child->next == NULL)
    {
        return sdtParseRelExp(r->child);
    }
    else
    {
        int i = sdtParseEqExp(r->child);
        int j = sdtParseRelExp(r->child->next->next);
        if (strcmp(r->child->next->name, "EQ") == 0)
            return i == j;
        else
            return i != j;
    }
}

int sdtParseRelExp(treeNode *r) // AddExp | RelExp LT/GT/LTE/GTE AddExp
{
    if (r->child->next == NULL)
    {
        return sdtParseAddExp(r->child, 0, 0, NULL, 1);
    }
    else
    {
        int i = sdtParseRelExp(r->child);
        int j = sdtParseAddExp(r->child->next->next, 0, 0, NULL, 1);
        if (strcmp(r->child->next->name, "LT") == 0)
            return i < j;
        if (strcmp(r->child->next->name, "GT") == 0)
            return i > j;
        if (strcmp(r->child->next->name, "LTE") == 0)
            return i <= j;
        else
            return i >= j;
    }
}

/* print the syntax tree */
int sdtParse(treeNode *r)
{
    initScopeTable();
    sdtParseCompUnit(r);
    clearScope0();
    return errorcnt;
}

Func querySymTable_func(char *name)
{
    int pos = myHash(name);
    List list = funcTable[pos];
    if (list == NULL || listsize(list) == 0)
        return NULL;
    ListItr i = getGListItr(list, 0);
    while (hasNext(i))
    {
        Func tmp = (Func)(nextItem(i));
        if (strcmp(tmp->name, name) == 0)
        {
            return tmp;
        }
    }
    return NULL;
}

int func_exist(char *name)
{
    int t = checkReserveNames(name);
    if (t != 0)
    {
        return 1; // handleError(11, 0);
    }
    Func res = querySymTable_func(name);
    if (res != NULL)
    {
        return 2; // handleError(12, 0);
    }
    return 0;
}

int insertSymTable_func(Func r, int lineno)
{
    int t = func_exist(r->name);
    if (t == 1)
    {
        handleError(11, lineno);
        return 1;
    }
    if (t == 2)
    {
        handleError(12, lineno);
        return 1;
    }
    int pos = myHash(r->name);
    if (funcTable[pos] == NULL)
    {
        funcTable[pos] = newList();
    }
    addFirst(funcTable[pos], r);
    return 0;
}