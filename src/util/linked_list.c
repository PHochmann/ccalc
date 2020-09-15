#include "linked_list.h"
#include "../util/alloc_wrappers.h"
#include <string.h>

ListNode *malloc_node(size_t elem_size, void *data)
{
    ListNode *new = malloc_wrapper(sizeof(ListNode) + elem_size);
    new->next = NULL;
    if (data != NULL) memcpy((void*)new->data, data, elem_size);
    return new;
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

ListNode *list_get_node(LinkedList *list, size_t index)
{
    if (index >= list->count) return NULL;

    ListNode *curr;
    if (index < list->count / 2)
    {
        // Search forward
        curr = list->first;
        while (curr != NULL && index > 0)
        {
            curr = curr->next;
            index--;
        }
    }
    else
    {
        // Search backward
        index = list->count - index - 1;
        curr = list->last;
        while (curr != NULL && index > 0)
        {
            curr = curr->previous;
            index--;
        }
    }
    
    return curr;
}

void *list_get(LinkedList *list, size_t index)
{
    ListNode *node = list_get_node(list, index);
    if (node == NULL)
    {
        return NULL;
    }
    return (void*)node->data;
}

ListNode *list_append(LinkedList *list, void *data)
{
    return list_insert_after(list, list->last, data);
}

ListNode *list_insert_after(LinkedList *list, ListNode *before, void *data)
{
    ListNode *after;
    if (before == NULL)
    {
        after = list->first;
    }
    else
    {
        after = before->next;
    }
    
    ListNode *new = malloc_node(list->elem_size, data);
    new->previous = before;
    new->next = after;

    if (before == NULL)
    {
        list->first = new;
    }
    else
    {
        before->next = new;
    }
    
    if (after == NULL)
    {
        list->last = new;
    }
    else
    {
        after->previous = new;
    }
    
    list->count++;
    return new;
}

ListNode *list_insert(LinkedList *list, size_t index, void *data)
{
    return list_insert_after(list, list_get_node(list, index), data);
}

void list_delete_node(LinkedList *list, ListNode *node)
{
    ListNode *after = node->next;
    ListNode *before = node->previous;
    free(node);

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
    else
    {
        after->previous = before;
    }
    
    list->count--;
}

void list_delete(LinkedList *list, size_t index)
{
    list_delete_node(list, list_get_node(list, index));
}

size_t list_count(LinkedList *list)
{
    return list->count;
}
