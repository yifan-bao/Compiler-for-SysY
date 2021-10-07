#include"visast.h"
// reference: http://tang3w.com/2015/02/01/抽象语法树的可视化.html
/* 用法: 在编译时加入visast.c, 并传入构建好的语法树的根即调用COSStylePrintAstAsDot(root),
 *      之后会生成.dot文件, 用Graphviz进行解析即可 (需要下载Graphviz/在线解析网站https://dreampuf.github.io/GraphvizOnline/)
 */
char FILENAME[] = "./visAST/ast.dot";

void COSStylePrintAstNodes(treeNode* ast,FILE* f){
    if (ast == NULL) return;
    fprintf(f,"_%p[label=%s]\n", ast, ast->name);
    treeNode* node = ast->child;
    
    vector children = newVector();
    //treeNode* children[100];
    int count = 0;
    while(node!=NULL) {
        fprintf(f,"_%p -> _%p\n", ast, node);
        //children[count++] = node;
        addItem(children,node);
        node = node->next;
    }
    //printf("vector size is: %d\n",getSize(children));
    count = getSize(children);
    for(int i = 0; i < count; i++) {
        COSStylePrintAstNodes((treeNode*)getItem(children,i),f);
    }
}

void COSStylePrintAstAsDot(treeNode *ast) {
    FILE* f = fopen(FILENAME,"w");
    fprintf(f,"hello");
    fprintf(f,"digraph G {\n");
    fprintf(f,"node[shape=rect]\n");

    COSStylePrintAstNodes(ast,f);

    fprintf(f,"}");
    fclose(f);
}


