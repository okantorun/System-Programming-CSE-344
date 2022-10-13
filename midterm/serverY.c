#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include<time.h>
#include "fifo_seqnum.h"
#include "become_daemon.h"

#define CLIENT_FIFO "/tmp/seqnum_cl"
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO)+20)
static char clientFifo[CLIENT_FIFO_NAME_LEN]= "/tmp/seqnum_cl";
#define FIFO_PERM (S_IRUSR | S_IWUSR | S_IWGRP )
#define dInstantiation "doubleInstantiation"

int requestCounter=0;
int invertibleCount=0;
int notInvertibleCount=0; 
int fdInstantiation;
int fdLog;
int handlingControl=0;
int forwardControl=0;
int forwardCount=0;
char tempPath[80]="0";

sem_t *semaControl;
sem_t *semaForward;
sem_t *semaRequest;
sem_t *semaInvControl;
mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

pid_t pidZ;

void PrintMessage(const char *message){
    write(STDERR_FILENO,message,strlen(message));
    exit(EXIT_FAILURE);
}
int                                     /* Returns 0 on success, -1 on error */
becomeDaemon(int flags)
{
    int maxfd, fd;

    switch (fork()) {                   /* Become background process */
    case -1: return -1;
    case 0:  break;                     /* Child falls through... */
    default: _exit(EXIT_SUCCESS);       /* while parent terminates */
    }

    if (setsid() == -1)                 /* Become leader of new session */
        return -1;

    switch (fork()) {                   /* Ensure we are not session leader */
    case -1: return -1;
    case 0:  break;
    default: _exit(EXIT_SUCCESS);
    }

    if (!(flags & BD_NO_UMASK0))
        umask(0);                       /* Clear file mode creation mask */

    if (!(flags & BD_NO_CLOSE_FILES)) { /* Close all open files */
        maxfd = sysconf(_SC_OPEN_MAX);
        if (maxfd == -1)                /* Limit is indeterminate... */
            maxfd = BD_MAX_CLOSE;       /* so take a guess */

        for (fd = 0; fd < maxfd; fd++)
            close(fd);
    }

    if (!(flags & BD_NO_REOPEN_STD_FDS)) {
        close(STDIN_FILENO);            /* Reopen standard fd's to /dev/null */

        fd = open("/dev/null", O_RDWR);

        if (fd != STDIN_FILENO)         /* 'fd' should be 0 */
            return -1;
        if (dup2(STDIN_FILENO, STDOUT_FILENO) != STDOUT_FILENO)
            return -1;
        if (dup2(STDIN_FILENO, STDERR_FILENO) != STDERR_FILENO)
            return -1;
    }

    return 0;
}

void handler(int sig_num){
    char buffer[256];


    sem_getvalue(semaRequest, &requestCounter);
    sem_getvalue(semaInvControl, &invertibleCount);

    sem_getvalue(semaForward, &forwardCount);

    time_t t;   // not a primitive datatype
    time(&t);

    sem_post(semaControl);
    sem_getvalue(semaControl, &handlingControl);
    if(handlingControl == 1){
        fdLog = open(tempPath, O_WRONLY | O_CREAT | O_APPEND ,mode);
        sprintf(buffer,"%s SIGINT received, terminating Z and exiting server Y. Total requests handled: %d, %d invertible, %d not. %d requests were forwarded\n",ctime(&t),requestCounter,invertibleCount,requestCounter-invertibleCount,forwardCount);
        write(fdLog,buffer,strlen(buffer));  
        close(fdLog);    
    }
    remove(dInstantiation);
    kill(pidZ,SIGKILL);
     exit(EXIT_FAILURE);
}

int** ReadMatrixFile(int matrixSize,char* oldMatrix,int **matrix){
    int col=0,row=0;
    int count=0;
    char *tempStr=NULL;
    int tempStrSize=0;
    tempStr = (char*)malloc(1);
     
    while(oldMatrix[count] != '\0'){
        if(oldMatrix[count]==','){
            matrix[row][col] = atoi(tempStr);
            tempStrSize=0;
            free(tempStr);
            tempStr = (char*)malloc(1);
            col++;
        }
        else if(oldMatrix[count] == '\n'){
             matrix[row][col] = atoi(tempStr);
            tempStrSize=0;
            free(tempStr);
            tempStr = (char*)malloc(1);
            row++;
            col=0;
        }
        else{
            tempStr = realloc(tempStr, tempStrSize+1);
            tempStr[tempStrSize] = oldMatrix[count];
            tempStrSize++;
        }
        count++;

    }
    matrix[row][col] = atoi(tempStr);
    free(tempStr);
    return matrix;
}
int CheckMatrixDesign(char* matrix){
    int col=0,row=0;
    int matrixSize=0;
    int count=0;

    while(matrix[count] != '\0'){
        if(matrix[count] == ','){
            col++;
            count++;
            matrixSize++;
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
void getCofactor(int** matrix, int** temp, int p, int q, int n)
{
    int i = 0, j = 0;
 
    // Looping for each element of the matrix
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < n; col++) {
            // Copying into temporary matrix only those element
            // which are not in given row and column
            if (row != p && col != q) {
                temp[i][j++] = matrix[row][col];
 
                // Row is filled, so increase row index and
                // reset col index
                if (j == n - 1) {
                    j = 0;
                    i++;
                }
            }
        }
    }
}
int determinantOfMatrix(int** matrix, int n)
{
    int D = 0; // Initialize result
 
    // Base case : if matrix contains single element
    if (n == 1)
        return matrix[0][0];
 
    int** temp; // To store cofactors
    temp = (int**)malloc(n*sizeof(int));
        for (int i = 0; i < n; ++i)
            temp[i] = (int*)malloc(n*sizeof(int)); 
 
    int sign = 1; // To store sign multiplier
 
    // Iterate for each element of first row
    for (int f = 0; f < n; f++) {
        // Getting Cofactor of mat[0][f]
        getCofactor(matrix, temp, 0, f, n);
        D += sign * temp[0][f] * determinantOfMatrix(temp, n - 1);
 
        // terms are to be added with alternate sign
        sign = -sign;
    }
    for (int i = 0; i < n; ++i)
            free(temp[i]);
    free(temp);
    return D;
}
int isInvertible(int** matrix, int n)
{
   
    if (determinantOfMatrix(matrix, n) != 0)
        return 1;
    else
        return 0;
}

int main(int argc , char *argv[]){


	int serverFd, dummyFd, clientFd;
  
    struct request req;//struct
    struct request req2;//struct
    char buffer[256]="0";
    int poolSize,poolSize2,sleepDuration;
    pid_t child_pid=1;
    pid_t child_pidZ=1;
    int filedes[2];//pipe for y's child
    int filedesZ[2];//pipe for Z
    int opt=0;
    int sflag=0, oflag=0, pflag=0, rflag=0, tflag=0;
    char *pathToLogFile = NULL, *pathServerFifo = NULL;
    int fdInst;
    char **enVar;

    if(fdInst = open(dInstantiation, O_WRONLY ) != -1){
        PrintMessage("File exist");
        exit(EXIT_FAILURE);
    }
    else{fdInst = open(dInstantiation, O_CREAT );}

    becomeDaemon(0);

     if(pipe(filedesZ) == -1)
        perror("pipe");

    enVar = (char**)malloc(2);
    for (int i = 0; i < 2; ++i)
        enVar[i] = (char*)malloc(5);
    sprintf(enVar[0],"%d",filedesZ[0]);
    sprintf(enVar[1],"%d",filedesZ[1]); 
    
    child_pidZ=fork();
    if(child_pidZ == 0){
        pidZ = child_pidZ;
        execve("./serverZ",argv,enVar);
        
    }
    
    for (int i = 0; i < 2; ++i)
        free(enVar[i]);
    free(enVar);



    
    sem_t *sema = mmap(NULL, sizeof(*sema),  PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1, 0);
    if (sema == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    if ( sem_init(sema, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    semaRequest = mmap(NULL, sizeof(*semaRequest),  PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1, 0);
    if (semaRequest == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    if ( sem_init(semaRequest, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    
    sem_t *sema3 = mmap(NULL, sizeof(*sema3),  PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1, 0);
    if (sema3 == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    if ( sem_init(sema3, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    semaControl = mmap(NULL, sizeof(*semaControl),  PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1, 0);
    if (semaControl == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    if ( sem_init(semaControl, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    semaInvControl = mmap(NULL, sizeof(*semaInvControl),  PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1, 0);
    if (semaInvControl == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    if ( sem_init(semaInvControl, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semaZ = mmap(NULL, sizeof(semaZ),  PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1, 0);
    if (semaZ == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    if ( sem_init(semaZ, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    semaForward = mmap(NULL, sizeof(*semaForward),  PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1, 0);
    if (semaForward == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    if ( sem_init(semaForward, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    struct sigaction sa;
    memset (&sa,0,sizeof(sa));
    sa.sa_handler = &handler;
    sigaction(SIGINT, &sa,NULL);


    while((opt = getopt(argc, argv, "s:o:p:r:t:")) != -1) {  
        switch(opt) { 
            case 'o': 
                oflag++;
                pathToLogFile = optarg;
                break;
            case 's':
                sflag++;
                pathServerFifo = optarg;
                pathServerFifo=optarg;
                break;
                
            case 'p': 
                pflag++;
                poolSize = atoi(optarg);
                break;
            case 'r': 
                rflag++;
                poolSize2 = atoi(optarg);
                break;
            case 't': 
                tflag++;
                sleepDuration=atoi(optarg);
                //*fd1=xopen(optarg);
                break;
        }
    }  
    if(sflag > 1 || oflag > 1 || pflag > 1 || rflag > 1 || tflag > 1)
        PrintMessage("Each flag can only use one time. \n");

    if(sflag == 0 || oflag == 0 || pflag == 0 || rflag == 0 || tflag == 0)
        PrintMessage("Each flag can only use one time. \n");
    
    strcpy(tempPath,pathToLogFile);

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

    fdLog = open(pathToLogFile, O_WRONLY | O_CREAT | O_APPEND ,mode);
    sprintf(buffer,"Server Y (%s , p=%d, t=%d) started\nInstantiated server Z\n",pathToLogFile,poolSize,sleepDuration);
    write(fdLog,buffer,strlen(buffer));

    if(pipe(filedes)==-1)
        perror("pipe");
   
    umask(0);

    if(mkfifo(pathServerFifo,FIFO_PERM) == -1)
        perror("SevermkFifo");

    serverFd = open(pathServerFifo, O_RDONLY );
    if(serverFd == -1)
        perror("serverFd");
    
    dummyFd = open(pathServerFifo,O_WRONLY);
    if(dummyFd == -1)
        perror("dummyFd");
    int matrixSize=0;
    int** matrix;

    for(int i=0;i<poolSize;i++) 
    {
        if(child_pid != 0)
            child_pid = fork();
    }
    
    close(fdLog);
    for(;;){
        if(child_pid == 0){
            while(1){
                read(filedes[0], &req2,  sizeof(struct request));
                matrixSize=CheckMatrixDesign(req2.matrix);
                matrix = (int**)malloc(sqrt(matrixSize)*sizeof(int));
                for (int i = 0; i < sqrt(matrixSize); ++i)
                    matrix[i] = (int*)malloc(sqrt(matrixSize)*sizeof(int)); 
    

                for (int i = 0; i < sqrt(matrixSize); ++i)
                    free(matrix[i]);
                free(matrix);

                clientFd = open(clientFifo, O_WRONLY);
                if(clientFd == -1)
                    perror("clientFd");
                time_t t;   // not a primitive datatype
                time(&t);
                int turn; 
                sem_getvalue(sema ,&turn);
                fdLog = open(pathToLogFile, O_WRONLY | O_CREAT | O_APPEND ,mode);
                sprintf(buffer,"%s Worker PID#%d is handling client PID#%d, matrix size %dx%d, pool busy %d/%d\n",ctime(&t),getpid(),req2.pid,(int)sqrt(matrixSize),(int)sqrt(matrixSize),turn,poolSize);
                write(fdLog,buffer,strlen(buffer));
                printf("%s Z:Worker PID#%d is handling client PID#%d, matrix size %dx%d, pool busy %d/%d\n",ctime(&t),getpid(),req2.pid,(int)sqrt(matrixSize),(int)sqrt(matrixSize),turn,poolSize);
                close(fdLog);
                sleep(sleepDuration);

                sem_post(semaInvControl);
                sem_getvalue(semaInvControl, &invertibleCount);
                write(clientFd, "Accepted",strlen("Accepted"));
                /*if(isInvertible(matrix,(int)sqrt(matrixSize))){
                    sem_post(semaInvControl);
                    sem_getvalue(semaInvControl, &invertibleCount);
                    write(clientFd, "Accepted",strlen("Accepted"));
                }
                else{
                   
                    write(clientFd, "NotAccepted",strlen("NotAccepted"));

                }*/
                if(close(clientFd) == -1){
                    perror("close");
                }
                sem_wait(sema);
            }
             
        }
        else    
        {   
            if(read(serverFd, &req, sizeof(struct request))
                        != sizeof(struct request)){
                perror("Error reading requestİdiscarding\n");
            }
            time_t t;   // not a primitive datatype
            time(&t);

            int zChildControl;
            int turn2; 
            sem_post(sema);
            sem_getvalue(sema, &turn2);
            
            sem_post(semaRequest);
            sem_getvalue(semaRequest, &requestCounter);
   
            if(turn2>poolSize){
                sem_post(semaZ);
                sem_getvalue(semaZ, &zChildControl);
                close(filedesZ[0]);
                
                
                if(zChildControl<=poolSize2){
                    sem_wait(sema);
                    sem_getvalue(sema, &turn2);
                    matrixSize=CheckMatrixDesign(req.matrix);
                    fdLog = open(pathToLogFile, O_WRONLY | O_CREAT | O_APPEND ,mode);
                    sprintf(buffer,"%s Forwarding request of client PID#%d, to serverZ, matrix size %dx%d, pool busy %d/%d\n",ctime(&t),getpid(),(int)sqrt(matrixSize),(int)sqrt(matrixSize),turn2,poolSize);
                    write(fdLog,buffer,strlen(buffer));
                    printf("%s Forwarding request of client PID#%d, to serverZ, matrix size %dx%d, pool busy %d/%d\n",ctime(&t),getpid(),(int)sqrt(matrixSize),(int)sqrt(matrixSize),turn2,poolSize);
                    close(fdLog);
                    sem_post(semaForward);
                    write(filedesZ[1], &req, sizeof(struct request));
                }
                else
                    write(filedes[1], &req,  sizeof(struct request));
            }
            else{
                write(filedes[1], &req,  sizeof(struct request));
            }
        }  
        
    }  
    
    return 0;
}