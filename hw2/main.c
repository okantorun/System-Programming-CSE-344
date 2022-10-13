#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include "utils.h"

void* xmalloc(int size){
    void* p;
    if ((p = malloc(size)) == NULL){
        perror("ERROR:Out of memory.");
        exit(EXIT_FAILURE);
    }
    return p;
}
void* xrealloc(void *var, int size){
    void* p;
    if ((p = realloc(var, size)) == NULL){
        perror("ERROR:Out of memory.");
        exit(EXIT_FAILURE);
    }
    return p;
}
int ErrorCheckOpenFile(int fd){
	if(fd == -1){
		perror("open");
		return 1;
	}
	return 0;
}
int ErrorCheckArgNum(int argc){
	if(argc!=5){
		perror("Number of Arguments");
		return 1;
	}
	return 0;
}
int ErroCheckInputOutput(char **argv){
	if(strcmp(argv[1],"-i") || strcmp(argv[3],"-o")){
		perror("please enter the input and output file paths in the correct order.");
		return 1;
	}
	return 0;
}

void FormPrint(int childCount,char **coordinates){
	printf("Created R_%d with ",childCount);
	for (int i=0;i<10;i++)
		printf("(%d,%d,%d),", coordinates[i][0],coordinates[i][1],coordinates[i][2]);
	printf("\n");
}

void CombineAllMatrix(char *filePath,double **dMatrix,int childCount){
	char tempBuffer;
	char *strMatrix;
	int matrixRow=0;
	int matrixColumn=0;
	int strSize=0;
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

	int fd = open(filePath, O_RDWR | O_EXCL, mode);
	if(ErrorCheckOpenFile(fd)){
		perror("open");
		exit(EXIT_FAILURE);
	}

	strMatrix=(char*)xmalloc(1);
	while(read(fd,&tempBuffer,1) != '\0'){

		if(tempBuffer==','){
			dMatrix[matrixRow][matrixColumn] = strtod(strMatrix,NULL);
			matrixColumn++;
			strSize=0;
			free(strMatrix);
			strMatrix=(char*)xmalloc(1);
		}
		else if(tempBuffer=='\n'){
			dMatrix[matrixRow][matrixColumn] = strtod(strMatrix,NULL);
			matrixRow++;
			matrixColumn=0;
			strSize=0;
			free(strMatrix);
			strMatrix=(char*)xmalloc(1);
		}
		else{
			strMatrix=(char*)realloc(strMatrix,strSize+1);
			strMatrix[strSize]=tempBuffer;
			strSize++;
		}	
			
	}
	free(strMatrix);

}
double *FrobeniusNorm(double **dMatrix,int childCount){
	int row=0,column=0;
	double result=0.0;
	int count=0;
	int tempChildCount=0;
	double *frobeniusNormResult;

	frobeniusNormResult=xmalloc(childCount*sizeof(double));

	for(row=0;row<3*childCount;row++){
		for(column=0;column<3;column++){
			result+= (dMatrix[row][column]*dMatrix[row][column]);
		}
		count++;
		if(count==3){
			count=0;
			frobeniusNormResult[tempChildCount]=sqrt(result);
			result=0;
			tempChildCount++;
		}
	}
	return frobeniusNormResult;
}

void FindDistance(double *frobeniusNormResult,int childCount){

	double min=abs(frobeniusNormResult[0]-frobeniusNormResult[1]);
	int child1,child2;

	for(int i=0;i<childCount;i++){
		for (int j = i+1; j < childCount; j++)
		{
			if(abs(frobeniusNormResult[i]-frobeniusNormResult[j])<min){
				if(frobeniusNormResult[i]-frobeniusNormResult[j]<0)
					min=(frobeniusNormResult[i]-frobeniusNormResult[j])*(-1.0);
				else{
					min=(frobeniusNormResult[i]-frobeniusNormResult[j]);
				}
				child1=i+1;
				child2=j+1;
			}
		}
	}
	printf("The closest 2 matrices are R_%d and R_%d , their distance is %0.2lf",child1,child2,min);
}

int main(int argc, char *argv[])
{	
	char tempBuffer;
	char **coordinates;
	int coordinateSize=0;
	int pointSize=0;
	int childCount=0;
	double **dMatrix;
	double *frobeniusNormResult;

	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

	int fd = open(argv[2], O_RDWR | O_EXCL, mode);
	int fd2 = open(argv[4], O_CREAT | O_TRUNC | O_WRONLY,mode);
	close(fd2);

	if(ErrorCheckOpenFile(fd))
		return 1;
	else if(ErrorCheckArgNum(argc))
		return 1;
	else if(ErroCheckInputOutput(argv))
		return 1;
	printf("Proccess P reading %s\n",argv[2]);
	pid_t child_pid;

	coordinates = (char**)xmalloc(10);
	coordinates[0] = (char*)xmalloc(3);
	while(read(fd,&tempBuffer,1) != '\0'){
		if(pointSize == 3){ 
			coordinates[coordinateSize][pointSize]='\0';
			coordinateSize++;
			coordinates[coordinateSize] = (char*)xmalloc(3);
			pointSize=0;
		}
		if(coordinateSize==10){
			coordinateSize=0;
			childCount++;
			FormPrint(childCount,coordinates);//Created Process
			child_pid=fork();
			coordinateSize=0;
			if(child_pid==0){
				coordinates[10]=NULL;
				execve("child", argv, coordinates);
			}
		}
		if(coordinateSize==0 && pointSize==0){
			
			
			coordinates[0] = (char*)xmalloc(3);
		}
		coordinates[coordinateSize][pointSize] = tempBuffer;
		pointSize++;

	}
	while(wait(NULL)>0){}

	dMatrix = (double**)xmalloc(3*childCount*sizeof(double)+5);
	for(int i=0;i<3*childCount*sizeof(double);i++)
		dMatrix[i] = (double*)xmalloc(3*sizeof(double));

	CombineAllMatrix(argv[4],dMatrix,childCount);

	frobeniusNormResult=malloc(childCount*sizeof(double));
	frobeniusNormResult= FrobeniusNorm(dMatrix,childCount);

	printf("Reached EOF,collecting outputs from %s\n",argv[4]);

	FindDistance(frobeniusNormResult,childCount);
	
	free(frobeniusNormResult);
	free(coordinates);
	free(dMatrix);

	return 0;
}