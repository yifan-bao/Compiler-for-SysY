/* 多行注释测试 */
/**
 * @file    syntaxRight.c
 * @note    这个文件测试SysY语言的所有文法要素, 用于检查生成的文法分析器的正确性。
 *          最终的结果是能够正确解析该文件并输出语法书。有关测试内容在下面给出注释。
 */
/* CompUnit */
/* 全局声明 */
int a;
/* 全局声明与定义初始化 */
int b = 0; 

/* 连续声明与定义初始化 */
int ch = 5, d = 1, e;
/* 多维数组声明 */
int thisIsAnArray[19][20][21];
/* 常量的全局声明 */
const int aa = 2;
const int brr[4] = {1,2,3,4};
const int z[2][2] = {{5}, 8, 9};
/* Function 2, void Return Value, void Parameter */
/* 函数2 返回void 参数也是空*/
void HelloWorld() {
    /* 库函数调用-参见SysY运行时库 */
    //printf("Hello, SysY!");
    
    /* 空返回 */
    return;
}
/* 函数3 多个参数 有int类型返回值 */
int max(int a, int b) {
    // return exp
    return  a;
}
// 函数 1
int main() {
    /* 局部变量声明与初始化 */
    int i=0, j, t, controller = 1, condition = 0;
    /**
     *  @note   Statements, Control Flows, Scopes
     **/
    /* 空循环 */
    while (1);
    while (1) { } // 空语句块

    /* 嵌套循环 */
    while ( 1 ) {
        while ( controller ) {
            controller = controller+1;
            /* If-else 语句 */
            if ( controller < 10) 
                continue; // continue语句
            else {
                HelloWorld();
                break;    // break语句
            }
        }
        
    }    
    /* else 悬挂 */
    if ( 1 ) 
        if ( 2 ) i = 333;
        else i = 955;
    else if ( 3 ) i = 666;
    else i = 233;
    /* 语句块嵌套 */
    t = -5;
    {   int t = 6;
        {   int t = 10;
            {   int t = 0;
                t = t * 10;  // 乘除加减模运算 + 赋值
                t = t / 10;
                t = t + 10;
                t = t - 10;
                t = t % 5;
            }
        }
    }
    /* 空 */
    ;;;;;;    
    /**
     *  @note   表达式
     **/

    /* 运算符 */
   
    // 逻辑运算 SysY语言中逻辑运算必须在Cond或while语句中
    if (1 && 2) {
        if (3 || 0) {
            // 比较运算符
            while( i !=0 && j >= 0 ) {
                i = i - 1;
                j = j - 1;
            }
        }
    }
    // 其他的比较运算符
    if (a > 2)
        if (a <10)
            if(a >= 5)
                if(a <= 8)
                    a = 4;
    
    // 单目运算符
    thisIsAnArray[0][0][1] = max(1,2);
    thisIsAnArray[0][0][1];
    -thisIsAnArray[0][0][1];
    (thisIsAnArray[0][0][1]);
    (3+4 * thisIsAnArray[0][0][1]);
   
    /* 运算符优先级 */ 
    // SysY语言文法本身就体现了优先级和结合性
    if(0 && 1) {
        i = 0;
    }
    if (1 || 0) {
        i = 2 - 1 * -1;
    }
    return 0;
}
