#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/mman.h>
#include<time.h>
#include<math.h>
#include<pthread.h>
#include <sys/ipc.h>
#include <sys/sem.h>

double PI = 3.141592653589793;

int fdInput1,fdInput2,fdOut;
int mValue;
int nValue;
int matrixEdge;
double **fileValues1, **fileValues2, **matrixC, **DFTRe, **DFTIm;
pthread_t *tid;
char *filePath1 = NULL;
char *filePath2 = NULL;
char *outputFile = NULL;
struct matrixinfo *objmatrixinfo; 
int arrived=0;
pthread_mutex_t mutex;
pthread_cond_t cond ;

struct matrixinfo{
    int start;
    int end;
    int matrixEdge;
    int thrNum;
};

void PrintMessage(const char *message){
    write(STDERR_FILENO,message,strlen(message));
    exit(EXIT_FAILURE);
}

void ErrorCheckArgNum(int argc){
	if(argc != 11)
        PrintMessage("Number of arguments is not correct");
}

void handler(int sig_num){

    free(objmatrixinfo);
    free(tid);
    for(int i = 0; i < matrixEdge*matrixEdge; i++)
        free(fileValues1[i]);
    free(fileValues1);

    for(int i = 0; i < matrixEdge*matrixEdge; i++)
        free(fileValues2[i]);
    free(fileValues2);

    for(int i = 0; i < matrixEdge*matrixEdge; i++)
        free(matrixC[i]);
    free(matrixC);

    for(int i = 0; i < matrixEdge*matrixEdge; i++)
        free(DFTIm[i]);
    free(DFTIm);

    for(int i = 0; i < matrixEdge*matrixEdge; i++)
        free(DFTRe[i]);
    free(DFTRe);
    close(fdInput1);
    close(fdInput2);
    close(fdOut);
    printf("Exiting the program... (Because of CTRL+C interrupt)\n");
    exit(EXIT_FAILURE);
}

void *CalculateC(void *arg){
    int start,end,matrixEdge,thrNum;
    double result=0;
    struct matrixinfo *my_matrixinfo = (struct matrixinfo*)arg;
    double cpu_time_used;
    clock_t startTime, endTime;
    char timeArr[100];
    time_t t;  

    start = (*my_matrixinfo).start;
    end = (*my_matrixinfo).end;
    matrixEdge = (*my_matrixinfo).matrixEdge;
    thrNum = (*my_matrixinfo).thrNum;
    startTime = clock();

    for(int i = start ; i < end; i++){
        for(int j = 0 ; j < matrixEdge; j++){
            for(int k = 0 ; k < matrixEdge; k++){
               result += fileValues1[j][k] * fileValues2[k][i]; 
            }
            matrixC[j][i] = result;
            result = 0;
        }
    }
    endTime = clock();
    cpu_time_used = ((double) (endTime - startTime)) / CLOCKS_PER_SEC;
    time(&t);
    sprintf(timeArr,"%s ",ctime(&t));
    timeArr[strlen(timeArr)-2] = '\0';
    printf("%s Thread %d has reached the rendezvous point in %lf seconds.\n",timeArr,thrNum,cpu_time_used);

    pthread_mutex_lock(&mutex);
    ++arrived;
    while(arrived < mValue)
    {   
        pthread_cond_wait(&cond, &mutex);
    }    
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
    
    time(&t);
    sprintf(timeArr,"%s ",ctime(&t));
    timeArr[strlen(timeArr)-2] = '\0';
    printf("%s Thread %d is advancing to the second part\n",timeArr,thrNum);
    int totalCell;
    totalCell = pow(2,nValue);
    startTime = clock();
    for(int k=start;k<end;k++)
    {
        for(int l=0;l<totalCell;l++)
        {
            double reTemp=0; 
            double imTemp=0;
            for(int m=0;m<totalCell;m++)
            {
                for(int n=0;n<totalCell;n++)
                {

                    double x=-2.0*PI*k*m/(double)totalCell;
                    double y=-2.0*PI*l*n/(double)totalCell;
                    reTemp+=matrixC[m][n]*cos(x+y);
                    imTemp+=matrixC[m][n]*1.0*sin(x+y);
                }
            }
            DFTRe[k][l]=reTemp;
            DFTIm[k][l]=imTemp;
        }
    }
    endTime = clock();
    cpu_time_used = ((double) (endTime - startTime)) / CLOCKS_PER_SEC;
    time(&t);
    sprintf(timeArr,"%s ",ctime(&t));
    timeArr[strlen(timeArr)-2] = '\0';
    printf("%s Thread %d has has finished the second part in %lf seconds.\n",timeArr,thrNum,cpu_time_used);
    free(my_matrixinfo);
    return NULL;

}

int main(int argc , char *argv[]){
    char tempChar;
    void *res;
    double cpu_time_used;
    clock_t startTime, endTime;
    char timeArr[100];
    time_t t; 

    setbuf(stdout,NULL);

    struct sigaction sa;
    memset (&sa,0,sizeof(sa));
    sa.sa_handler = &handler;
    sigaction(SIGINT, &sa,NULL);
  
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    ErrorCheckArgNum(argc);
    
    int opt=0,iFlag=0,jFlag=0,oFlag=0,nFlag=0,mFlag=0;
    while((opt = getopt(argc, argv, "i:j:o:n:m:")) != -1) {  
        switch(opt) { 
            case 'i': 
                iFlag++;
                filePath1 = optarg;
                break;
            case 'j': 
                jFlag++;
                filePath2 = optarg;
                break;
            case 'o': 
                oFlag++;
                outputFile = optarg;
                break;
            case 'n': 
                nFlag++;
                nValue = atoi(optarg);
                break;
            case 'm': 
                mFlag++;
                mValue = atoi(optarg);
                break;
            default:
                PrintMessage("Invalid Argument \n");
                break;
        }
    }  
    if(iFlag > 1 || jFlag > 1 || oFlag > 1 || nFlag > 1 || mFlag > 1)
        PrintMessage("Each flag can only use one time. \n");

    if(iFlag == 0 || jFlag == 0 || oFlag == 0 || nFlag == 0 || mFlag == 0)
        PrintMessage("Each flag can only use one time. \n");

    if(nValue < 3){
        PrintMessage("The value of n must be greater than 2.\n");     // n > 2
    }
    if(mValue < 2){
        PrintMessage("The value of m must be greater than 1.\n");     // m > 1
    }

    matrixEdge = pow(2, nValue);
    int lastThread=0;
    int responsibleCol;

    if(mValue > matrixEdge){
        while(matrixEdge % mValue !=0){
            mValue = mValue - 1;
        }
        responsibleCol = matrixEdge / mValue;
    }
    else if(matrixEdge % mValue != 0){
       responsibleCol = matrixEdge / mValue;
       lastThread = matrixEdge - (matrixEdge/mValue * (mValue-1));
    }
    else{
        responsibleCol = matrixEdge / mValue;
    }



    fileValues1 = (double**)malloc(matrixEdge * matrixEdge * sizeof(double));
	for(int i = 0 ; i < matrixEdge * matrixEdge ; i++)
		fileValues1[i] = (double*)malloc(matrixEdge * matrixEdge * sizeof(double));

    fileValues2 = (double**)malloc(matrixEdge * matrixEdge * sizeof(double));
	for(int i = 0 ; i < matrixEdge * matrixEdge ; i++)
		fileValues2[i] = (double*)malloc(matrixEdge * matrixEdge * sizeof(double));

    matrixC = (double**)malloc(matrixEdge * matrixEdge * sizeof(double));
	for(int i = 0 ; i < matrixEdge * matrixEdge ; i++)
		matrixC[i] = (double*)malloc(matrixEdge * matrixEdge * sizeof(double));

    DFTRe = (double**)malloc(matrixEdge * matrixEdge * sizeof(double));
	for(int i = 0 ; i < matrixEdge * matrixEdge ; i++)
		DFTRe[i] = (double*)malloc(matrixEdge * matrixEdge * sizeof(double));

    DFTIm = (double**)malloc(matrixEdge * matrixEdge * sizeof(double));
	for(int i = 0 ; i < matrixEdge * matrixEdge ; i++)
		DFTIm[i] = (double*)malloc(matrixEdge * matrixEdge * sizeof(double));
        
    

    fdInput1 = open(filePath1, O_RDONLY, mode);
    fdInput2 = open(filePath2, O_RDONLY, mode);
    int matrixCounter = 0;
    int matrixRow = 0;
    int matrixCol = 0;
    startTime = clock();
    while(matrixCounter < (matrixEdge * matrixEdge)){
        if(read(fdInput1,&tempChar,1) == '\0'){
            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(fileValues1[i]);
            free(fileValues1);

            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(fileValues2[i]);
            free(fileValues2);

            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(matrixC[i]);
            free(matrixC);

            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(DFTIm[i]);
            free(DFTIm);

            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(DFTRe[i]);
            free(DFTRe);
            PrintMessage("Your inputfile1 does not have enough characters !!!\n");

        }
        if((matrixCounter % matrixEdge == 0) && matrixCounter != 0){
            matrixRow++;
            matrixCol = 0;
        }
        fileValues1[matrixRow][matrixCol] = tempChar;
        matrixCol++;
        matrixCounter++;
    }

    matrixCounter = 0;
    matrixRow = 0;
    matrixCol = 0;
    while(matrixCounter < (matrixEdge * matrixEdge)){
        if(read(fdInput2,&tempChar,1) == '\0'){
            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(fileValues1[i]);
            free(fileValues1);

            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(fileValues2[i]);
            free(fileValues2);

            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(matrixC[i]);
            free(matrixC);

            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(DFTIm[i]);
            free(DFTIm);

            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(DFTRe[i]);
            free(DFTRe);
            PrintMessage("Your inputfile2 does not have enough characters !!!\n");
        }
        if((matrixCounter % matrixEdge == 0) && matrixCounter != 0){
            matrixRow++;
            matrixCol = 0;
        }
        fileValues2[matrixRow][matrixCol] = tempChar;
        matrixCol++;
        matrixCounter++;
    }
    time(&t);
    sprintf(timeArr,"%s ",ctime(&t));
    timeArr[strlen(timeArr)-2] = '\0';
    printf("%s Two matrices of size %dx%d have been read. The number of threads is %d\n",timeArr,matrixEdge,matrixEdge,mValue);

    int error;
    int s;
    tid = malloc(mValue * sizeof(pthread_t));

    for(int i = 0; i < mValue; i++){
        if(i == mValue-1 && lastThread != 0){
            objmatrixinfo = malloc(sizeof(struct matrixinfo));
            (*objmatrixinfo).start = i * responsibleCol;
            (*objmatrixinfo).end = matrixEdge;
            (*objmatrixinfo).matrixEdge = matrixEdge;
            (*objmatrixinfo).thrNum = i;
        }
        else{
            objmatrixinfo = malloc(sizeof(struct matrixinfo));
            (*objmatrixinfo).start = i * responsibleCol;
            (*objmatrixinfo).end = (i * responsibleCol) + responsibleCol;
            (*objmatrixinfo).matrixEdge = matrixEdge;
            (*objmatrixinfo).thrNum = i;
        }
        if ((error = pthread_create(&tid[i], NULL, CalculateC, (void*)objmatrixinfo))){
            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(fileValues1[i]);
            free(fileValues1);

            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(fileValues2[i]);
            free(fileValues2);

            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(matrixC[i]);
            free(matrixC);

            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(DFTIm[i]);
            free(DFTIm);

            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(DFTRe[i]);
            free(DFTRe);
            PrintMessage("Failed to create thread.");
        }
    }
    for(int i=0; i<mValue; i++){
        s = pthread_join(tid[i], &res);
        if(s != 0){
            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(fileValues1[i]);
            free(fileValues1);

            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(fileValues2[i]);
            free(fileValues2);

            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(matrixC[i]);
            free(matrixC);

            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(DFTIm[i]);
            free(DFTIm);

            for(int i = 0; i < matrixEdge*matrixEdge; i++)
                free(DFTRe[i]);
            free(DFTRe);
            PrintMessage("The thread could not join.");
        }
    }
    endTime = clock();
    cpu_time_used = ((double) (endTime - startTime)) / CLOCKS_PER_SEC;
    time(&t);
    sprintf(timeArr,"%s ",ctime(&t));
    timeArr[strlen(timeArr)-2] = '\0';
    printf("%s The process has written the output file. The total time spent is %lf seconds.\n",timeArr,cpu_time_used);

    fdOut = open(outputFile, O_WRONLY | O_CREAT | O_APPEND ,mode);
    char tempDFT[500];
    for(int i=0; i< matrixEdge; i++){
        for(int j=0 ; j<matrixEdge;j++){
            sprintf (tempDFT, "%lf + j(%lf),",DFTRe[i][j],DFTIm[i][j]);
			write(fdOut,tempDFT,strlen(tempDFT));
        }
        write(fdOut,"\n",1);
    }
    
    free(objmatrixinfo);
    free(tid);
    for(int i = 0; i < matrixEdge*matrixEdge; i++)
        free(fileValues1[i]);
    free(fileValues1);

    for(int i = 0; i < matrixEdge*matrixEdge; i++)
        free(fileValues2[i]);
    free(fileValues2);

    for(int i = 0; i < matrixEdge*matrixEdge; i++)
        free(matrixC[i]);
    free(matrixC);

    for(int i = 0; i < matrixEdge*matrixEdge; i++)
        free(DFTIm[i]);
    free(DFTIm);

    for(int i = 0; i < matrixEdge*matrixEdge; i++)
        free(DFTRe[i]);
    free(DFTRe);


    close(fdInput1);
    close(fdInput2);
    close(fdOut);
    pthread_exit(0);
}