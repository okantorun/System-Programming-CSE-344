struct request{
    pid_t pid;
    char matrix[1024];
};

struct response{
    int matrixSize;
    int state;
};