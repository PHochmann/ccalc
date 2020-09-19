#include <string.h>
#include <assert.h>

#include "linked_list.h"
#include "alloc_wrappers.h"

ListNode *malloc_node(size_t elem_size, void *data)
{
    ListNode *new = malloc_wrapper(sizeof(ListNode) + elem_size);
    new->next = NULL;
    if (data != NULL) memcpy((void*)new->data, data, elem_size);
    return new;
}

/*
Summary: Creates a new linked list without any items 
*/
LinkedList list_create(size_t elem_size)
{
    return (LinkedList){
        .elem_size = elem_size,
        .first     = NULL,
        .last      = NULL,
        .count     = 0
    };
}

/*
Summary: Frees all ressources allocated for the list
*/
void list_destroy(LinkedList *list)
{
    assert(list != NULL);

    ListNode *curr = list->first;
    while (curr != NULL)
    {
        ListNode *next = curr->next;
        free(curr);
        curr = next;
    }
    list->count = 0;
}

/*
Summary: Retrieves a list node (linear time)
    It can be used to manipulate the linked list in constant time.
*/
ListNode *list_get_node(LinkedList *list, size_t index)
{
    assert(list != NULL);
    assert(index < list->count);

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

/*
Returns: Pointer to payload of a list node
*/
void *list_get_at(LinkedList *list, size_t index)
{
    ListNode *node = list_get_node(list, index);
    if (node == NULL)
    {
        return NULL;
    }
    return (void*)node->data;
}

/*
Summary: Creates a new list node and copies elem_size many bytes pointed to by data into it
Returns: New list node
*/
ListNode *list_append(LinkedList *list, void *data)
{
    return list_insert_after(list, list->last, data);
}

/*
Summary: Creates new list node and inserts it after 'before'
Returns: New list node
*/
ListNode *list_insert_after(LinkedList *list, ListNode *before, void *data)
{
    assert(list != NULL);

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

/*
Summary: Inserts list node at index
Params
    index: Valid indices are 0 to list count (inclusive)
        Thus, appending is possible, but index must not be higher!
*/
ListNode *list_insert_at(LinkedList *list, size_t index, void *data)
{
    if (index == 0)
    {
        return list_insert_after(list, NULL, data);
    }
    else
    {
        return list_insert_after(list, list_get_node(list, index - 1), data);
    }
}

void list_delete_node(LinkedList *list, ListNode *node)
{
    assert(list != NULL);
    assert(node != NULL);

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

void list_delete_at(LinkedList *list, size_t index)
{
    list_delete_node(list, list_get_node(list, index));
}

size_t list_count(LinkedList *list)
{
    assert(list != NULL);
    return list->count;
}
