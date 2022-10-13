#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <math.h>
#include "fifo_seqnum.h"

#define CLIENT_FIFO "/tmp/seqnum_cl"
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO)+20)
static char clientFifo[CLIENT_FIFO_NAME_LEN]= "/tmp/seqnum_cl";
#define FIFO_PERM (S_IRUSR | S_IWUSR | S_IWGRP)

int xopen(const char* filePath );
void* xmalloc(int size);
void* xrealloc(void *var, int size);
int ErrorCheckOpenFile(int fd);
int ErrorCheckArgNum(int argc);
char* parseCommandLine(int argc, char **argv,int* fd1, int* fd2);
void printMessage(const char *errMessage);
int CheckMatrixDesign(int fd1);
char* ReadMatrixFile(int matrixSize,int fd1,char *matrix);


static void removeFifo(void)
{
    unlink(clientFifo);
}
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
void PrintMessage(const char *message){
    write(STDERR_FILENO,message,strlen(message));
    exit(EXIT_FAILURE);
}
int xopen(const char* filePath){
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    int fd=0;

    if ((fd = open(filePath, O_CREAT | O_RDWR, mode)) < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    return fd;
}

int CheckMatrixDesign(int fd1){
    int expectedCol=1;
    int col=0,row=0;
    char tempBuffer;
    int matrixSize=0;

    while(read(fd1,&tempBuffer,1) != '\0'){
        if(tempBuffer==','){
            if(row==0) {expectedCol++;}
            col++;
            matrixSize++;
        }
        else if(tempBuffer=='\n'){
            if(expectedCol!=col+1)
                PrintMessage("The number of columns and rows in the matrix must be equal.");
            row++;
            col=0;
            matrixSize++;
        }
        else{
            matrixSize++;
        }

    }
    if(expectedCol != row+1)
        PrintMessage("The number of columns and rows in the matrix must be equal.");
    else if(expectedCol < 2)
        PrintMessage("The number of columns and rows in the matrix must be equal.");
   
    close(fd1);
    return matrixSize;

}
int CheckMatrixDesign2(char* matrix){
    int col=0,row=0;
    int matrixSize=0;
    int count=0;

    while(matrix[count] != '\0'){
        if(matrix[count] == ','){
            col++;
            count++;
            //matrixSize++;
        }
        else if(matrix[count] == '\n'){
            row++;
            col=0;
            count++;
            matrixSize++;
        }
        else{
            count++;
        }
    }
    
    return matrixSize+1;

}


char* ReadMatrixFile(int matrixSize,int fd1,char *matrix){
    char tempBuffer;
    int size=0; 
    while(read(fd1,&tempBuffer,1) != '\0'){
         matrix[size]=tempBuffer;
         size++;
    }
    close(fd1);
    return matrix;
}


char* ParseCommandLine(int argc, char **argv,int* fd1,int* fd2,int *matrixSize){
    int opt=0;
    int sflag=0, oflag=0;
    char *myfifo;

    while((opt = getopt(argc, argv, "s:o:")) != -1) {  
        switch(opt) { 
            case 'o': 
                oflag++;
                *fd1=xopen(optarg);
                *matrixSize = CheckMatrixDesign(*fd1);
                *fd1=xopen(optarg);
                break;
            case 's':
                sflag++;
                myfifo=optarg;
                break;
        }
    }

    if(oflag > 1 || sflag > 1)
        PrintMessage("Each flag can only use one time.(Flags : -o, -s) \n");

    if(oflag == 0 || sflag == 0)
        PrintMessage("Each flag can only use one time.(Flags : -o, -s) \n");

    return myfifo;
}
void handler(int sig_num){
    PrintMessage("Exiting the program.. (Because of CTRL+C interrupt)\n");
}
    
int main(int argc,char *argv[]){
    int fd1,fd2;
    int matrixSize=0;
    int reelMatrixSize=0;
    char *serverFifo;
    char str[1024] = "0";
     int serverFd, clientFd;
     char *matrix=NULL;

    struct request req;

    struct sigaction sa;
    memset (&sa,0,sizeof(sa));
    sa.sa_handler = &handler;
    sigaction(SIGINT, &sa,NULL);

    if(ErrorCheckArgNum(argc)){
        return 1;
    }
    serverFifo=ParseCommandLine(argc,argv,&fd1,&fd2,&matrixSize);

    matrix = (char*)malloc((matrixSize+1)*sizeof(char));

    strcpy(req.matrix,ReadMatrixFile(matrixSize,fd1,matrix));

    reelMatrixSize=CheckMatrixDesign2(matrix);

    clock_t start, end;
     double cpu_time_used;
   

    umask(0);
    if(mkfifo(clientFifo,FIFO_PERM) == -1)
        perror("mkFifo");
    
    if(atexit(removeFifo) != 0)
        PrintMessage("atexit");

    req.pid = getpid();

    serverFd = open(serverFifo, O_WRONLY );
    if(serverFd == -1)
        perror("serverFifo");
    

     start = clock(); 
    if(write(serverFd, &req,sizeof(struct request)) !=
                sizeof(struct request)){
                    perror("Can't write to server");
                }
 

    clientFd = open(clientFifo, O_RDONLY );
    if(clientFd == -1)
        perror("clientFifo");
    read(clientFd, str, 1024);
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    printf("Client PID%d (path) is submitting is %dx%d matrix\n",req.pid,reelMatrixSize,reelMatrixSize);
    if(strcmp(str,"Accepted")==0){
        printf("Client PID%d: the matrix is invertible, total time %lf seconds, goodbye.",req.pid,cpu_time_used*100);
    }
    else
        printf("Client PID%d: the matrix is non-invertible, total time %lf seconds, goodbye.",req.pid,cpu_time_used*100);


    free(matrix);
    
    exit(EXIT_SUCCESS);
    

    
    

	return 0;

}
