#include "list.h"
#include <string.h>

ListNode *malloc_node(size_t elem_size, void *data)
{
    ListNode *new = malloc(sizeof(ListNode) + elem_size);
    new->next = NULL;
    if (data != NULL) memcpy(data, (void*)new->data, elem_size);
    return new;
}

ListNode *list_get_node(LinkedList *list, size_t index)
{
    ListNode *curr = list->first;
    while (curr != NULL && index > 0)
    {
        curr = curr->next;
        index--;
    }
    return curr;
}

LinkedList list_create(size_t elem_size)
{
    return (LinkedList){
        .elem_size = elem_size,
        .first = NULL,
        .last = NULL
    };
}

void list_destroy(LinkedList *list)
{
    ListNode *curr = list->first;
    while (curr != NULL)
    {
        ListNode *next = curr->next;
        free(curr);
        curr = next;
    }
    list->count = 0;
}

void list_append(LinkedList *list, void *data)
{
    ListNode *new = malloc_node(list->elem_size, data);
    if (list->last != NULL)
    {
        list->last->next = new;
    }
    else
    {
        list->first = new;
    }
    list->last = new;
    list->count++;
}

void *list_append_empty(LinkedList *list)
{
    ListNode *new = malloc_node(list->elem_size, NULL);
    if (list->last != NULL)
    {
        list->last->next = new;
    }
    else
    {
        list->first = new;
    }
    list->last = new;
    list->count++;
    return new->data;
}

void list_insert(LinkedList *list, size_t index, void *data)
{
    ListNode *before = NULL;
    ListNode *after = NULL;
    ListNode *new = malloc_node(list->elem_size, data);

    if (index != 0)
    {
        before = list_get_node(list, index - 1);
        if (before != NULL) after = before->next;
    }
    else
    {
        before = NULL;
        after = list->first;
        list->first = new;
    }

    new->next = after;

    if (after == NULL)
    {
        list->last = new;
    }

    list->count++;
}

void *list_get(LinkedList *list, size_t index)
{
    ListNode *node = list_get_node(list, index);
    if (node == NULL) return NULL;
    return (void*)node->data;
}

void list_delete(LinkedList *list, size_t index)
{
    ListNode *before = NULL;
    ListNode *to_delete = NULL;
    ListNode *after = NULL;

    if (index != 0)
    {
        before = list_get_node(list, index - 1);
        if (before != NULL)
        {
            to_delete = before->next;
            if (to_delete != NULL)
            {
                after = to_delete->next;
            }
        }
    }
    else
    {
        before = NULL;
        to_delete = list->first;
        if (to_delete != NULL)
        {
            after = to_delete->next;
        }
    }

    if (to_delete != NULL)
    {
        free(to_delete);
    }

    if (before == NULL)
    {
        list->first = after;
    }
    else
    {
        before->next = after;
    }
    
    if (after == NULL)
    {
        list->last = before;
    }

    list->count--;
}

size_t list_count(LinkedList *list)
{
    return list->count;
}
