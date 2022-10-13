#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "data-structures.h"

struct node* search(struct node *root, int cityId)
{
    if(root==NULL || root->cityId==cityId) 
        return root;
    else if(cityId>root->cityId) 
        return search(root->right_child, cityId);
    else 
        return search(root->left_child,cityId);
}

struct node* new_node(int cityId,char *cityName)
{
    struct node *p;
    p = malloc(sizeof(struct node));
    p->subdirectory = malloc(sizeof(struct linked_list_node));
    p->cityId = cityId;
    strcpy(p->cityName,cityName);
    p->left_child = NULL;
    p->right_child = NULL;

    return p;
}

struct node* insert(struct node *root, int cityId, char *cityName)
{
    
    if(root==NULL)
        return new_node(cityId,cityName);
    else if(cityId>root->cityId) 
        root->right_child = insert(root->right_child, cityId, cityName);
    else 
        root->left_child = insert(root->left_child,cityId, cityName);
    return root;
}

struct Queue* createQueue(unsigned capacity)
{
    struct Queue* queue = (struct Queue*)malloc(
        sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
 
    queue->rear = capacity - 1;
    queue->array = (int*)malloc(
        queue->capacity * sizeof(int));
    return queue;
}
 
int isFull(struct Queue* queue)
{
    return (queue->size == queue->capacity);
}
 
int isEmpty(struct Queue* queue)
{
    return (queue->size == 0);
}
 
void enqueue(struct Queue* queue, int item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
}
 
int dequeue(struct Queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1)
                   % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}
 
int front(struct Queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->front];
}
 
int rear(struct Queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->rear];
}
