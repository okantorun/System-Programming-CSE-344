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
#include <netinet/in.h>
#include <limits.h>
#include "utils.h"
#include "server.h"
#include "data-structures.h"

#define TRUE 1

int PORT;
int numberOfThreads;
pthread_t *tid;
struct Queue* queue ;
pthread_mutex_t mutex;
pthread_cond_t cond ;
my_servant *servantObj;
int servantCount=0;
request *requestObj;
int socket_arr[1024];
int socketCounter=0;
int responseCounter=0;
int handlerTemp=0;
int *my_thr_num;

void PrintMessage(const char *message){
    write(STDERR_FILENO,message,strlen(message));
    exit(EXIT_FAILURE);
}

void ErrorCheck(int pFlag, int tFlag, int argc){
    if(pFlag > 1 || tFlag > 1){
        PrintMessage("Each flag can only use one time. \n");
    }
    if(pFlag == 0 || tFlag == 0){
        PrintMessage("Each flag can only use one time. \n");
    }
        
    if(numberOfThreads<5){
        PrintMessage("You must create at least 5 threads!");
    }

	if(argc!=5){
         PrintMessage("Number of arguments is not correct");
    }
       
}

void ParseCommandLine(int argc,char *argv[]){
    int opt=0,pFlag=0,tFlag=0;
    while((opt = getopt(argc, argv, "p:t:")) != -1) {  
        switch(opt) { 
            case 'p': 
                pFlag++;
                PORT = atoi(optarg);
                break;

            case 't': 
                tFlag++;
                numberOfThreads = atoi(optarg);
                break;
            
            default:
                PrintMessage("Invalid Argument \n");
                break;
        }
    }  
    ErrorCheck(pFlag,tFlag,argc);
}
int count=0;
void *MyServerThread(void *arg){
    char timeArr[100];
    time_t t;   
    common_structure common_structure_obj;
    int my_new_socket;
    int cityControl=0;
    int sockFd = 0, serverFd;
    struct sockaddr_in serverAddr;
    servantObj=malloc(sizeof(my_servant));
    while (TRUE)
    {   
        pthread_mutex_lock(&mutex);
        while(!isEmpty(queue))
        {   
            my_new_socket = dequeue(queue); 
            read(my_new_socket,&common_structure_obj, sizeof( common_structure_obj ));
            if(common_structure_obj.distinct==2){
                servantObj=realloc(servantObj,(servantCount+1)*sizeof(my_servant));
                servantObj[servantCount]=*(struct my_servant*)common_structure_obj.info;
                time(&t);
                sprintf(timeArr,"%s ",ctime(&t));
                timeArr[strlen(timeArr)-2] = '\0';
                printf("%s---Servant %d present at port %d handling cities %s-%s\n",timeArr,servantObj[servantCount].uniquePort,servantObj[servantCount].uniquePort,
                                                                    servantObj[servantCount].responsibleCities[0],servantObj[servantCount].responsibleCities[servantObj->citySize-1]);
                servantCount++;
            }
            else if(common_structure_obj.distinct==1){
                requestObj=(struct request*)common_structure_obj.info;
                if(strcmp(requestObj->city,"NULL") !=0 ){
                    for(int i=0;i<servantCount;i++){
                        for(int j=0;j<servantObj[i].citySize;j++){
                            if(strcmp(servantObj[i].responsibleCities[j],requestObj->city)==0){
                                cityControl++;
                                time(&t);
                                sprintf(timeArr,"%s ",ctime(&t));
                                timeArr[strlen(timeArr)-2] = '\0';
                                printf("%s---Request arrived “transactionCount %s %s %s %s\n",timeArr,requestObj->realEstate,requestObj->startDate,
                                                                                             requestObj->endDate,requestObj->city);
                                printf("%s---Contacting servant %d\n",timeArr,servantObj[i].uniquePort);
                                if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                                    PrintMessage("Socket creation error \n");
                                }
                                serverAddr.sin_family = AF_INET;
                                serverAddr.sin_port = htons(servantObj[i].uniquePort);
                                if (inet_pton(AF_INET, servantObj[i].IP, &serverAddr.sin_addr)
                                    <= 0) {
                                    PrintMessage("Invalid address/ Address not supported \n");
                                }
                                while ((serverFd = connect(sockFd, (struct sockaddr*)&serverAddr,
                                                        sizeof(serverAddr)))
                                                                        < 0) {
                                    close(serverFd);
                                }
                                send(sockFd, requestObj, sizeof(request), 0);
                                
                                result_structure result_structure_obj;
                                read(sockFd, &result_structure_obj, sizeof(result_structure_obj));
                                responseCounter++;
                                time(&t);
                                sprintf(timeArr,"%s ",ctime(&t));
                                timeArr[strlen(timeArr)-2] = '\0';
                                printf("%s---Response received : %d, forwarded to client\n",timeArr,responseCounter);
                                send(my_new_socket, &result_structure_obj, sizeof(result_structure_obj), 0);
                                break;
                            }
                        }
                    }
                    close(serverFd);
                    if(cityControl==0){
                        time(&t);
                        sprintf(timeArr,"%s ",ctime(&t));
                        timeArr[strlen(timeArr)-2] = '\0';
                        printf("%s---The requested %s transaction was not found.\n",timeArr,requestObj->city);
                    }
                    cityControl=0;
                }
                else{
                    time(&t);
                    sprintf(timeArr,"%s ",ctime(&t));
                    timeArr[strlen(timeArr)-2] = '\0';
                    printf("%s---Request arrived “transactionCount %s %s %s\n",timeArr,requestObj->realEstate,requestObj->startDate,requestObj->endDate);
                    printf("%s---Contacting ALL servants\n",timeArr);
                    result_structure result_structure_obj_end;
                    result_structure_obj_end.result=0;
                    for(int i=0;i<servantCount;i++){
                        if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                            fprintf(stderr,"Socket creation error \n");                       
                        }
                        serverAddr.sin_family = AF_INET;
                        serverAddr.sin_port = htons(servantObj[i].uniquePort);
                        if (inet_pton(AF_INET, servantObj[i].IP, &serverAddr.sin_addr)
                            <= 0) {
                                fprintf(stderr,"Invalid address/ Address not supported \n");
                        }
                        while ((serverFd = connect(sockFd, (struct sockaddr*)&serverAddr,
                                                sizeof(serverAddr)))
                                                                < 0) {
                            close(serverFd);
                        }
                        send(sockFd, requestObj, sizeof(request), 0);
                        
                        result_structure result_structure_obj;
                        read(sockFd, &result_structure_obj, sizeof(result_structure_obj));
                        result_structure_obj_end.result+=result_structure_obj.result;
                        close(serverFd);
                    
                    }
                    responseCounter++;
                    time(&t);
                    sprintf(timeArr,"%s ",ctime(&t));
                    printf("%s---Response received : %d, forwarded to client\n",timeArr,responseCounter);
                    send(my_new_socket, &result_structure_obj_end, sizeof(result_structure_obj_end), 0); 
                }
            }
            break;
        }   
        pthread_cond_wait(&cond, &mutex);
        pthread_mutex_unlock(&mutex); 
    }
    
    count++;
}
void freeVar(){
    free(servantObj);
    free(requestObj);
    free(tid);
    free(queue->array);
    free(queue);
    free(my_thr_num);
}
void handler(int sig_num){
    if(sig_num==SIGINT)
    {
        handlerTemp=1;
    }
}
int main(int argc , char *argv[]){
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int error;
    int s;
    pthread_attr_t attr;
    
    struct sigaction sa;
    memset (&sa,0,sizeof(sa));
    sa.sa_handler = &handler;
    sigaction(SIGINT, &sa,NULL);

    queue = createQueue(1000);
    ParseCommandLine(argc,argv);
    my_thr_num=malloc(numberOfThreads*sizeof(int));
    tid = malloc(numberOfThreads * sizeof(pthread_t));

    if (pthread_mutex_init(&mutex, NULL) != 0) {
        fprintf(stderr,"Server mutex init has failed \n");
    }

    s = pthread_attr_init(&attr);
    if(s != 0)
        fprintf(stderr,"The thread could not join. \n");

    s = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(s != 0)
        fprintf(stderr,"The thread could not join. \n");
        
    for(int i = 0; i<numberOfThreads; i++){
        my_thr_num[i]=i;
        if ((error = pthread_create(&tid[i], NULL, MyServerThread,  &my_thr_num[i])))
            fprintf(stderr,"Failed to create thread. \n");
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0))
        == 0) {
        
        fprintf(stderr,"socket failed");
        exit(EXIT_FAILURE);
    }
 
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
       
        fprintf(stderr,"setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
 
   
    if (bind(server_fd, (struct sockaddr*)&address,
             sizeof(address))
        < 0) {
         fprintf(stderr,"bind failed");   
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        fprintf(stderr,"listen");
        exit(EXIT_FAILURE);
    }
    int sockFd,serverFd;
    struct sockaddr_in serverAddr;
    while(TRUE){
        if ((socket_arr[socketCounter]
            = accept(server_fd, (struct sockaddr*)&address,
                  (socklen_t*)&addrlen))< 0) {
            if(handlerTemp==1){
                for(int i=0;i<servantCount;i++){
                    requestObj=malloc(sizeof(requestObj));
                    requestObj[0].sigNum=1;
                    if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                        fprintf(stderr,"Invalid address/ Address not supported \n");
                        PrintMessage("Socket creation error \n");
                    }
                    serverAddr.sin_family = AF_INET;
                    serverAddr.sin_port = htons(servantObj[i].uniquePort);
                    if (inet_pton(AF_INET, servantObj[i].IP, &serverAddr.sin_addr)
                        <= 0) {
                            fprintf(stderr,"Invalid address/ Address not supported \n");
                        
                    }
                    while ((serverFd = connect(sockFd, (struct sockaddr*)&serverAddr,
                                            sizeof(serverAddr)))
                                                            < 0) {
                        close(serverFd);
                    }
                    send(sockFd, &requestObj[0], sizeof(request), 0);
                }
                freeVar();
                exit(EXIT_FAILURE);
            }
            fprintf(stderr,"accept");
            exit(EXIT_FAILURE);
        }

        enqueue(queue, socket_arr[socketCounter]);
        socketCounter++;
        pthread_cond_broadcast(&cond);
    }

   
    close(new_socket);
    shutdown(server_fd, SHUT_RDWR);
    pthread_exit(0);
    
    
}