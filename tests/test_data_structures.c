#include <stdlib.h>
#include <string.h>

#include "test_data_structures.h"
#include "../src/util/linked_list.h"
#include "../src/util/trie.h"

bool data_structures_test(StringBuilder *error_builder)
{
    // Case 1: vector
    Vector vec = vec_create(sizeof(int), 1);
    if (vec_count(&vec) != 0)
    {
        ERROR_RETURN_VAL("vec_count");
    }
    for (int i = 0; i < 50; i++)
    {
        VEC_PUSH_ELEM(&vec, int, i);
    }
    if (vec_count(&vec) != 50)
    {
        ERROR_RETURN_VAL("vec_count");
    }
    for (int i = 0; i < 50; i++)
    {
        if (*(int*)vec_get(&vec, i) != i)
        {
            ERROR("Value mismatch at index %zu\n", i);
        }
    }
    if (*(int*)vec_pop(&vec) != 49)
    {
        ERROR_RETURN_VAL("vec_pop");
    }
    if (vec_count(&vec) != 49)
    {
        ERROR_RETURN_VAL("vec_count");
    }
    vec_destroy(&vec);

    // Case 2: linked list
    LinkedList list = list_create(sizeof(char*));
    if (list_count(&list) != 0)
    {
        ERROR_RETURN_VAL("list_count");
    }
    LIST_APPEND_ELEM(&list, char*, "Two");
    LIST_APPEND_ELEM(&list, char*, "Three");
    LIST_APPEND_ELEM(&list, char*, "Two");
    if (list_count(&list) != 3)
    {
        ERROR_RETURN_VAL("list_count");
    }
    list_delete_at(&list, 2); // Delete last element
    LIST_INSERT_ELEM_AFTER(&list, NULL, char*, "One");
    if (strcmp(*(char**)list_get_at(&list, 0), "One") != 0
        || strcmp(*(char**)list_get_at(&list, 1), "Two") != 0
        || strcmp(*(char**)list_get_at(&list, 2), "Three") != 0)
    {
        ERROR("Unexpected item in list\n");
    }
    list_delete_at(&list, 0); // Delete first element
    if (strcmp(*(char**)list_get_at(&list, 0), "Two") != 0)
    {
        ERROR("First element of list not correct\n");
    }
    LIST_INSERT_ELEM_AT(&list, 1, char*, "2.5");
    if (strcmp(*(char**)list_get_at(&list, 1), "2.5") != 0)
    {
        ERROR("In-between list insertion incorrect\n");
    }
    list_destroy(&list);

    // Case 3: trie
    Trie trie = trie_create(sizeof(int));
    int *data;
    if (trie_contains(&trie, "", (void**)&data))
    {
        ERROR("Empty string should not be in trie\n");
    }
    TRIE_ADD_ELEM(&trie, "", int, 42);
    if (!trie_contains(&trie, "", (void**)&data))
    {
        ERROR("Empty string should be in trie\n");
    }
    if (*data != 42)
    {
        ERROR("Wrong value after lookup\n");
    }
    TRIE_ADD_ELEM(&trie, "aaaa", int, 21);
    if (trie_longest_prefix(&trie, "aaaaaaa", (void**)&data) != 4)
    {
        ERROR_RETURN_VAL("trie_longest_prefix");
    }
    if (*data != 21)
    {
        ERROR("trie_longest_prefix lookup was %zu, should be 21.\n", *data);
    }
    if (trie_contains(&trie, "z", NULL))
    {
        ERROR_RETURN_VAL("trie_contains");
    }
    if (!trie_contains(&trie, "aaaa", NULL))
    {
        ERROR_RETURN_VAL("trie_contains");
    }
    if (trie_add_str(&trie, "aaaa") != NULL)
    {
        ERROR("trie_add_str did not return NULL on duplicate insertion\n");
    }
    trie_remove_str(&trie, "");
    if (trie_contains(&trie, "", NULL))
    {
        ERROR("Empty string should not be in trie, as it has been deleted\n");
    }
    *(int*)trie_add_str(&trie, "aaaaaaaaaaaa") = 1;
    *(int*)trie_add_str(&trie, "aabzzz") = 3;
    *(int*)trie_add_str(&trie, "aaaaab") = 2;

    TrieIterator ti = trie_get_iterator(&trie);
    int *a = iterator_get_next((Iterator*)&ti);
    int *b= iterator_get_next((Iterator*)&ti);
    int *c = iterator_get_next((Iterator*)&ti);
    int *d = iterator_get_next((Iterator*)&ti);
    int *e = iterator_get_next((Iterator*)&ti);
    int *f = iterator_get_next((Iterator*)&ti);
    if (*a != 42 || *b != 21 || *c != 1 || *d != 2 || *e != 3 || f != NULL)
    {
        ERROR("Trie iterator error\n");
    }

    trie_remove_str(&trie, "aaaaaaaaaaaa");
    trie_destroy(&trie);

    return true;
}

Test get_data_structures_test()
{
    return (Test){
        data_structures_test,
        "Data structures"
    };
}
