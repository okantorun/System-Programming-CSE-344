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
#include <sys/mman.h>
#include "fifo_seqnum.h"
#include "become_daemon.h"

#define CLIENT_FIFO "/tmp/seqnum_cl"
#define CLIENT_FIFO_NAME_LEN (sizeof(CLIENT_FIFO)+20)
static char clientFifo[CLIENT_FIFO_NAME_LEN]= "/tmp/seqnum_cl";
#define FIFO_PERM (S_IRUSR | S_IWUSR | S_IWGRP )
#define dInstantiation "doubleInstantiation"

struct childZ{
    int childsPid;
    int status;
};

int requestCounter=0;
int invertibleCount=0;
int notInvertibleCount=0; 
int fdInstantiation;
int fdLog;
int handlingControl=0;
int forwardControl=0;
char tempPath[80]="0";

sem_t *semaControl;
sem_t *semaForward;
sem_t *semaRequest;
sem_t *semaInvControl;

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

    time_t t;   // not a primitive datatype
    time(&t);

    sem_post(semaControl);
    sem_getvalue(semaControl, &handlingControl);

    

    if(handlingControl == 1){
        mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
        fdLog = open(tempPath, O_WRONLY | O_CREAT | O_APPEND ,mode);
        sprintf(buffer,"%s SIGINT received, terminating Z and exiting server Y. Total requests handled: %d, %d invertible, %d not. 6 requests were forwarded\n",ctime(&t),requestCounter,invertibleCount,requestCounter-invertibleCount);
        write(fdLog,buffer,strlen(buffer));      
        close(fdLog);
    }
    remove(dInstantiation);
    kill(getpid(),SIGKILL);
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
int CheckMatrixDesign(char matrix[1024]){
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
void* create_shared_memory(size_t size) {
  // Our memory buffer will be readable and writable:
  int protection = PROT_READ | PROT_WRITE;

  // The buffer will be shared (meaning other processes can access it), but
  // anonymous (meaning third-party processes cannot obtain an address for it),
  // so only this process and its children will be able to use it:
  int visibility = MAP_SHARED | MAP_ANONYMOUS;

  // The remaining parameters to `mmap()` are not important for this use case,
  // but the manpage for `mmap` explains their purpose.
  return mmap(NULL, size, protection, visibility, -1, 0);
}

int main(int argc , char *argv[],char *argve[]){


	int  clientFd;
    struct request req;//struct
    char buffer[256]="0";
    int poolSize2,sleepDuration;
    pid_t child_pid=1;
    int filedesZ[2];//pipe
    int opt=0;
    int sflag=0, oflag=0, pflag=0, rflag=0, tflag=0;
    char *pathToLogFile = NULL;
    int matrixSize=0;
     int** matrix;
    
    struct childZ *objChildsZ;

    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    fdLog = open(pathToLogFile, O_WRONLY | O_CREAT | O_APPEND ,mode);

    while((opt = getopt(argc, argv, "s:o:p:r:t:")) != -1) {  
        switch(opt) { 
            case 'o': 
                oflag++;
                pathToLogFile = optarg;
                break;
            case 's':
                sflag++;
                break;
                
            case 'p': 
                pflag++;
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
    struct sigaction sa;
    memset (&sa,0,sizeof(sa));
    sa.sa_handler = &handler;
    sigaction(SIGKILL, &sa,NULL);
    strcpy(tempPath,pathToLogFile);

    objChildsZ = malloc(poolSize2*sizeof(int));

    filedesZ[0]=atoi(argve[0]);
    filedesZ[1]=atoi(argve[1]);


    struct request* shmem = (struct request *)create_shared_memory(128);

    objChildsZ = (struct childZ *)malloc(poolSize2 * sizeof(struct childZ));
    sem_t *sema = mmap(NULL, sizeof(*sema),  PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1, 0);
    if (sema == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    if ( sem_init(sema, 1, 0) < 0) {
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

    semaRequest = mmap(NULL, sizeof(*semaRequest),  PROT_READ |PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1, 0);
    if (semaRequest == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    if ( sem_init(semaRequest, 1, 0) < 0) {
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

    
    
    
    for(int i=0;i<poolSize2;i++) 
    {
        if(child_pid != 0){
            child_pid = fork();
            if(child_pid != 0){
                objChildsZ[i].childsPid = child_pid;
                objChildsZ[i].status = 0;
                kill(child_pid, SIGSTOP);
            }
        }
    }
    close(fdLog);

     for(;;){
        if(child_pid == 0){
            while(1){
                matrixSize=CheckMatrixDesign(shmem->matrix);
                matrix = (int**)malloc(sqrt(matrixSize)*sizeof(int));
                for (int i = 0; i < sqrt(matrixSize); ++i)
                    matrix[i] = (int*)malloc(sqrt(matrixSize)*sizeof(int)); 
                matrix = ReadMatrixFile(matrixSize,shmem->matrix,matrix);

                for (int i = 0; i < sqrt(matrixSize); ++i)
                    free(matrix[i]);
                free(matrix);

                clientFd = open(clientFifo, O_WRONLY);
                if(clientFd == -1)
                    perror("clientFd");
                int temp=(int)sqrt(matrixSize);

                time_t t;   // not a primitive datatype
                time(&t);
                int turn; 
                sem_getvalue(sema ,&turn);
                fdLog = open(pathToLogFile, O_WRONLY | O_CREAT | O_APPEND ,mode);
                sprintf(buffer,"Z:%s Worker PID#%d is handling client PID#%d, matrix size %dx%d, pool busy %d/%d\n",ctime(&t),getpid(),shmem->pid,temp,temp,turn,poolSize2);
                write(fdLog,buffer,strlen(buffer));
                printf("%s Worker PID#%d is handling client PID#%d, matrix size %dx%d, pool busy %d/%d\n",ctime(&t),getpid(),shmem->pid,(int)sqrt(matrixSize),(int)sqrt(matrixSize),turn,poolSize2);
                close(fdLog);
                sleep(sleepDuration);

                write(clientFd, "Accepted",strlen("Accepted"));
                
                 for(int i=0;i<poolSize2;i++){
                    if(objChildsZ[i].childsPid == getpid()){
                        objChildsZ[i].status = 0;
                    }
                }
                if(close(clientFd) == -1){
                    perror("close");
                }

                sem_wait(sema);
                kill(getpid(), SIGSTOP);
            }
        }
        else{
            read(filedesZ[0], &req, sizeof(struct request));
            memcpy(shmem, &req, sizeof(struct request));
            int turn2; 
            sem_post(sema);
            sem_getvalue(sema, &turn2);
            for(int i=0;i<poolSize2;i++){
                if(objChildsZ[i].status == 0){
                    kill(objChildsZ[i].childsPid, SIGCONT);
                    objChildsZ[i].status = 1;
                    break;
                }
            }
        }
     }
     close(fdLog);

    
    return 0;
}