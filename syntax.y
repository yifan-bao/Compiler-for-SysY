%{
    #include "node.h"
	#include "symtable.h"
	#include "intercode.h"
	#include "codegen.h"
    // #include "lex.yy.c"
    treeNode* root;
	extern int yylineno;
	int yylex(void);
	int yyerror(char* msg);
	int yyrestart(FILE* f);
%}

/*types*/
%union {
	int type_int;              /* Constant integer value */
	treeNode* node;
    };

//tokens
%token <node> INT ID SEMI COMMA ASSIGNOP  LT GT LTE GTE EQ NEQ PLUS MINUS STAR DIV PERCENT 
%token <node> AND OR NOT LP RP LB RB LC RC RETURN IF ELSE WHILE 
//int id ; , = < > <= >= == != + - * / % 
//&& || ! ( ) [ ] { } return if else while
%token <node> CONST VOID BREAK CONTINUE INTCONST
//const void break continue intconst? 

//association
%right ASSIGNOP  
%left OR AND LT GT LTE GTE EQ NEQ
%left PLUS MINUS STAR DIV PERCENT
%right NOT
%left LB RB LP RP

//noassociation 
%nonassoc ELSE
%nonassoc RETURN WHILE

//non-terminal
%type <node> CompUnit Decl FuncDef ConstDecl VarDecl
%type <node> ConstDef ConstInitVal ConstExp  
%type <node> InitVal Exp FuncType FuncFParams Block FuncFParam
%type <node> BlockItem Stmt LVal Cond AddExp LOrExp Number
%type <node> UnaryExp PrimaryExp UnaryOp FuncRParams
%type <node> MulExp RelExp EqExp LAndExp  

%type <node> ComConstDef_rep ConstExp_rep ComVarDef_rep VarDef
%type <node>  ComInitVal_rep ComFuncFParam_rep LbEXPRb_rep
%type <node> BlockItem_rep Exp_rep comExp_rep ComConstInitVal_rep

%%

CompUnit : CompUnit Decl {$$=newNodeOp("CompUnit");root = $$;addChild($$,$2);addChild($$,$1);}
	| CompUnit FuncDef {$$=newNodeOp("CompUnit");root = $$;addChild($$,$2);addChild($$,$1);}
	| Decl {$$=newNodeOp("CompUnit");root = $$;addChild($$,$1);}
	| FuncDef {$$=newNodeOp("CompUnit");addChild($$,$1);root = $$;}
	; 
	//ok

Decl : ConstDecl {$$=newNodeString("Decl","");addChild($$,$1);}
	| VarDecl {$$=newNodeString("Decl","");addChild($$,$1);}
	;
	//ok

ConstDecl : CONST FuncType ConstDef ComConstDef_rep SEMI {$$=newNodeOp("ConstDecl");addChild($$,$5);addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	;
ComConstDef_rep	: ComConstDef_rep COMMA ConstDef {$$=newNodeOp("ComConstDef_rep");addChild($$,$3);addChild($$,$2),addChild($$,$1);}
	| /*empty*/	 {$$=newNodeOp("ComConstDef_rep");addChild($$,NULL);}
	;
	//ok
 
ConstDef : ID ConstExp_rep ASSIGNOP ConstInitVal {$$=newNodeOp("ConstDef");addChild($$,$4);addChild($$,$3);addChild($$, $2);addChild($$,$1);}
	;
ConstExp_rep : ConstExp_rep LB ConstExp RB {$$=newNodeOp("ConstExp_rep");addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
  	| /* empty */ {$$=newNodeOp("ConstExp_rep");addChild($$, NULL);}
	;
	//ok

ConstInitVal : ConstExp {$$=newNodeOp("ConstInitVal");addChild($$,$1);}
	| LC ConstInitVal ComConstInitVal_rep RC {$$=newNodeOp("ConstInitVal");addChild($$,$4),addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| LC RC {$$=newNodeOp("ConstInitVal");addChild($$,$2);addChild($$,$1);}
	;
ComConstInitVal_rep : ComConstInitVal_rep COMMA ConstInitVal  {$$=newNodeOp("ComConstInitVal_rep");addChild($$,$3);addChild($$,$2),addChild($$,$1);}
	| /*empty*/		{$$=newNodeOp("ComConstInitVal_rep");addChild($$, NULL);}
	;
	//ok

//VarDecl : 
VarDecl : FuncType VarDef ComVarDef_rep SEMI {$$=newNodeOp("VarDecl");addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
ComVarDef_rep : ComVarDef_rep COMMA VarDef {$$=newNodeOp("ComVarDef_rep");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| /*empty*/ {$$=newNodeOp("ComVarDef_rep");addChild($$, NULL);}
	;
	//ok

VarDef : ID ConstExp_rep  {$$=newNodeOp("VarDef");addChild($$,$2);addChild($$,$1);}
	| ID ConstExp_rep ASSIGNOP InitVal {$$=newNodeOp("VarDef");addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}

InitVal : Exp {$$=newNodeOp("InitVal");addChild($$,$1);}
	| LC InitVal ComInitVal_rep RC {$$=newNodeOp("InitVal");addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| LC RC {$$=newNodeOp("InitVal");addChild($$,$2);addChild($$,$1);}
	;
ComInitVal_rep : ComInitVal_rep COMMA InitVal  {$$=newNodeOp("ComInitVal_rep");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| /*empty*/  {$$=newNodeOp("ComInitVal_rep");addChild($$, NULL);}
	//ok

FuncDef : FuncType ID LP FuncFParams RP Block  {$$=newNodeOp("FuncDef");addChild($$,$6);addChild($$,$5);addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| FuncType ID LP RP Block	{$$=newNodeOp("FuncDef");addChild($$,$5);addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	;
	//ok

FuncType : VOID {$$=newNodeOp("FuncType");addChild($$,$1);}
		  | INT {$$=newNodeOp("FuncType");addChild($$,$1);}

FuncFParams : FuncFParam ComFuncFParam_rep {$$=newNodeOp("FuncFParams");addChild($$,$2);addChild($$,$1);}
	;
ComFuncFParam_rep : ComFuncFParam_rep COMMA FuncFParam {$$=newNodeOp("ComFuncFParam_rep");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| /*empty*/ {$$=newNodeOp("ComFuncFParam_rep");addChild($$, NULL);}
	;
	//ok

FuncFParam : FuncType ID {$$=newNodeOp("FuncFParam");addChild($$,$2);addChild($$,$1);}
	| FuncType ID LB RB LbEXPRb_rep {$$=newNodeOp("FuncFParam");addChild($$,$5);addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	;
LbEXPRb_rep : LbEXPRb_rep LB Exp RB {$$=newNodeOp("LbEXPRb_rep");addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| /*empty*/ {$$=newNodeOp("LbEXPRb_rep");addChild($$, NULL);}
	;
	//ok

Block : LC BlockItem_rep RC {$$=newNodeOp("Block");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	;
BlockItem_rep : BlockItem_rep BlockItem {$$=newNodeOp("BlockItem_rep");addChild($$,$2);addChild($$,$1);}
	| /*empty*/ {$$=newNodeOp("BlockItem_rep");addChild($$, NULL);}
	;
	//ok

BlockItem : Decl  {$$=newNodeOp("BlockItem");addChild($$,$1);}
	| Stmt	{$$=newNodeOp("BlockItem");addChild($$,$1);}
	;
	//ok

Stmt : LVal ASSIGNOP Exp SEMI {$$=newNodeOp("Stmt");addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| Exp SEMI {$$=newNodeOp("Stmt");addChild($$,$2);addChild($$,$1);}
	| SEMI	{$$=newNodeOp("Stmt");addChild($$,$1);}
	| Block {$$=newNodeOp("Stmt");addChild($$,$1);}
	| IF LP Cond RP Stmt ELSE Stmt {$$=newNodeOp("Stmt");addChild($$,$7);addChild($$,$6);addChild($$,$5);addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| IF LP Cond RP Stmt {$$=newNodeOp("Stmt");addChild($$,$5);addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| WHILE LP Cond RP Stmt {$$=newNodeOp("Stmt");addChild($$,$5);addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| BREAK SEMI {$$=newNodeOp("Stmt");addChild($$,$2);addChild($$,$1);}
	| CONTINUE SEMI {$$=newNodeOp("Stmt");addChild($$,$2);addChild($$,$1);}
	| RETURN Exp SEMI {$$=newNodeOp("Stmt");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| RETURN SEMI {$$=newNodeOp("Stmt");addChild($$,$2);addChild($$,$1);}
	;
	//ok

Exp : AddExp {$$=newNodeOp("Exp");addChild($$,$1);}
	;
	//ok

Cond : LOrExp {$$=newNodeOp("Cond");addChild($$,$1);}
	; 

LVal : ID Exp_rep {$$=newNodeOp("LVal");addChild($$,$2);addChild($$,$1);}
	;	
Exp_rep : Exp_rep LB Exp RB  {$$=newNodeOp("Exp_rep");addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| /*empty*/ {$$=newNodeOp("Exp_rep");addChild($$, NULL);}
	;

PrimaryExp : LP Exp RP  {$$=newNodeOp("PrimaryExp");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| LVal {$$=newNodeOp("PrimaryExp");addChild($$,$1);}
	| Number {$$=newNodeOp("PrimaryExp");addChild($$,$1);}
	;

Number : INTCONST {$$=newNodeOp("Number");addChild($$,$1);}
	;

UnaryExp : PrimaryExp  {$$=newNodeOp("UnaryExp");addChild($$,$1);}
	| ID LP FuncRParams RP {$$=newNodeOp("UnaryExp");addChild($$,$4);addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| ID LP RP {$$=newNodeOp("UnaryExp");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| UnaryOp UnaryExp {$$=newNodeOp("UnaryExp");addChild($$,$2);addChild($$,$1);}
	;
	//ok

UnaryOp : PLUS  {$$=newNodeOp("UnaryOp");addChild($$,$1);}
	| MINUS {$$=newNodeOp("UnaryOp");addChild($$,$1);}
	| NOT {$$=newNodeOp("UnaryOp");addChild($$,$1);}
	;

FuncRParams : Exp comExp_rep {$$=newNodeOp("FuncRParams");addChild($$,$2);addChild($$,$1);}
	;
comExp_rep : comExp_rep COMMA Exp {$$=newNodeOp("comExp_rep");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| /*empty*/ {$$=newNodeOp("comExp_rep");addChild($$, NULL);}
	;
	

MulExp : UnaryExp {$$=newNodeOp("MulExp");addChild($$,$1);}
	| MulExp STAR UnaryExp {$$=newNodeOp("MulExp");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| MulExp DIV UnaryExp {$$=newNodeOp("MulExp");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| MulExp PERCENT UnaryExp {$$=newNodeOp("MulExp");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	;

AddExp : MulExp {$$=newNodeOp("AddExp");addChild($$,$1);}
	| AddExp PLUS MulExp {$$=newNodeOp("AddExp");addChild($$,$3);addChild($$,$2),addChild($$,$1);}
	| AddExp MINUS MulExp {$$=newNodeOp("AddExp");addChild($$,$3);addChild($$,$2),addChild($$,$1);}
	;

RelExp : AddExp {$$=newNodeOp("RelExp");addChild($$,$1);}
	| RelExp LT AddExp {$$=newNodeOp("RelExp");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| RelExp GT AddExp {$$=newNodeOp("RelExp");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| RelExp LTE AddExp {$$=newNodeOp("RelExp");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| RelExp GTE AddExp {$$=newNodeOp("RelExp");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	;

EqExp : RelExp {$$=newNodeOp("EqExp");addChild($$,$1);}
	| EqExp EQ RelExp {$$=newNodeOp("EqExp");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	| EqExp NEQ RelExp {$$=newNodeOp("EqExp");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	;

LAndExp : EqExp  {$$=newNodeOp("LAndExp");addChild($$,$1);}
	| LAndExp AND EqExp {$$=newNodeOp("LAndExp");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	;
LOrExp : LAndExp {$$=newNodeOp("LOrExp");addChild($$,$1);}
	| LOrExp OR LAndExp {$$=newNodeOp("LOrExp");addChild($$,$3);addChild($$,$2);addChild($$,$1);}
	;
ConstExp : AddExp {$$=newNodeOp("ConstExp");addChild($$,$1);}
	;

%%

int main(int argc, char** argv)
{
	root=NULL;
	yylineno=1;
	// yyrestart(f);
	yyparse();
	printTree(root,0);
	int errcnt = sdtParse(root);
	if (errcnt == 0) {
		printf("------------------IR INFO-------------------\n");
		CodeList cl = translate_InterCode(root, NULL);
		printf("------------------CODEGEN INFO--------------\n");
		codegen(cl);
	}
	return 0;
}

int yyerror(char* msg)
{
	printf("Error type B at line %d:%s===>unexpected near '%s'\n",yylineno,msg,yylval.node->value);
}