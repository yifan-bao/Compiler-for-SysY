#ifndef _VECTOR_H_
#define _VECTOR_H_
#define NEW(p) ((p) = malloc(sizeof *(p)))
#define NEW0(p) memset(NEW(p), 0, sizeof *(p))
typedef void* DataType;

typedef struct Vector{
    DataType*   data;
	int     size;
    int     max_size;
} *vector;

vector newVector();
DataType getItem(vector v, int i);
void addItem(vector v, DataType item);
int getSize(vector v);
DataType getLastItem(vector v);
void removeLastItem(vector v);
void destoryVector(vector v);

#endif