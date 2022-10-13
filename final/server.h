void ErrorCheck(int pFlag, int tFlag, int argc);
void ParseCommandLine(int argc,char *argv[]);
void *MyServerThread(void *arg);
void handler(int sig_num);
void freeVar();