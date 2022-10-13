typedef struct request {
    char transactionCount[64];
    char realEstate[128];
    char endDate[128];
    char startDate[128];
    int startDay;
    int startMonth;
    int startYear;
    int endDay;
    int endMonth;
    int endYear;
    char city[128];
    int thrNum;
    int sigNum;
}request;

typedef struct my_servant {
    char responsibleCities[128][64];
    char IP[32];
    int citySize;
    int uniquePort;
}my_servant;

typedef struct common_structure {
    char info[10000];
    int distinct;
}common_structure;

typedef struct result_structure {
    int result;
}result_structure;

void PrintMessage(const char *message);