#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

// To save a little bit of space, rule out portions of the ASCII table that will
// never occur in strings inserted into the trie.
#define START_CHAR '!'
#define END_CHAR   ('z' + 1) // Exclusive bound

#define TRIE_ADD_ELEM(trie, str, type, expr) (*(type*)trie_add_str(trie, str) = (expr))

/*
Summary: A TrieNode directly contains the data and is always on heap
*/
typedef struct TrieNode
{
    bool is_terminal;                             // True if node represents last char of an inserted string
    unsigned char num_successors;                 // To detect and delete non-terminal leaves
    struct TrieNode *next[END_CHAR - START_CHAR]; // Pointers to next chars
    uint8_t data[];                               // Payload
} TrieNode;

typedef struct
{
    size_t elem_size;
    TrieNode *first_node;
} Trie;

Trie trie_create(size_t elem_size);
void trie_destroy(Trie *trie);
void *trie_add_str(Trie *trie, const char *string);
void trie_remove_str(Trie *trie, const char *string);
bool trie_contains(const Trie *trie, const char *string, void **out_data);
size_t trie_longest_prefix(const Trie *trie, const char *string, void **out_data);
