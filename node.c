#include <stdio.h>
#include <stdlib.h>
typedef struct node
{
    void *p_data;
    struct node *p_next;
    
} node;


node * arr_to_linked_list(void * arr, const unsigned int elem_size, const unsigned int array_size){
    node * head = malloc(sizeof(node));
    head->p_data = arr;
    head->p_next = NULL;
    node * cur = head;
    for (void * i = arr + elem_size; i < arr + array_size; i += elem_size){
        cur->p_next = malloc(sizeof(node));
        cur->p_next->p_data = i;
        cur->p_next->p_next = NULL;
        cur = cur->p_next;
    }

    return head;
}

void * remove_last(node * head){ // returns last nodes data so you could free it
    node * cur = head;
    while (cur->p_next->p_next != NULL)
    {
        cur = (cur->p_next);
    }
    node * next = cur->p_next;
    void * data = next->p_data;
    free(next);
    cur->p_next = NULL;
    return data;
}

void print_list(node * head, const char * format_spec, const unsigned int elem_size){
    node * cur = head;
    while (cur != NULL)
    {
        
        printf(format_spec, **((unsigned char (*)[elem_size])(cur->p_data)));
        cur = (cur->p_next);
    }
    
}
