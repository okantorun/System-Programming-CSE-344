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

#define CHEF_SIZE 6

struct chefs{
    int childsPid;
    int status;
};
void PrintMessage(const char *message);
void ErrorCheckArgNum(int argc);
char* ParseCommandLine(int argc, char **argv,char *inputFile);


void PrintMessage(const char *message){
    write(STDERR_FILENO,message,strlen(message));
    exit(EXIT_FAILURE);
}

void ErrorCheckArgNum(int argc){
	if(argc!=5)
        PrintMessage("Number of arguments is not correct");
}


void* create_shared_memory(size_t size) {
  int protection = PROT_READ | PROT_WRITE;
  int visibility = MAP_SHARED | MAP_ANONYMOUS;
  return mmap(NULL, size, protection, visibility, -1, 0);
}
 sem_t *semaTotalDesert;

void handler(int sig_num){

    printf("Exiting the program... (Because of CTRL+C interrupt)\n");
    exit(EXIT_FAILURE);

}

int main(int argc , char *argv[]){

    struct chefs *objChefs;
    int child_pid=1;
    char *ingredients;
    char *inputFile=NULL;
    char *namedSem=NULL;
    int fdInput;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

    ErrorCheckArgNum(argc);
    int opt=0,iFlag=0,nFlag=0;
    while((opt = getopt(argc, argv, "i:n:")) != -1) {  
        switch(opt) { 
            case 'i': 
                iFlag++;
                inputFile = optarg;
                break;
            case 'n': 
                nFlag++;
                namedSem = optarg;
                break;
        }
    }  
    if(iFlag > 1 || nFlag > 1)
        PrintMessage("Each flag can only use one time. \n");

    if(iFlag == 0 || nFlag > 1)
        PrintMessage("Each flag can only use one time. \n");
   
    objChefs = malloc(CHEF_SIZE * sizeof(struct chefs));

    struct chefs* shmemChefs = (struct chefs *)create_shared_memory(128);
    char* shmem = (char*)create_shared_memory(128);

    
    fdInput = open(inputFile, O_RDONLY, mode);

    struct sigaction sa;
    memset (&sa,0,sizeof(sa));
    sa.sa_handler = &handler;
    sigaction(SIGINT, &sa,NULL);


    semaTotalDesert = sem_open(namedSem,(O_CREAT | O_RDWR),0644,0);
    if ( sem_init(semaTotalDesert, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semaMilk = sem_open("semMilk4",(O_CREAT | O_RDWR),0644,0);
    if ( sem_init(semaMilk, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semaFlour = sem_open("semFlour4",(O_CREAT | O_RDWR),0644,0);
    if ( sem_init(semaFlour, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semaWalnuts = sem_open("semWalnuts4",(O_CREAT | O_RDWR),0644,0);
    if ( sem_init(semaWalnuts, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semaSugar = sem_open("semSugar4",(O_CREAT | O_RDWR),0644,0);
    if ( sem_init(semaSugar, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semaMilkSugar = sem_open("semMS4",(O_CREAT | O_RDWR),0644,0);
    if ( sem_init(semaMilkSugar, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semaMilkFlour = sem_open("semMF4",(O_CREAT | O_RDWR),0644,0);
    if ( sem_init(semaMilkFlour, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semaMilkWalnuts = sem_open("semMW4",(O_CREAT | O_RDWR),0644,0);
    if ( sem_init(semaMilkWalnuts, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semaSugarFlour = sem_open("semSF4",(O_CREAT | O_RDWR),0644,0);
    if ( sem_init(semaSugarFlour, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semaFlourWalnuts = sem_open("semFW4",(O_CREAT | O_RDWR),0644,0);
    if ( sem_init(semaFlourWalnuts, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semaWalnutsSugar = sem_open("semWS4",(O_CREAT | O_RDWR),0644,0);
    if ( sem_init(semaWalnutsSugar, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *m = sem_open("m4",(O_CREAT | O_RDWR),0644,0);
    if ( sem_init(m, 1, 1) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *semaAgent = sem_open("semAgent4",(O_CREAT | O_RDWR),0644,0);
    if ( sem_init(semaAgent, 1, 1) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }

    sem_t *tempHandle = sem_open("tempHandle",(O_CREAT | O_RDWR),0644,0);
    if ( sem_init(tempHandle, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    
   

    int isMilk2=0,isFlour2=0,isWalnuts2=0,isSugar2=0;

    sem_t *isMilk = sem_open("semIsMilk4",(O_CREAT | O_RDWR),0644,0);
    if ( sem_init(isMilk, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *isFlour = sem_open("semIsFlour4",(O_CREAT | O_RDWR),0644,0);
    if ( sem_init(isFlour, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *isWalnuts = sem_open("semIsSugar4",(O_CREAT | O_RDWR),0644,0);
    if ( sem_init(isWalnuts, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    sem_t *isSugar = sem_open("semIsWalnuts4",(O_CREAT | O_RDWR),0644,0);
    if ( sem_init(isSugar, 1, 0) < 0) {
        perror("sem_init");
        exit(EXIT_FAILURE);
    }
    
    
   
    int totalDesert;
    for(int i = 0; i < 10; i++) 
    {
        child_pid=fork();
        if(child_pid==0){
                if(i==5){
                    printf("chef%d (pid %d) is waiting for M and S\n",i,getpid());
                    int desert=0;
                    while(1){
                        sem_wait(semaMilkSugar);
                         int temp;
                        sem_getvalue(tempHandle,&temp);
                        if(temp==1){
                            printf("chef%d (pid %d) is exiting\n",i,getpid());
                            exit(desert);
                        }
                        printf("\nchef%d (pid %d) has taken the %c\n",i,getpid(),shmem[0]);
                        printf("chef%d (pid %d) has taken the %c\n",i,getpid(),shmem[1]);
                        printf("chef%d (pid %d) is preparing the dessert\n",i,getpid());
                        printf("chef%d (pid %d) has delivered the dessert\n",i,getpid());
                        printf("%s\n",shmem);
                        sem_post(semaTotalDesert);
                        sem_post(semaAgent);
                        desert++;
                    }
                }
                else if(i==3){
                    printf("chef%d (pid %d) is waiting for M and F\n",i,getpid());
                    int desert=0;
                    while(1){
                        sem_wait(semaMilkFlour);
                        int temp;
                        sem_getvalue(tempHandle,&temp);
                        if(temp==1){
                            printf("chef%d (pid %d) is exiting\n",i,getpid());
                            exit(desert);
                        }
                        printf("\nchef%d (pid %d) has taken the %c\n",i,getpid(),shmem[0]);
                        printf("chef%d (pid %d) has taken the %c\n",i,getpid(),shmem[1]);
                        printf("chef%d (pid %d) is preparing the dessert\n",i,getpid());
                        printf("chef%d (pid %d) has delivered the dessert\n",i,getpid());
                        printf("%s\n",shmem);
                        sem_post(semaTotalDesert);
                        sem_post(semaAgent);
                        desert++;
                    }
                    
                }
                else if(i==4){
                    printf("chef%d (pid %d) is waiting for M and W\n",i,getpid());
                    int desert=0;
                    while(1){
                        sem_wait(semaMilkWalnuts);
                         int temp;
                        sem_getvalue(tempHandle,&temp);
                        if(temp==1){
                            printf("chef%d (pid %d) is exiting\n",i,getpid());
                            exit(desert);
                        }
                        printf("\nchef%d (pid %d) has taken the %c\n",i,getpid(),shmem[0]);
                        printf("chef%d (pid %d) has taken the %c\n",i,getpid(),shmem[1]);
                        printf("chef%d (pid %d) is preparing the dessert\n",i,getpid());
                        printf("chef%d (pid %d) has delivered the dessert\n",i,getpid());
                        printf("%s\n",shmem);
                        sem_post(semaTotalDesert);
                        sem_post(semaAgent);
                        desert++;
                        
                    }
                }
                else if(i==0){
                    printf("chef%d (pid %d) is waiting for W and S\n",i,getpid());
                    int desert=0;
                    while(1){
                        sem_wait(semaWalnutsSugar);
                         int temp;
                        sem_getvalue(tempHandle,&temp);
                        if(temp==1){
                            printf("chef%d (pid %d) is exiting\n",i,getpid());
                            exit(desert);
                        }
                        printf("\nchef%d (pid %d) has taken the %c\n",i,getpid(),shmem[0]);
                        printf("chef%d (pid %d) has taken the %c\n",i,getpid(),shmem[1]);
                        printf("chef%d (pid %d) is preparing the dessert\n",i,getpid());
                        printf("chef%d (pid %d) has delivered the dessert\n",i,getpid());
                        printf("%s\n",shmem);
                        sem_post(semaTotalDesert);
                        sem_post(semaAgent);
                        desert++;
                    }
                }
                else if(i==2){
                    printf("chef%d (pid %d) is waiting for S and F\n",i,getpid());
                    int desert=0;
                    while(1){
                        sem_wait(semaSugarFlour);
                         int temp;
                        sem_getvalue(tempHandle,&temp);
                        if(temp==1){
                            printf("chef%d (pid %d) is exiting\n",i,getpid());
                            exit(desert);
                        }
                        printf("\nchef%d (pid %d) has taken the %c\n",i,getpid(),shmem[0]);
                        printf("chef%d (pid %d) has taken the %c\n",i,getpid(),shmem[1]);
                        printf("chef%d (pid %d) is preparing the dessert\n",i,getpid());
                        printf("chef%d (pid %d) has delivered the dessert\n",i,getpid());
                        printf("%s\n",shmem);
                        sem_post(semaTotalDesert);
                        sem_post(semaAgent);
                        desert++;
                    }
                }
                else if(i==1){
                    printf("chef%d (pid %d) is waiting for F and W\n",i,getpid());
                    int desert=0;
                    while(1){
                        sem_wait(semaFlourWalnuts);
                         int temp;
                        sem_getvalue(tempHandle,&temp);
                        if(temp==1){
                            printf("chef%d (pid %d) is exiting\n",i,getpid());
                            exit(desert);
                        }
                        printf("\nchef%d (pid %d) has taken the %c\n",i,getpid(),shmem[0]);
                        printf("chef%d (pid %d) has taken the %c\n",i,getpid(),shmem[1]);
                        printf("chef%d (pid %d) is preparing the dessert\n",i,getpid());
                        printf("chef%d (pid %d) has delivered the dessert\n",i,getpid());
                        printf("%s\n",shmem);
                        sem_post(semaTotalDesert);
                        sem_post(semaAgent);
                        desert++;
                    }
                }
                else if(i==6){
                    while(1){
                        sem_wait(semaMilk);
                         int temp;
                        sem_getvalue(tempHandle,&temp);
                        if(temp==1){
                            exit(EXIT_FAILURE);
                        }
                        sem_wait(m);
                        sem_getvalue(isFlour,&isFlour2);
                        sem_getvalue(isWalnuts,&isWalnuts2);
                        sem_getvalue(isSugar,&isSugar2);
                        if(isFlour2){
                            sem_wait(isFlour);
                            sem_post(semaMilkFlour);
                        }
                        else if(isWalnuts2){
                            sem_wait(isWalnuts);
                            sem_post(semaMilkWalnuts);
                        }
                        else if(isSugar2){
                           sem_wait(isSugar);
                            sem_post(semaMilkSugar);
                        }
                        else{
                            sem_post(isMilk);
                        }
                        sem_post(m);

                    }
                }
                else if(i==7){
                    while(1){
                        sem_wait(semaFlour);
                         int temp;
                        sem_getvalue(tempHandle,&temp);
                        if(temp==1){
                            exit(EXIT_FAILURE);
                        }
                        sem_wait(m);
                        sem_getvalue(isMilk,&isMilk2);
                        sem_getvalue(isWalnuts,&isWalnuts2);
                        sem_getvalue(isSugar,&isSugar2);
                        if(isMilk2){
                            sem_wait(isMilk);
                            sem_post(semaMilkFlour);
                        }
                        else if(isWalnuts2){
                            sem_wait(isWalnuts);
                            sem_post(semaFlourWalnuts);
                        }
                        else if(isSugar2){
                            sem_wait(isSugar);
                            sem_post(semaSugarFlour);
                        }
                        else{
                            sem_post(isFlour);
                        }
                        sem_post(m);

                    }
                }
                else if(i==8){
                    while(1){
                        sem_wait(semaWalnuts);
                         int temp;
                        sem_getvalue(tempHandle,&temp);
                        if(temp==1){
                            exit(EXIT_FAILURE);
                        }
                        sem_wait(m);
                        sem_getvalue(isMilk,&isMilk2);
                        sem_getvalue(isFlour,&isFlour2);
                        sem_getvalue(isSugar,&isSugar2);
                        if(isMilk2){
                            sem_wait(isMilk);
                            sem_post(semaMilkWalnuts);
                        }
                        else if(isFlour2){
                            sem_wait(isFlour);
                            sem_post(semaFlourWalnuts);
                        }
                        else if(isSugar2){
                            sem_wait(isSugar);
                            sem_post(semaWalnutsSugar);
                        }
                        else{
                            sem_post(isWalnuts);
                        }
                        sem_post(m);
                    }
                }
                else if(i==9){
                    while(1){
                        sem_wait(semaSugar);
                         int temp;
                        sem_getvalue(tempHandle,&temp);
                        if(temp==1){
                            exit(EXIT_FAILURE);
                        }
                        sem_wait(m);
                        sem_getvalue(isMilk,&isMilk2);
                        sem_getvalue(isFlour,&isFlour2);
                        sem_getvalue(isWalnuts,&isWalnuts2);
                        if(isFlour2){
                            sem_wait(isFlour);
                            sem_post(semaSugarFlour);

                        }
                        else if(isWalnuts2){
                            sem_wait(isWalnuts);
                            sem_post(semaWalnutsSugar);
                        }
                        else if(isMilk2){
                            sem_wait(isMilk);
                            sem_post(semaMilkSugar);
                            
                        }
                        else{
                            sem_post(isSugar);
                        }
                        sem_post(m);
                    }
                }
        }
        else{
            if(i<=5){
                objChefs[i].childsPid = child_pid;
                memcpy(shmemChefs,objChefs,CHEF_SIZE*sizeof(struct chefs*));
            }
        }

    }
    int desertCounter=0;
    while(1){
        char tempBuffer;
        sem_getvalue(semaTotalDesert,&totalDesert);
        sem_wait(semaAgent);
        if(totalDesert>0){
             printf("\nthe wholesaler (pid %d) has obtained the dessert and left\n",getpid());
        }
        ingredients = malloc(2);
        read(fdInput,ingredients,2);
        if(ingredients[0] != 'S' && ingredients[0] != 'M' && ingredients[0] != 'W' && ingredients[0] != 'F' ){
            close(fdInput);
            int status,total=0;
            sem_post(tempHandle);
            sem_post(semaMilk);
            sem_post(semaFlour);
            sem_post(semaWalnuts);
            sem_post(semaSugar);
            sem_post(semaMilkSugar);
            sem_post(semaMilkFlour);
            sem_post(semaMilkWalnuts);
            sem_post(semaSugarFlour);
            sem_post(semaWalnutsSugar);
            sem_post(semaFlourWalnuts);
            sem_post(m);
            sem_post(semaAgent);
            sem_post(isMilk);
            sem_post(isFlour);
            sem_post(isWalnuts);
            sem_post(isSugar);
            
            for(int i=0;i<CHEF_SIZE;i++){
                waitpid(shmemChefs[i].childsPid,&status,0);
                total += WEXITSTATUS(status);
            }
            printf("\nthe wholesaler (pid %d) is done (total desserts: %d)\n",getpid(),total);

            sem_unlink("tempHandle");
            sem_unlink("semMilk4");
            sem_unlink("semFlour4");
            sem_unlink("semWalnuts4");
            sem_unlink("semSugar4");
            sem_unlink("semMS4");
            sem_unlink("semMF4");
            sem_unlink("semMW4");
            sem_unlink("semSF4");
            sem_unlink("semFW4");
            sem_unlink("semWS4");
            sem_unlink("m4");
            sem_unlink("semIsMilk4");
            sem_unlink("semIsFlour4");
            sem_unlink("semIsSugar4");
            sem_unlink("semIsWalnuts4");
            sem_unlink("semAgent4");
            sem_unlink(namedSem);

            sem_close(tempHandle);
            sem_close(semaTotalDesert);
            sem_close(semaMilk);
            sem_close(semaFlour);
            sem_close(semaWalnuts);
            sem_close(semaSugar);
            sem_close(semaMilkSugar);
            sem_close(semaMilkFlour);
            sem_close(semaMilkWalnuts);
            sem_close(semaSugarFlour);
            sem_close(semaFlourWalnuts);
            sem_close(semaWalnutsSugar);
            sem_close(m);
            sem_close(semaAgent);
            sem_close(isMilk);
            sem_close(isFlour);
            sem_close(isWalnuts);
            sem_close(isSugar);

            free(ingredients);
            free(objChefs);

            exit(EXIT_FAILURE);

        }
        read(fdInput,&tempBuffer,1);
        
        printf("\nthe wholesaler (pid %d) delivers %c and %c\n",getpid(),ingredients[0],ingredients[1]);
        printf("the wholesaler (pid %d) is waiting for the dessert\n",getpid());
        desertCounter++;

        memcpy(shmem, ingredients, strlen(ingredients)+1);

        if(ingredients[0] == 'M'){sem_post(semaMilk);}
        else if(ingredients[0] == 'S'){sem_post(semaSugar);}
        else if(ingredients[0] == 'F'){sem_post(semaFlour);}
        else if(ingredients[0] == 'W'){sem_post(semaWalnuts);}
        if(ingredients[1] == 'M'){sem_post(semaMilk);}
        else if(ingredients[1] == 'S'){sem_post(semaSugar);}
        else if(ingredients[1] == 'F'){sem_post(semaFlour);}
        else if(ingredients[1] == 'W'){sem_post(semaWalnuts);}
    }

    return 0;

}
