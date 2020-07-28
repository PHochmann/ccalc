#include <stdlib.h>

#define LIST_APPEND_ELEM(list, type, expr) (*(type*)list_append(list, NULL)->data) = expr
#define LIST_GET_ELEM(list, type, index) (*(type*)list_get(list, index))

typedef struct ListNode {
    struct ListNode *next;
    struct ListNode *previous;
    char data[];
} ListNode;

typedef struct {
    size_t elem_size;
    ListNode *first;
    ListNode *last;
    size_t count;
} LinkedList;

LinkedList list_create(size_t elem_size);
void list_destroy(LinkedList *list);

void *list_get(LinkedList *list, size_t index);
ListNode *list_get_node(LinkedList *list, size_t index);

ListNode *list_append(LinkedList *list, void *data);
ListNode *list_insert(LinkedList *list, size_t index, void *data);

void list_delete(LinkedList *list, size_t index);
void list_delete_node(LinkedList *list, ListNode *node);

size_t list_count(LinkedList *list);
