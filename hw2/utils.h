#ifndef __UTILS_H__
#define __UTILS_H__

void* xmalloc(int size);
void* xrealloc(void *var, int size);
int ErrorCheckOpenFile(int fd);
int ErrorCheckArgNum(int argc);
int ErroCheckInputOutput(char **argv);
void FormPrint(int childCount,char **coordinates);
void CombineAllMatrix(char *filePath,double **dMatrix,int childCount);
double *FrobeniusNorm(double **dMatrix,int childCount);
void FindDistance(double *frobeniusNormResult,int childCount);



#endif