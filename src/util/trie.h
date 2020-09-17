#pragma once
#include <stdbool.h>
#include <stdlib.h>

#define START_CHAR '!'
#define END_CHAR   'z'

#define TRIE_ADD_ELEM(trie, str, type, expr) *(type*)trie_add_str(trie, str) = (expr)

/*
Summary: A TrieNode directly contains the data and is always on heap
*/
typedef struct TrieNode
{
    bool is_leaf;
    struct TrieNode *next[END_CHAR - START_CHAR + 1];
    char data[];
} TrieNode;

typedef struct
{
    TrieNode *first_node;
    size_t elem_size;
} Trie;

Trie trie_create(size_t elem_size);
void trie_destroy(Trie *trie);
void *trie_add_str(Trie *trie, char *string);
void trie_remove_str(Trie *trie, char *string);
bool trie_contains(Trie *trie, char *string, void **out_data);
size_t trie_longest_prefix(Trie *trie, char *string, void **out_data);
