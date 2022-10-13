#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "child.h"

double myPow(int base){

	int result=1;

	for (int exponent=2; exponent>0; exponent--)
	{
		result = result * base;
	}
	return result;
}
double covMatrixOneToZero(char **coordinates,double *avrCor){//1,0 and 0,1

	double avr=0.0;

	for(int i=0;i<10;i++){
		avr+=(coordinates[i][0]-avrCor[0])*(coordinates[i][1]-avrCor[1]);
	}
	return avr/10;

}
double covMatrixTwoToZero(char **coordinates,double *avrCor){//2,0 and 0,2

	double avr=0.0;

	for(int i=0;i<10;i++){
		avr+=(coordinates[i][0]-avrCor[0])*(coordinates[i][2]-avrCor[2]);
	}
	return avr/10;

}
double covMatrixOneToTwo(char **coordinates,double *avrCor){//1,2 and 2,1

	double avr=0.0;

	for(int i=0;i<10;i++){
		avr+=(coordinates[i][1]-avrCor[1])*(coordinates[i][2]-avrCor[2]);
	}
	return avr/10;

}
double covMatrixZeroToZero(char **coordinates,double *avrCor){//x
	double corX=0.0;
	for(int i=0;i<10;i++){
		corX+=(coordinates[i][0]-avrCor[0])*(coordinates[i][0]-avrCor[0]);
	}
	return corX/10;
}
double covMatrixOneToOne(char **coordinates,double *avrCor){//x
	double corY=0.0;

	for(int i=0;i<10;i++){
		corY+=(coordinates[i][1]-avrCor[1])*(coordinates[i][1]-avrCor[1]);
		
	}
	return corY/10;
}
double covMatrixTwoToTwo(char **coordinates,double *avrCor){//x
	double corZ=0.0;
	
	for(int i=0;i<10;i++){
		corZ+=(coordinates[i][2]-avrCor[2])*(coordinates[i][2]-avrCor[2]);
	}
	return corZ/10;
}

void FindMiddle(char **coordinates,double *avrCor){
	double avrX=0.0,avrY=0.0,avrZ=0.0;

	for(int i=0;i<10;i++){
		avrX+=coordinates[i][0];
		avrY+=coordinates[i][1];
		avrZ+=coordinates[i][2];
	}
	avrCor[0]=avrX/10.0;
	avrCor[1]=avrY/10.0;
	avrCor[2]=avrZ/10.0;
}

void* xmalloc(int size){
    void* p;
    if ((p = malloc(size)) == NULL){
        perror("ERROR:Out of memory.");
        exit(EXIT_FAILURE);
    }
    return p;
}
int ErrorCheck(int fd){
	if(fd == -1){
		perror("open");
		return 1;
	}
	return 0;
}
void PrintMatrix(double **matrix){
	for(int i=0;i<3;i++){
		for(int j=0;j<3;j++)
			printf("%0.2lf,",matrix[i][j]);
		printf("\n");
	}
}
void FillMatrix(char **coordinates,double *avrCor,double **matrix){
	matrix[1][0]=covMatrixOneToZero(coordinates,avrCor);
	matrix[0][1]=covMatrixOneToZero(coordinates,avrCor);
	matrix[2][0]=covMatrixTwoToZero(coordinates,avrCor);
	matrix[0][2]=covMatrixTwoToZero(coordinates,avrCor);
	matrix[1][2]=covMatrixOneToTwo(coordinates,avrCor);
	matrix[2][1]=covMatrixOneToTwo(coordinates,avrCor);
	matrix[0][0]=covMatrixZeroToZero(coordinates,avrCor);
	matrix[1][1]=covMatrixOneToOne(coordinates,avrCor);
	matrix[2][2]=covMatrixTwoToTwo(coordinates,avrCor);

}

int main(int argc, char *argv[],char *argve[]){

	double **matrix;
	double *avrCor;
	char *tempString;
	struct flock lock;
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

	tempString = (char*)xmalloc(sizeof(double));
	avrCor = (double*)xmalloc(3*sizeof(double));

	matrix = (double**)xmalloc(3*sizeof(double));
	for(int i=0;i<3;i++)
		matrix[i] = (double*)xmalloc(3*sizeof(double));
	

	int fd = open(argv[4], O_WRONLY| O_APPEND, mode);

	memset (&lock, 0, sizeof(lock));
	lock.l_type = F_WRLCK;
	 fcntl (fd, F_SETLKW, &lock);

	if(ErrorCheck(fd))
		return 1;
	
	FindMiddle(argve,avrCor);
	FillMatrix(argve,avrCor,matrix);	

	for(int i=0;i<3;i++){
		for(int j=0;j<3;j++){
			sprintf (tempString, "%0.2lf",matrix[i][j]);
			write(fd,tempString,4);
			if(j!=2)
				write(fd,",",1);
		}
		write(fd,"\n",1);
	}

	lock.l_type = F_UNLCK;
 	fcntl (fd, F_SETLKW, &lock);

	close(fd);
	exit(EXIT_SUCCESS);

}

