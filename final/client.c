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
#include <sys/socket.h>
#include <arpa/inet.h>
#include "utils.h"
#include "client.h"

#define REQUEST_TYPE 5

char **requestInfo=NULL;
char *requestFile=NULL,*IP=NULL;
int PORT;
int fdRequest;
char **reqArr;
int reqSize=0;
pthread_t *tid;
pthread_mutex_t mutex;
pthread_cond_t cond ;
int arrived=0;
request *req;

void PrintMessage(const char *message){
    write(STDERR_FILENO,message,strlen(message));
    exit(EXIT_FAILURE);
}

void ErrorCheck(int rFlag, int qFlag, int sFlag, int argc){
    if(rFlag > 1 || qFlag > 1 || sFlag > 1){
        fprintf(stderr,"Each flag can only use one time. \n");
    }
    if(rFlag == 0 || qFlag == 0 || sFlag == 0){
        fprintf(stderr,"Each flag can only use one time. \n");
    }
	if(argc!=7){
        fprintf(stderr,"Number of arguments is not correct \n");
        
    }        
}

void ParseCommandLine(int argc,char *argv[]){

    int opt=0,rFlag=0,qFlag=0,sFlag=0;
    while((opt = getopt(argc, argv, "r:q:s:")) != -1) {  
        switch(opt) { 
            case 'r': 
                rFlag++;
                requestFile = optarg;
                break;
            case 'q': 
                qFlag++;
                PORT = atoi(optarg);
                break;
            case 's': 
                sFlag++;
                IP = optarg;
                break;
            default:
                PrintMessage("Invalid Argument \n");
                break;
        }
    }  
    ErrorCheck(rFlag,qFlag,sFlag,argc);
}

void ReadRequestFile(){
    char tempBuffer;
    int col=0;
    int row=0;
    int first=0;

    reqArr = (char**)malloc(sizeof(char*));
    reqArr[0] = (char*)malloc(sizeof(char));
    
    while(read(fdRequest,&tempBuffer,1) != '\0'){
        if(tempBuffer=='\n'){
            if(first != 0){
                reqArr[row][col]=' ';
                row++;reqSize++;
            }
            col=0;
            first=0;
        }
        else{
            if(first == 0){
                reqArr = (char**)realloc(reqArr,(row+1)* sizeof(char*));
            }
            reqArr[row] = (char*)realloc(reqArr[row],(col+1)* sizeof(char));
            reqArr[row][col] = tempBuffer;
            col++;
            first++;
            
        }

    }
    close(fdRequest);
}

void WriteToStruct(request *req){
    int reqCount=0;
    char * token;   
    while(reqCount<reqSize){
        for(int col=0 ; col<REQUEST_TYPE ;col++){
            if(col % 5 == 0){
                token = strtok(reqArr[reqCount], " ");
                strcpy(req[reqCount].transactionCount,token);
            }
            else if(col % 5 == 1){
                token = strtok(NULL, " ");
                strcpy(req[reqCount].realEstate,token);
            }
            else if(col % 5 == 2){
                token = strtok(NULL, " ");
                strcpy(req[reqCount].startDate,token);
            }
            else if(col % 5 == 3){
                token = strtok(NULL, " ");
                strcpy(req[reqCount].endDate,token);
            }
            else if(col % 5 == 4){
                token = strtok(NULL, " ");
                if(token == NULL){strcpy(req[reqCount].city,"NULL");}
                else{
                    strcpy(req[reqCount].city,token);
                    
                }
            }
        }
                                                /*****START DATE********/
        char temp[64];
        strcpy(temp,req[reqCount].startDate);
        token = strtok(temp, "-");
        req[reqCount].startDay=atoi(token);
        token = strtok(NULL, "-");
        req[reqCount].startMonth=atoi(token);
        token = strtok(NULL, "");
        req[reqCount].startYear=atoi(token);
                                                 /*****END DATE********/
        strcpy(temp,req[reqCount].endDate);                                        
        token = strtok(temp, "-");
        req[reqCount].endDay=atoi(token);
        token = strtok(NULL, "-");
        req[reqCount].endMonth=atoi(token);
        token = strtok(NULL, "");
        req[reqCount].endYear=atoi(token);

        reqCount++;
    }
}

void *MyThread(void *arg){
    char timeArr[100];
    time_t t;   
    int sockFd = 0, clientFd;
    struct sockaddr_in serverAddr; 

    request requestObj = *((struct request*)arg);
    if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr,"Socket creation error \n");
        
    }
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, IP, &serverAddr.sin_addr)
        <= 0) {
         fprintf(stderr,"Invalid address/ Address not supported \n");   
    }
                                                    /*******synchronization*******/
    pthread_mutex_lock(&mutex);
    ++arrived;
    while(arrived < reqSize)
    {   
        pthread_cond_wait(&cond, &mutex);
    }    
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    while ((clientFd = connect(sockFd, (struct sockaddr*)&serverAddr,
                                                        sizeof(serverAddr)))
                                                                        < 0) {
        close(clientFd);
    }
    time(&t);
    sprintf(timeArr,"%s ",ctime(&t));
    timeArr[strlen(timeArr)-2] = '\0';
    if(strcmp(requestObj.city,"NULL")==0){
        printf("%s---Client-Thread-%d: I am requesting transactionCount %s %s %s\n",timeArr,requestObj.thrNum,
                                                                        requestObj.realEstate,requestObj.startDate,requestObj.endDate);
                                                        
    }
    else{
        printf("%s---Client-Thread-%d: I am requesting transactionCount %s %s %s %s\n",timeArr,requestObj.thrNum,
                                                                        requestObj.realEstate,requestObj.startDate,requestObj.endDate,requestObj.city);
    
    }

    common_structure common_structure_obj;
    common_structure_obj.distinct=1;
    memcpy(common_structure_obj.info,&requestObj,sizeof(requestObj));
    send(sockFd, (char *)&common_structure_obj, sizeof(common_structure_obj), 0);

    result_structure result_structure_obj;
    read(sockFd, &result_structure_obj, sizeof(result_structure_obj));
    time(&t);
    sprintf(timeArr,"%s ",ctime(&t));
    timeArr[strlen(timeArr)-2] = '\0';
    if(strcmp(requestObj.city,"NULL")==0){
        printf("%s---Client-Thread-%d: The server’s response to “/transactionCount %s %s %s” is %d\n",timeArr,requestObj.thrNum,requestObj.realEstate,requestObj.startDate,
                                                                                    requestObj.endDate,result_structure_obj.result);
    }
    else{
        printf("%s---Client-Thread-%d: The server’s response to “/transactionCount %s %s %s %s” is %d\n",timeArr,requestObj.thrNum,requestObj.realEstate,requestObj.startDate,
                                                                                    requestObj.endDate,requestObj.city,result_structure_obj.result);
    }
    
    printf("%s---Client-Thread-%d: Terminating\n",timeArr,requestObj.thrNum);
    close(clientFd);
    return NULL;
}
void handler(int sig_num){
    
    for(int i=0;i<reqSize;i++){
        free(reqArr[i]);
    }
    free(reqArr);
    free(tid);
    free(req);
     exit(EXIT_FAILURE);
}

int main(int argc , char *argv[]){
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;
    int error;
    int s;
    void *res;

    struct sigaction sa;
    memset (&sa,0,sizeof(sa));
    sa.sa_handler = &handler;
    sigaction(SIGINT, &sa,NULL);


    ParseCommandLine(argc,argv);

    if ((fdRequest = open(requestFile, O_RDWR, mode)) < 0) {
        fprintf(stderr,"open");
        exit(EXIT_FAILURE);
    }
    ReadRequestFile();
    req = (request *) malloc(reqSize*sizeof(request));
    WriteToStruct(req);

    char timeArr[100];
    time_t t;   
    time(&t);
    sprintf(timeArr,"%s ",ctime(&t));
    timeArr[strlen(timeArr)-2] = '\0';
    printf("%s---Client: I have loaded %d requests and I’m creating %d threads.\n",timeArr,reqSize,reqSize);
    tid = malloc(reqSize * sizeof(pthread_t));
    for(int i=0;i<reqSize;i++){
        req[i].thrNum = i;
        if ((error = pthread_create(&tid[i], NULL, MyThread, (void*)&req[i]))){
            fprintf(stderr,"Failed to create thread.\n");
            
        }
        printf("%s---Client-Thread-%d: Thread-%d has been created\n",timeArr,i,i);
    }
     for(int i=0; i<reqSize; i++){
        s = pthread_join(tid[i], &res);
        if(s != 0){
            fprintf(stderr,"The thread could not join.\n");
        }
     }
     time(&t);
     sprintf(timeArr,"%s ",ctime(&t));
     timeArr[strlen(timeArr)-2] = '\0';  
     printf("%s---Client: All threads have terminated, goodbye.\n",timeArr);

    exit(EXIT_FAILURE);
    return 0;
    


}