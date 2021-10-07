# myCompiler

A simple compiler for a sysY language

SysY language is a subset of the C programming language. Every source code of SysY program is stored with an extension of 'sy'. It has one and only one 'main' function. It can also have several global variable declarations, constant variable declarations and other function definitions. SysY language supports **int** type and **multidimensional arrays**. 'int' type integer is 32-bit signed number.

SysY language itself has no input/output(I/O) constructs, and I/O is provided by means of runtime library, which can be invoked inside functions of SysY programs. 

We implement a SysY language compiler which can carry out lexical analysis, syntax analysis, semantic analysis, code generation and optimization.

## 1 Lexical analysis

### 1.1 Tokens

* Types：`int`，`void`，`const`
* Control Flow：`if`，`else`，`while`，`continue`，`break`
* Function：`return`，`{}`，`;`
* Operators（by priority）:
  * `()`，`[]` ：parameter、subscript
  * `!`，`-` ：negation、minus
  * `*`，`/`，`%`：multiplication、division、modulus
  * `+`，`-`：plus、minus
  * `<`，`<=`，`>`，`>=`：relation operators
  * `==`，`!=`：equals to、not equals to
  * `&&`：logical and
  * `||`：logical or
  * `=`：assign
  * `,`：separator

### 1.2 Token definitions

```c
// keywords
%token <node> RETURN IF ELSE WHILE CONST VOID BREAK CONTINUE INT

// identifiers and numbers
%token <node> ID INTCONST

// Operators
%token <node> LT GT LTE GTE EQ NEQ PLUS MINUS STAR DIV PERCENT
%token <node> AND OR NOT ASSIGNOP

// Delimeters
%token <node> LP RP LB RB LC RC SEMI COMMA
//            (  )  [  ]  {  }  ;    ,

Regular expressions:

```c
// ID
identifier = [a-zA-Z_][a-zA-Z0-9_]*

// INTCONST
octal_const = 0[0-7]*
decimal_const = [1-9][0-9]*
hexadecimal_const = (0x|0X)[0-9A-F]+

// Comment
single_line_comment = "//".*
multi_line_comment = [/][*][^*]*[*]+([^*/][^*]*[*]+)*[/]


### 1.3 Implementation using Lex

We use Lex to implement lexical analysis. Lex source code is mainly a table composed of regular expressions and corresponding code snippets. Lex will convert the table into a corresponding program to read the input stream, divide the input into character strings matching regular expressions, and execute the corresponding program fragments after identifying the corresponding character strings. Lex recognizes expressions by generating deterministic finite automata, and code fragments will be executed in the order of expressions corresponding to strings appearing in the input stream.

Consider the generation of the subsequent syntax tree (details are given later), after identifying the corresponding string, we will create a new and return the corresponding node. Examples are as follows:

```c
"+"         { yylval.node = newNodeOp("PLUS"); return PLUS; }
```

The lex codes of keywords, operators, and delimiters are all similar to this, and will not be listed one by one.

Identifiers and real numbers are different. In addition to returning nodes of the corresponding type, they also require additional operations. For example, identifiers need to return the identifier name, and real numbers need to return the corresponding value. The specific implementation is as follows:

```c
// identifier
[a-zA-Z_][a-zA-Z0-9_]* { 
    yylval.node = newNodeString("ID", yytext); 
    return ID; 
} 
// numbers
{octal_const} {
    int itmp;
    sscanf(yytext, "%o", &itmp);
    yylval.node = newNodeInt("INTCONST", itmp); 
    return INTCONST; 
}
{decimal_const} {
    int itmp;
    sscanf(yytext, "%d", &itmp);
    yylval.node = newNodeInt("INTCONST", itmp); 
    return INTCONST; 
}
{hexadecimal_const} {
    int itmp;
    sscanf(yytext, "%x", &itmp);
    yylval.node = newNodeInt("INTCONST", itmp); 
    return INTCONST; 
}
```

When a blank character of comment is encountered, the operation is skipped.

## 2 Syntax analysis

### 2.1 Syntax 
///










