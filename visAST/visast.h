#ifndef VISAST_H_
#define VISAST_H_
#include "../node.h"
#include "../vector.h"
#include <string.h>
#include <stdlib.h>
void COSStylePrintAstNodes(treeNode* ast,FILE* f);
void COSStylePrintAstAsDot(treeNode *ast);
#endif