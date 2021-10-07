%{
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "node.h"
#include "syntax.tab.h"
%}
octal_const 0[0-7]*
decimal_const [1-9][0-9]*
hexadecimal_const (0x|0X)[0-9A-F]+
%option yylineno
%option noyywrap
%%

[ \t\n]     {  }
";"         { yylval.node = newNodeOp("SEMI"); return SEMI; }
","         { yylval.node = newNodeOp("COMMA"); return COMMA; }
"<="        { yylval.node = newNodeString("LTE","<="); return LTE; }
">="        { yylval.node = newNodeString("GTE",">=");return GTE; }
"=="        { yylval.node = newNodeString("EQ","==");return EQ; }
"!="        { yylval.node = newNodeString("NEQ","!="); return NEQ; }
"="         { yylval.node = newNodeOp("ASSIGNOP"); return ASSIGNOP; }
"<"         { yylval.node = newNodeString("LT","<"); return LT; }
">"         { yylval.node = newNodeString("GT",">"); return GT; }
"!"         { yylval.node = newNodeOp("NOT"); return NOT; }
"+"         { yylval.node = newNodeOp("PLUS"); return PLUS; }
"-"         { yylval.node = newNodeOp("MINUS"); return MINUS; }
"*"         { yylval.node = newNodeOp("STAR"); return STAR; }
"/"         { yylval.node = newNodeOp("DIV"); return DIV; }
"%"         { yylval.node = newNodeOp("PERCENT"); return PERCENT; }
"("         { yylval.node = newNodeOp("LP"); return LP; }
")"         { yylval.node = newNodeOp("RP"); return RP; }
"["         { yylval.node = newNodeOp("LB"); return LB; }
"]"         { yylval.node = newNodeOp("RB"); return RB; }
"{"         { yylval.node = newNodeOp("LC"); return LC; }
"}"         { yylval.node = newNodeOp("RC"); return RC; }
"&&"        { yylval.node = newNodeOp("AND"); return AND; }
"||"        { yylval.node = newNodeOp("OR"); return OR; }

"return"    { yylval.node = newNodeOp("RETURN"); return RETURN; }
"const"     { yylval.node = newNodeOp("CONST"); return CONST; }
"else"      { yylval.node = newNodeOp("ELSE"); return ELSE; }
"if"        { yylval.node = newNodeOp("IF"); return IF; }
"while"     { yylval.node = newNodeOp("WHILE"); return WHILE; }
"void"      { yylval.node = newNodeOp("VOID"); return VOID; }
"break"     { yylval.node = newNodeOp("BREAK"); return BREAK; }
"continue"  { yylval.node = newNodeOp("CONTINUE"); return CONTINUE; }
"int"       { yylval.node = newNodeOp("INT"); return INT; }

[a-zA-Z_][a-zA-Z0-9_]* { 
    yylval.node = newNodeString("ID", yytext); 
    return ID; 
} 
{octal_const} {
    int itmp;   																														double tmp;
    sscanf(yytext, "%o", &itmp);
    yylval.node = newNodeInt("INTCONST", itmp); 
    return INTCONST; 
}

{decimal_const} {
    int itmp;   																														double tmp;
    sscanf(yytext, "%d", &itmp);
    yylval.node = newNodeInt("INTCONST", itmp); 
    return INTCONST; 
}

{hexadecimal_const} {
    int itmp;   																														double tmp;
    sscanf(yytext, "%x", &itmp);
    yylval.node = newNodeInt("INTCONST", itmp); 
    return INTCONST; 
}

"//".* { /* Single line comment. DO NOTHING */}
[/][*][^*]*[*]+([^*/][^*]*[*]+)*[/]       { /* Multiline comment DO NOTHING */ }
[/][*]                                    { perror("fatal_error"); }


. {
    // error
}
%%

