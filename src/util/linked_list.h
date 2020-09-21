#pragma once
#include <stdlib.h>
#include <stdint.h>

#include "iterator.h"

#define LIST_INSERT_ELEM_AFTER(list, before, type, expr) ((*(type*)list_insert_after(list, before, NULL)->data) = (expr))
#define LIST_INSERT_ELEM_AT(list, index, type, expr) ((*(type*)list_insert_at(list, index, NULL)->data) = (expr))
#define LIST_APPEND_ELEM(list, type, expr) ((*(type*)list_append(list, NULL)->data) = (expr))

/*
A ListNode directly contains the payload and is always on heap
*/
typedef struct ListNode
{
    struct ListNode *next;
    struct ListNode *previous;
    uint8_t data[];
} ListNode;

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
    LinkedList *list;
    ListNode *curr_node;
} LinkedListIterator;

LinkedList list_create(size_t elem_size);
void list_destroy(LinkedList *list);

void *list_get_at(LinkedList *list, size_t index);
ListNode *list_get_node(LinkedList *list, size_t index);
LinkedListIterator list_get_iterator(LinkedList *list);

ListNode *list_append(LinkedList *list, void *data);
ListNode *list_insert_after(LinkedList *list, ListNode *before, void *data);
ListNode *list_insert_at(LinkedList *list, size_t index, void *data);
void list_delete_at(LinkedList *list, size_t index);
void list_delete_node(LinkedList *list, ListNode *node);

size_t list_count(LinkedList *list);
