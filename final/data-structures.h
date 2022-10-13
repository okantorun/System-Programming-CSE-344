
struct node
{
    int cityId; 
    char cityName[128];
    struct linked_list_node *subdirectory;
    struct node *right_child; 
    struct node *left_child; 
};
struct linked_list_node {
    int realEstateSize;
    char fileName[32];
    int day;
    int month;
    int year;
    char **realEstate;
    struct linked_list_node* next;
};
struct Queue {
    int front, rear, size;
    unsigned capacity;
    int* array;
};

struct node* search(struct node *root, int cityId);
struct node* new_node(int cityId,char *cityName);
struct node* insert(struct node *root, int cityId, char *cityName);
struct Queue* createQueue(unsigned capacity);
int isFull(struct Queue* queue);
int isEmpty(struct Queue* queue);
void enqueue(struct Queue* queue, int item);
int dequeue(struct Queue* queue);
int front(struct Queue* queue);
int rear(struct Queue* queue);