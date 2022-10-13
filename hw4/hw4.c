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
#include<pthread.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "sem.h"

int fileSize;
int semid;
int consumerNumber,nValue;
int fdInput;

int sempostOne(int semid){
    struct sembuf sbObj;
    sbObj.sem_num = 0;
    sbObj.sem_op = 1;
    sbObj.sem_flg = 0;
    return semop(semid, &sbObj, 1);
}
int sempostTwo(int semid){
    struct sembuf sbObj;
    sbObj.sem_num = 1;
    sbObj.sem_op = 1;
    sbObj.sem_flg = 0;
    return semop(semid, &sbObj, 1);
}
int semwaitOneAndTwo(int semid){
    struct sembuf sbObj[2];
    
    sbObj[0].sem_num = 0;
    sbObj[1].sem_num = 1;

    sbObj[0].sem_op = -1;
    sbObj[1].sem_op = -1;

    sbObj[0].sem_flg = 0;
    sbObj[1].sem_flg = 0;

    return semop(semid, sbObj, 2);
}
int binary_semaphore_initialize (int semid)
{
    union semun argument;
    unsigned short values[2];
    values[0] = 0;
    values[1] = 0;
    argument.array = values;
    return semctl (semid, 0, SETALL, argument);
}

void PrintMessage(const char *message){
    write(STDERR_FILENO,message,strlen(message));
    exit(EXIT_FAILURE);
}

void ErrorCheckArgNum(int argc){
	if(argc!=7)
        PrintMessage("Number of arguments is not correct");
}
void *SupplierFd(void *arg){
    int fd;
    int currentIndex=0;
    char tempChar;
    union semun argument;
    int value1,value2;
    char timeArr[100];

    fd = *((int *)(arg));

    time_t t;

    while(read(fd,&tempChar,1) != '\0' && currentIndex < fileSize*2){
        if(tempChar == '1'){
            value1 = semctl (semid, 0, GETVAL, argument);
            value2 = semctl (semid, 1, GETVAL, argument);
            time(&t);
            sprintf(timeArr,"%s ",ctime(&t));
            timeArr[strlen(timeArr)-2] = '\0';
            printf("%s Supplier: read from input a ‘1’. Current amounts: %d x ‘1’, %d x ‘2’.\n",timeArr,value1,value2);
            sempostOne(semid);
            value1 = semctl (semid, 0, GETVAL, argument);
            value2 = semctl (semid, 1, GETVAL, argument);
            time(&t);
            sprintf(timeArr,"%s ",ctime(&t));
            timeArr[strlen(timeArr)-2] = '\0';
            printf("%s Supplier: delivered a ‘1’. Post-delivery amounts: %d x ‘1’, %d x ‘2’.\n",timeArr,value1,value2);
        }
        else if(tempChar == '2'){
            value1 = semctl (semid, 0, GETVAL, argument);
            value2 = semctl (semid, 1, GETVAL, argument);
            time(&t);
            sprintf(timeArr,"%s ",ctime(&t));
            timeArr[strlen(timeArr)-2] = '\0';
            printf("%s Supplier: read from input a ‘2’. Current amounts: %d x ‘1’, %d x ‘2’.\n",timeArr,value1,value2);
            sempostTwo(semid);
            value1 = semctl (semid, 0, GETVAL, argument);
            value2 = semctl (semid, 1, GETVAL, argument);
            time(&t);
            sprintf(timeArr,"%s ",ctime(&t));
            timeArr[strlen(timeArr)-2] = '\0';
            printf("%s Supplier: delivered a ‘2’. Post-delivery amounts: %d x ‘1’, %d x ‘2’.\n",timeArr,value1,value2);
        }
        currentIndex++;
    }
    time(&t);
    sprintf(timeArr,"%s ",ctime(&t));
    timeArr[strlen(timeArr)-2] = '\0';
    printf("%s The Supplier has left.\n",timeArr);
    close(fd);
    return NULL;
}

void *ConsumerFd(void *arg){
    int threadNumber;
    int value1,value2;
    union semun argument;
    char timeArr[100];

    threadNumber = *((int *)(arg));
    time_t t;  

    for(int j = 0; j < nValue; j++){
        value1 = semctl (semid, 0, GETVAL, argument);
        value2 = semctl (semid, 1, GETVAL, argument);

        
        time(&t);
        sprintf(timeArr,"%s ",ctime(&t));
        timeArr[strlen(timeArr)-2] = '\0';
        printf("%s Consumer-%d at iteration %d (waiting). Current amounts: %d x ‘1’, %d x ‘2'.\n",timeArr,threadNumber,j,value1,value2);

        semwaitOneAndTwo(semid);
        value1 = semctl (semid, 0, GETVAL, argument);
        value2 = semctl (semid, 1, GETVAL, argument);
        
        time(&t);
        sprintf(timeArr,"%s ",ctime(&t));
        timeArr[strlen(timeArr)-2] = '\0';
        printf("%s Consumer-%d at iteration %d (consumed). Post-consumption amounts: %d x ‘1’, %d x‘2’.\n",timeArr,threadNumber,j,value1,value2);
        
    }
    time(&t);
    sprintf(timeArr,"%s ",ctime(&t));
    timeArr[strlen(timeArr)-2] = '\0';
    printf("%s Consumer-%d has left.\n",timeArr,threadNumber);
    

    return NULL;
}
void handler(int sig_num){
    union semun argument;

    printf("Exiting the program... (Because of CTRL+C interrupt)\n");
    semctl(semid, 0, IPC_RMID, argument);
    semctl(semid, 1, IPC_RMID, argument);
    close(fdInput);
    exit(EXIT_FAILURE);
}

int main(int argc , char *argv[]){

    char *inputFile = NULL;
    char tempChar;
    void *res;

    setbuf(stdout,NULL);

    struct sigaction sa;
    memset (&sa,0,sizeof(sa));
    sa.sa_handler = &handler;
    sigaction(SIGINT, &sa,NULL);
  
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    ErrorCheckArgNum(argc);
    int opt=0,cFlag=0,nFlag=0,fFlag=0;
    while((opt = getopt(argc, argv, "C:N:F:")) != -1) {  
        switch(opt) { 
            case 'C': 
                cFlag++;
                consumerNumber = atoi(optarg);
                break;
            case 'N': 
                nFlag++;
                nValue = atoi(optarg);
                break;
            case 'F': 
                fFlag++;
                inputFile = optarg;
                break;
            default:
                PrintMessage("Invalid Argument \n");
                break;
        }
    }  
    if(cFlag > 1 || nFlag > 1 || fFlag > 1)
        PrintMessage("Each flag can only use one time. \n");

    if(cFlag == 0 || nFlag == 0 || fFlag == 0)
        PrintMessage("Each flag can only use one time. \n");

    if(consumerNumber < 5 || nValue < 2){
        PrintMessage("Please enter the C and N parameters as specified.\n");
    }
    int conArr[consumerNumber];
    fdInput = open(inputFile, O_RDONLY, mode);
    fileSize=consumerNumber*nValue;
                                                    /************Error Control**************/
    int realFileSize=0;
    while(read(fdInput,&tempChar,1) != '\0'){
        if(tempChar == '1' || tempChar == '2')
            realFileSize++;
    }
    if(realFileSize != fileSize*2){
         PrintMessage("The file size and the value range do not match. Please check again.\n");
    }
    close(fdInput);

    fdInput = open(inputFile, O_RDONLY, mode);
                                                                /*********Semaphore*************/
    if((semid = semget(IPC_PRIVATE, 2, IPC_CREAT | 0666)) == -1){
        fprintf(stderr, "Err semget\n");
        exit(EXIT_FAILURE);
    }
    if(binary_semaphore_initialize(semid) == -1){
        fprintf(stderr, "semctl\n");
        exit(EXIT_FAILURE); 
    }

    int error;
    int s;
    pthread_t tidSup, *tidCons;
    pthread_attr_t attr;

    tidCons = malloc(consumerNumber * sizeof(pthread_t));
                                                         /**********Supplier************/
    s = pthread_attr_init(&attr);
    if(s != 0)
         PrintMessage("The thread could not join.");

    s = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(s != 0)
         PrintMessage("The thread could not join.");
    
    if ((error = pthread_create(&tidSup, &attr, SupplierFd, &fdInput)))
        PrintMessage("Failed to create thread.");
                                                        /*************Consumer*************/

    for(int i = 0; i<consumerNumber; i++){
        conArr[i] = i;
        if ((error = pthread_create(&tidCons[i], NULL, ConsumerFd, &conArr[i])))
            PrintMessage("Failed to create thread.");
    }
    for(int i=0; i<consumerNumber; i++){
        s = pthread_join(tidCons[i], &res);
        if(s != 0)
            PrintMessage("The thread could not join.");
    }
    pthread_attr_destroy(&attr);
    free(tidCons);
    
    pthread_exit(0);
}