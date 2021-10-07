#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdio.h>
#include "vector.h"

vector newVector() {
    vector v;
    NEW0(v);
    v -> size = 0;
    v -> max_size = 10;
    v -> data = (DataType *)malloc(v->max_size*sizeof(DataType));
    memset(v->data, 0, 10);
    return v;
}

DataType getItem(vector v, int i) {
    return v->data[i];
}

void addItem(vector v, DataType item) {
    DataType *ptr;
    if (v->size >= v->max_size) {
        v->max_size *= 2;
        ptr = (DataType *)malloc(v->max_size*sizeof(DataType));
        for (int i = 0; i < v->size; i ++) {
            ptr[i] = v->data[i];
        }
        free(v->data);
        v->data = ptr;
    }
    v->data[v->size] = item;
    v->size += 1;
}

int getSize(vector v) {
    return v->size;
}

int isEmpty(vector v) {
    if (v->size == 0) return 1;
    else return 0;
}

DataType getLastItem(vector v) {
    if (v->size == 0) return NULL;
    else return v->data[v->size - 1];
}

void removeLastItem(vector v) {
    if (getLastItem(v) != NULL) {
        v->data[v->size - 1] = NULL;
        v->size -= 1;
    }
}

void destoryVector(vector v) {
    for (int i = 0; i < v->size; i ++) {
        v->data[i] = NULL;
    }
    free(v->data);
    free(v);
}

// test vector
// int main() {
//     vector v = newVector();
//     int a = 1;
//     int b = 2;
//     int c = 3;
//     addItem(v, &a);
//     addItem(v, &b);
//     addItem(v, &c);

//     printf("%d\n", *(int*)getLastItem(v));
//     printf("%d\n", *(int*)getItem(v, 0));
//     printf("%d\n", *(int*)getItem(v, 1));
//     printf("%d\n", *(int*)getItem(v, 2));
//     removeLastItem(v);
//     printf("%d\n", *(int*)getLastItem(v));
//     destoryVector(v);
// }