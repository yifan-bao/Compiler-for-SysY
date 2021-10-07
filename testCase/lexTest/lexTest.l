%{
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TESTINFO(type,value) printf("LexToken(%s,%s,%d))\n",type,value,yylineno)

%}
octal_const 0[0-7]*
decimal_const [1-9][0-9]*
hexadecimal_const (0x|0X)[0-9A-F]+
%option yylineno
%option noyywrap
%%

[ \t\n]     {  }
";"         { TESTINFO("SEMI"  ,  ";");  }
","         { TESTINFO("COMMA" ,  ",");  }
"<="        { TESTINFO("LTE"   ,  "<="); }
">="        { TESTINFO("GTE"   ,  ">="); }
"=="        { TESTINFO("EQ"    ,  "=="); }
"!="        { TESTINFO("NEQ"   ,  "!="); }
"="         { TESTINFO("ASSIGNOP" , "=");}
"<"         { TESTINFO("LT"    ,  "<");  }
">"         { TESTINFO("GT"    ,  ">");  }
"!"         { TESTINFO("NOT"   ,  "!");  }
"+"         { TESTINFO("PLUS"  ,  "+");  }
"-"         { TESTINFO("MINUS" ,  "-");  }
"*"         { TESTINFO("STAR"  ,  "*");  }
"/"         { TESTINFO("DIV"   ,  "/");  }
"%"         { TESTINFO("PERCENT" , "%"); }
"("         { TESTINFO("LP"    ,  "(");  }
")"         { TESTINFO("RP"    ,  ")");  }
"["         { TESTINFO("LB"    ,  "[");  }
"]"         { TESTINFO("RB"    ,  "]");  }
"{"         { TESTINFO("LC"    ,  "{");  }
"}"         { TESTINFO("RC"    ,  "}");  }
"&&"        { TESTINFO("AND"   ,  "&&"); }
"||"        { TESTINFO("OR"    ,  "||"); }

"return"    { TESTINFO("RETURN","return"); }
"const"     { TESTINFO("CONST" ,"const"); }
"else"      { TESTINFO("ELSE"  ,"else"); }
"if"        { TESTINFO("IF"    ,"if"); }
"while"     { TESTINFO("WHILE" ,"while"); }
"void"      { TESTINFO("VOID"  ,"void"); }
"break"     { TESTINFO("BREAK" ,"break");}
"continue"  { TESTINFO("CONTINUE" ,"continue");}
"int"       { TESTINFO("INT"   ,"int"); }

[a-zA-Z_][a-zA-Z0-9_]* { 
    TESTINFO("ID"   ,yytext);
} 
{octal_const} {
    int itmp;   																														double tmp;
    sscanf(yytext, "%o", &itmp);
    printf("LexToken(%s,%d,%d))\n","INTCONST",itmp,yylineno);
}

{decimal_const} {
    int itmp;   																														double tmp;
    sscanf(yytext, "%d", &itmp);
    printf("LexToken(%s,%d,%d))\n","INTCONST",itmp,yylineno);
}

{hexadecimal_const} {
    int itmp;   																														double tmp;
    sscanf(yytext, "%x", &itmp);
    printf("LexToken(%s,%d,%d))\n","INTCONST",itmp,yylineno);
}

"//".* { /* Single line comment. DO NOTHING */}
[/][*][^*]*[*]+([^*/][^*]*[*]+)*[/]       { /* Multiline comment DO NOTHING */ }
[/][*]                                    { perror("fatal_error"); }


. {
    // error
}
%%

int main() {
    yylex();
}