#include <string.h>
#include <assert.h>

#include "alloc_wrappers.h"
#include "trie.h"

static TrieNode *malloc_trienode(size_t elem_size)
{
    return calloc_wrapper(1, sizeof(TrieNode) + elem_size);
}

static bool is_legal_char(char c)
{
    return (c >= START_CHAR) && (c < END_CHAR);
}

static unsigned char char_to_index(char c)
{
    return (unsigned char)(c - START_CHAR);
}

/*
Pass 0 as elem_size to use the trie without payload
*/
Trie trie_create(size_t elem_size)
{
    return (Trie){
        .first_node = malloc_trienode(elem_size),
        .elem_size  = elem_size
    };
}

static void destroy_rec(TrieNode *node)
{
    for (unsigned char i = 0; i < END_CHAR - START_CHAR; i++)
    {
        if (node->next[i] != NULL) destroy_rec(node->next[i]);
    }
    free(node);
}

void trie_destroy(Trie *trie)
{
    assert(trie != NULL);
    destroy_rec(trie->first_node);
}

/*
Returns: Pointer to a buffer to attach an item to the inserted string
    The size of this buffer is equal to the elem_size specified when creating the trie
*/
void *trie_add_str(Trie *trie, char *string)
{
    assert(trie != NULL);
    assert(string != NULL);

    TrieNode *curr = trie->first_node;
    for (size_t i = 0; string[i] != '\0'; i++)
    {
        if (!is_legal_char(string[i])) return NULL;
        unsigned char index = char_to_index(string[i]);
        if (curr->next[index] == NULL)
        {
            curr->next[index] = malloc_trienode(trie->elem_size);
            curr->num_successors++;
        }
        curr = curr->next[index];
    }

    // curr is end node of inserted string
    if (curr->is_terminal)
    {
        // String is already present
        return NULL;
    }
    curr->is_terminal = true;
    return (void*)curr->data;
}

// Returns true if node has been freed
static bool remove_rec(TrieNode *node, size_t depth, char *string)
{
    if (depth == strlen(string))
    {
        node->is_terminal = false;
    }
    else
    {
        if (!is_legal_char(string[depth])) return false;
        unsigned char index = char_to_index(string[depth]);
        if (node->next[index] != NULL)
        {
            if (remove_rec(node->next[index], depth + 1, string))
            {
                node->next[index] = NULL;
                node->num_successors--;
            }
        }
    }

    // Never free the first trie node
    if (node->num_successors == 0 && depth != 0)
    {
        free(node);
        return true;
    }
    else
    {
        return false;
    }
}

void trie_remove_str(Trie *trie, char *string)
{
    assert(trie != NULL);
    assert(string != NULL);
    remove_rec(trie->first_node, 0, string);
}

bool trie_contains(Trie *trie, char *string, void **out_data)
{
    if (string[0] == '\0')
    {
        if (out_data != NULL) *out_data = trie->first_node->data;
        return trie->first_node->is_terminal;
    }
    else
    {
        return trie_longest_prefix(trie, string, out_data) == strlen(string);
    }
}

size_t trie_longest_prefix(Trie *trie, char *string, void **out_data)
{
    assert(trie != NULL);
    assert(string != NULL);
    
    size_t res = 0;
    TrieNode *curr = trie->first_node;
    if (out_data != NULL) *out_data = curr->data;

    for (size_t i = 0; string[i] != '\0'; i++)
    {
        // Char in string that can not be in trie
        if (!is_legal_char(string[i])) return res;
        unsigned char index = char_to_index(string[i]);
        if (curr->next[index] != NULL)
        {
            curr = curr->next[index];
        }
        else
        {
            return res;
        }

        if (curr->is_terminal)
        {
            res = i + 1;
            if (out_data != NULL) *out_data = curr->data;
        }
    }

    return res;
}
