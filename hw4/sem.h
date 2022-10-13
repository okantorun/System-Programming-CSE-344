#ifndef __SEM_H__
#define __SEM_H__

union semun
{
    int val;
    struct semid_ds * buf ;
    unsigned short* array;
    struct seminfo* __buf;
};
int sempostOne(int semid);
int sempostTwo(int semid);
int semwaitOneAndTwo(int semid);
int binary_semaphore_initialize (int semid);
void PrintMessage(const char *message);
void ErrorCheckArgNum(int argc);
void *SupplierFd(void *arg);
void *ConsumerFd(void *arg);
void handler(int sig_num);



#endif