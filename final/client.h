void ErrorCheck(int rFlag, int qFlag, int sFlag, int argc);
void ParseCommandLine(int argc,char *argv[]);
void ReadRequestFile();
void WriteToStruct(request *req);
void *MyThread(void *arg);