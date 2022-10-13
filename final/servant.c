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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/sem.h>
#include <dirent.h>
#include "utils.h"
#include "servant.h"
#include "data-structures.h"

char *dirPath=NULL;
int handleCityStart;
int handleCityEnd;
char *IP=NULL;
int PORT;
int allFolderCount=0;
char *transactionArr=NULL;
int transactionRow=0;
struct Queue* queue ;
pthread_mutex_t mutex;
pthread_cond_t cond ;
pthread_t *tid;
struct node *root;
int tidCount=0;
int socket_arr[1024];
int socketCounter=0;
 int totalRequest=0;
my_servant myServantObj;

void PrintMessage(const char *message){
    write(STDERR_FILENO,message,strlen(message));
    exit(EXIT_FAILURE);
}

void ErrorCheck(int dFlag, int cFlag,int rFlag,int pFlag, int argc){
    if(dFlag > 1 || cFlag > 1 || rFlag > 1 || pFlag > 1){
        fprintf(stderr,"Each flag can only use one time. \n");
    }

    if(dFlag == 0 || cFlag == 0 || rFlag == 0 || pFlag == 0){
        fprintf(stderr,"Each flag can only use one time. \n");
    }
        
	if(argc!=9){
        fprintf(stderr,"Number of arguments is not correct\n");
    }
}

void ParseCommandLine(int argc,char *argv[]){
    
    char *token;
    int opt=0,dFlag=0,cFlag=0,rFlag=0,pFlag=0;

    while((opt = getopt(argc, argv, "d:c:r:p:")) != -1) {  
        switch(opt) { 
            case 'd': 
                dFlag++;
                dirPath = optarg;
                break;
            case 'c': 
                cFlag++;
                token = strtok(optarg, "-");
                handleCityStart = atoi(token);
                token = strtok(NULL, "");
                handleCityEnd = atoi(token);
                break;
            case 'r': 
                rFlag++;
                IP = optarg;
                break;
            case 'p': 
                pFlag++;
                PORT = atoi(optarg);
                break;

            default:
                PrintMessage("Invalid Argument \n");
                break;
        }
    }  
    ErrorCheck(dFlag,cFlag,rFlag,pFlag,argc);
}
void ReadTransaction(char *transactionName,struct linked_list_node *temp){
    int fdTransaction;
    char tempBuffer;
    int col=0;
    int first=0;
    char * token;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

    fdTransaction = open(transactionName, O_RDWR, mode);
    transactionArr = (char*)malloc(sizeof(char));
    temp->realEstate=(char**)malloc(sizeof(char*));
   while(read(fdTransaction,&tempBuffer,1) != '\0'){
        if(tempBuffer=='\n'){
            if(first != 0){
                token = strtok(transactionArr, " ");
                token = strtok(NULL, " ");
                temp->realEstate = (char**)realloc(temp->realEstate,(temp->realEstateSize+1)* sizeof(char*));
                temp->realEstate[temp->realEstateSize]=malloc(strlen(token));
                strcpy(temp->realEstate[temp->realEstateSize],token);
                temp->realEstateSize++;
                free(transactionArr);
            }
            col=0;
            first=0;
        }
        else{
            if(first == 0){
                transactionArr = (char*)malloc(sizeof(char));
            }
            transactionArr = (char*)realloc(transactionArr,(col+1)* sizeof(char));
            transactionArr[col]=tempBuffer;
            col++;
            first++;
            
        }

    }
   close(fdTransaction);
}

void handler(){
    char timeArr[100];
    time_t t;   
    time(&t);
    sprintf(timeArr,"%s ",ctime(&t));
    timeArr[strlen(timeArr)-2] = '\0';
    printf("%s---Servant %d : SIGINT has been received. I handled a total of %d requests. Goodbye.\n",timeArr,myServantObj.uniquePort,totalRequest);
    exit(EXIT_FAILURE);
}
void *MyThread(void *arg){
    struct node *rootTemp;
    int my_new_socket;
    
    my_new_socket=*((int *)(arg));

    request requestObj;
    read(my_new_socket, &requestObj, sizeof(requestObj));
    if(requestObj.sigNum==1){
        handler();
    }
    result_structure result_structure_obj;
    int realEstateCount=0;
    if(strcmp(requestObj.city,"NULL")!=0){
        for(int i=handleCityStart;i<=handleCityEnd;i++){
            rootTemp=search(root,i);
            if(strcmp(rootTemp->cityName,requestObj.city)==0){
                while ((rootTemp->subdirectory->next)!=NULL)
                {
                    if(rootTemp->subdirectory->year>requestObj.startYear && rootTemp->subdirectory->year<requestObj.endYear){
                        for(int j=0;j<rootTemp->subdirectory->realEstateSize;j++){
                            if(strcmp(rootTemp->subdirectory->realEstate[j],requestObj.realEstate)==0){
                                realEstateCount++;
                            }
                        }
                    }
                    else if((rootTemp->subdirectory->year==requestObj.startYear)|| rootTemp->subdirectory->year==requestObj.endYear){
                        if(rootTemp->subdirectory->month>requestObj.startMonth || rootTemp->subdirectory->month<requestObj.startMonth){
                            for(int j=0;j<rootTemp->subdirectory->realEstateSize;j++){
                                if(strcmp(rootTemp->subdirectory->realEstate[j],requestObj.realEstate)==0){
                                    realEstateCount++;
                                }
                            }
                        }
                    }
                    rootTemp->subdirectory=rootTemp->subdirectory->next;
                }
                result_structure_obj.result=realEstateCount;
                break;
            }
        }
    }
    else{
        result_structure_obj.result=27;
    }
    send(my_new_socket, &result_structure_obj, sizeof(result_structure_obj), 0);
    close(my_new_socket);
    return NULL;
}
void Value_Range(){
    if((handleCityStart < 1) || (handleCityEnd > allFolderCount)){
        fprintf(stderr,"Please enter the correct city value range.\n");
        exit(EXIT_FAILURE);
    }
}
int main(int argc , char *argv[]){
    char **allFolder;
    ParseCommandLine(argc,argv);
    allFolder = (char**)malloc(sizeof(char*));
    DIR *d;
    struct dirent *dir;
    struct dirent **namelist;
    int n;
    n = scandir(dirPath, &namelist, NULL, alphasort);
    allFolder = (char**)malloc((n-2)*sizeof(char*));
    allFolderCount=n-2;
    
    Value_Range();
    if (n < 0){
        fprintf(stderr,"scandir");
    }
    else {
        while (n--) {
            if(n==1)
                break;
            allFolder[n-2] = (char*)malloc(strlen(namelist[n]->d_name)*sizeof(char));
            strcpy(allFolder[n-2],namelist[n]->d_name);
            free(namelist[n]);
        }
        free(namelist);
    }
    for(int cityId=1;cityId<=allFolderCount;cityId++){
        if((cityId>= handleCityStart) && (cityId<= handleCityEnd)){
            if(cityId==handleCityStart)
                root = new_node(cityId,allFolder[cityId-1]);
            else    
                insert(root,cityId,allFolder[cityId-1]);
        }
    }
    struct node *rootTemp;
    char *targetDir;
    char targetDirSub[512];
    char slash = '/';
    int targetDirSize=0;
    char * token;
    char temp[64];
    for(int i=handleCityStart;i<=handleCityEnd;i++){
        rootTemp = search(root,i);
        targetDirSize = strlen(rootTemp->cityName) +
                                    strlen(dirPath)+1;
        targetDir = (char*)malloc(targetDirSize);
        sprintf(targetDir,"%s%c%s",dirPath,slash,rootTemp->cityName);
        d = opendir(targetDir);
        if (d) {
            struct linked_list_node *temp2 = rootTemp->subdirectory ;
            while ((dir = readdir(d)) != NULL) {
                if((strcmp(dir->d_name,".") != 0) && (strcmp(dir->d_name,"..") != 0)){
                    rootTemp->subdirectory->next=NULL;
                    rootTemp->subdirectory->next=malloc(sizeof(struct linked_list_node));
                    strcpy(rootTemp->subdirectory->fileName,dir->d_name);
                    strcpy(temp,dir->d_name);
                    token = strtok(temp, "-");
                    rootTemp->subdirectory->day=atoi(token);
                    token = strtok(NULL, "-");
                    rootTemp->subdirectory->month=atoi(token);
                    token = strtok(NULL, "");
                    rootTemp->subdirectory->year=atoi(token);
                    sprintf(targetDirSub,"%s%c%s",targetDir,slash,dir->d_name);
                    rootTemp->subdirectory->realEstateSize=0;
                    ReadTransaction(targetDirSub,rootTemp->subdirectory);
                    rootTemp->subdirectory=rootTemp->subdirectory->next;

                }
            }
            rootTemp->subdirectory->next=NULL;
            rootTemp->subdirectory = temp2;
            closedir(d);
        }
        else{
            fprintf(stderr,"opendir");
            closedir(d);
            exit(EXIT_FAILURE);
        }
    
    }
    int counter=0;
    for(int i=handleCityStart;i<=handleCityEnd;i++){
        rootTemp=search(root,i);
        strcpy(myServantObj.responsibleCities[counter],rootTemp->cityName);
        counter++;
    }
    myServantObj.citySize=handleCityEnd-handleCityStart+1;
    myServantObj.uniquePort=PORT+handleCityEnd+handleCityStart+1;
    strcpy(myServantObj.IP,IP);
    int sockFd = 0, servantFd;
    struct sockaddr_in serverAddr;
    if ((sockFd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr,"Socket creation error \n");
    }
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, IP, &serverAddr.sin_addr)
        <= 0) {
            fprintf(stderr,"Invalid address/ Address not supported \n");
    }
    while ((servantFd = connect(sockFd, (struct sockaddr*)&serverAddr,
                                                        sizeof(serverAddr)))
                                                                        < 0) {
        close(servantFd);
    }

    rootTemp = search(root,handleCityStart);
    struct node *rootTemp2 = search(root,handleCityEnd);
    char timeArr[100];
    time_t t;   
    time(&t);
    sprintf(timeArr,"%s ",ctime(&t));
    timeArr[strlen(timeArr)-2] = '\0';
    printf("%s---Servant %d : loaded dataset, cities %s-%s\n",timeArr,myServantObj.uniquePort,rootTemp->cityName,rootTemp2->cityName);


    common_structure common_structure_obj;
    common_structure_obj.distinct=2;
    memcpy(common_structure_obj.info,&myServantObj,sizeof(myServantObj));
    send(sockFd, (char *)&common_structure_obj, sizeof( common_structure_obj ), 0);
    close(servantFd);

    

                                                 /*********OKUMAYA GEÇTİ*********/
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int error;
    int s;
    pthread_attr_t attr;
     queue = createQueue(1000);
     if (pthread_mutex_init(&mutex, NULL) != 0) {
        PrintMessage("Server mutex init has failed");
    }

    s = pthread_attr_init(&attr);
    if(s != 0)
        fprintf(stderr,"The thread could not join.");
        
    s = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(s != 0)
        fprintf(stderr,"The thread could not join.");

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr,"Socket creation error \n");
    }
 
    if (setsockopt(server_fd, SOL_SOCKET,
                   SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt))) {
        fprintf(stderr,"setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(myServantObj.uniquePort);
    tidCount++;
   
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
                                        /*********************LISTENING**********************/
   
    time(&t);
    sprintf(timeArr,"%s ",ctime(&t));
    timeArr[strlen(timeArr)-2] = '\0';
    printf("%s---Servant %d : listening at port %d\n",timeArr,myServantObj.uniquePort,myServantObj.uniquePort);

    tid=malloc(sizeof(sizeof(pthread_t)));
    while(1){
        tid=realloc(tid,(tidCount+1)*sizeof(pthread_t));
        if ((socket_arr[socketCounter]
            = accept(server_fd, (struct sockaddr*)&address,
                  (socklen_t*)&addrlen))< 0) {
                fprintf(stderr,"accept");
             exit(EXIT_FAILURE);
        }
        if ((error = pthread_create(&tid[tidCount], NULL, MyThread,  &socket_arr[socketCounter])))
            PrintMessage("Failed to create thread.");
        tidCount++;
        socketCounter++;
        totalRequest++;
    }

    pthread_exit(0);

    exit(EXIT_FAILURE);
    
   
    return 0;
}