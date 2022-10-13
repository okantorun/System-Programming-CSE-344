#ifndef __CHILD_H__
#define __CHILD_H__

double myPow(int base);
double covMatrixOneToZero(char **coordinates,double *avrCor);
double covMatrixTwoToZero(char **coordinates,double *avrCor);
double covMatrixOneToTwo(char **coordinates,double *avrCor);
double covMatrixZeroToZero(char **coordinates,double *avrCor);
double covMatrixOneToOne(char **coordinates,double *avrCor);
double covMatrixTwoToTwo(char **coordinates,double *avrCor);
void FindMiddle(char **coordinates,double *avrCor);
void* xmalloc(int size);
int ErrorCheck(int fd);
void PrintMatrix(double **matrix);
void FillMatrix(char **coordinates,double *avrCor,double **matrix);




#endif