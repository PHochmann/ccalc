#pragma once
#include <stdlib.h>
#include <stdint.h>

#include "iterator.h"

#define LIST_INSERT_ELEM_AFTER(list, before, type, expr) ((*(type*)listnode_get_data(list_insert_after(list, before, NULL))) = (expr))
#define LIST_INSERT_ELEM_AT(list, index, type, expr) ((*(type*)listnode_get_data(list_insert_at(list, index, NULL))) = (expr))
#define LIST_APPEND_ELEM(list, type, expr) ((*(type*)listnode_get_data(list_append(list, NULL))) = (expr))

typedef struct ListNode ListNode;

typedef struct
{
    size_t elem_size;
    ListNode *first;
    ListNode *last;
    size_t count;
} LinkedList;

typedef struct
{
    Iterator base;
    const LinkedList *list;
    ListNode *curr_node;
} LinkedListIterator;

LinkedList list_create(size_t elem_size);
void list_destroy(LinkedList *list);

void *list_get_at(const LinkedList *list, size_t index);
void *listnode_get_data(const ListNode *node);
ListNode *listnode_get_next(const ListNode *node);
ListNode *list_get_node(const LinkedList *list, size_t index);
LinkedListIterator list_get_iterator(const LinkedList *list);

ListNode *list_append(LinkedList *list, void *data);
ListNode *list_insert_after(LinkedList *list, ListNode *before, void *data);
ListNode *list_insert_at(LinkedList *list, size_t index, void *data);
void list_delete_at(LinkedList *list, size_t index);
void list_delete_node(LinkedList *list, ListNode *node);

size_t list_count(const LinkedList *list);
