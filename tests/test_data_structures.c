#include <stdlib.h>
#include <string.h>

#include "test_data_structures.h"
#include "../src/util/linked_list.h"

static const size_t NUM_CASES = 2;

bool data_structures_test(Vector *error_builder)
{
    // Case 1: linked list
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
    list_delete(&list, 2);
    LIST_INSERT_ELEM_AFTER(&list, NULL, char*, "One");
    if (strcmp(LIST_GET_ELEM(&list, char*, 0), "One") != 0
        || strcmp(LIST_GET_ELEM(&list, char*, 1), "Two") != 0
        || strcmp(LIST_GET_ELEM(&list, char*, 2), "Three") != 0)
    {
        ERROR("Unexpected item in list.\n");
    }
    list_destroy(&list);

    // Case 2: trie
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
    TRIE_ADD_ELEM(&trie, "test", int, 21);
    if (trie_longest_prefix(&trie, "testitest", (void**)&data) != 4)
    {
        ERROR_RETURN_VAL("trie_longest_prefix");
    }
    if (*data != 21)
    {
        ERROR("trie_longest_prefix lookup was %zu, should be 21.\n", *data);
    }
    trie_destroy(&trie);

    return true;
}

Test get_data_structures_test()
{
    return (Test){
        data_structures_test,
        NUM_CASES,
        "Data structures"
    };
}
