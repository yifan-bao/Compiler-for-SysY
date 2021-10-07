filePath=lexTest.txt
if [ $# -eq 1 ]; then filePath=$1; fi
flex lexTest.l 
gcc -o lextest -g ./lex.yy.c
./lextest < $filePath
rm lex.yy.c