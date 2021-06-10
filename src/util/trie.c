#include <string.h>
#include <assert.h>

#include "console_util.h"
#include "alloc_wrappers.h"
#include "trie.h"

/*
Todo: Refactor by combining adding and searching into single auxiliary function
*/

/*
Summary: A TrieNode directly contains the data and is always on heap
*/
struct TrieNode
{
    bool is_terminal;                             // True if node represents last char of an inserted string
    unsigned char num_successors;                 // To detect and delete non-terminal leaves
    struct TrieNode *next[TRIE_END_CHAR - TRIE_START_CHAR]; // Pointers to next chars
    uint8_t data[];                               // Payload
};

static TrieNode *malloc_trienode(size_t elem_size)
{
    return calloc_wrapper(1, sizeof(TrieNode) + elem_size);
}

static bool is_legal_char(char c)
{
    return (c >= TRIE_START_CHAR) && (c < TRIE_END_CHAR);
}

static unsigned char char_to_index(char c)
{
    return (unsigned char)(c - TRIE_START_CHAR);
}

/*
Pass elem_size = 0 to use the trie without payload
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
    for (unsigned char i = 0; i < TRIE_END_CHAR - TRIE_START_CHAR; i++)
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
    Returns NULL on duplicate string
*/
void *trie_add_str(Trie *trie, const char *string)
{
    assert(trie != NULL);
    assert(string != NULL);

    TrieNode *curr = trie->first_node;
    for (size_t i = 0; string[i] != '\0'; i++)
    {
        if (!is_legal_char(string[i]))
        {
            software_defect("Trying to insert out of range char '%c' into trie.\n", string[i]);
        }

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
static bool remove_rec(TrieNode *node, size_t depth, const char *string)
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

void trie_remove_str(Trie *trie, const char *string)
{
    assert(trie != NULL);
    assert(string != NULL);
    remove_rec(trie->first_node, 0, string);
}

// out_data may be written to even if string is not in trie
// because method uses trie_longest_prefix
bool trie_contains(const Trie *trie, const char *string, void **out_data)
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

/*
Returns: Length of longest prefix of 'string' that is present in trie
Params
    out_data: Allowed to be NULL
*/
size_t trie_longest_prefix(const Trie *trie, const char *string, void **out_data)
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

// Iterator implementation:

const TrieNode *find_terminal(TrieIterator *ti, size_t depth, int begin_from, bool allow_self)
{
    if (depth == TRIE_MAX_ITERATOR_DEPTH)
    {
        software_defect("Max trie iterator depth reached\n");
    }

    const TrieNode *node = ti->nodes[depth - 1];
    if (allow_self && node->is_terminal) return node;

    for (int i = begin_from; i < TRIE_END_CHAR - TRIE_START_CHAR; i++)
    {
        if (ti->nodes[depth - 1]->next[i] != NULL)
        {
            ti->nodes[depth] = ti->nodes[depth - 1]->next[i];
            ti->curr_str[depth] = i + TRIE_START_CHAR;
            if ((node = find_terminal(ti, depth + 1, 0, true)) != NULL) return node;
        }
    }

    return NULL;
}

static void *get_next(Iterator *iterator)
{
    TrieIterator *ti = (TrieIterator*)iterator;

    // Compute current depth
    size_t depth = 0;
    while (ti->nodes[depth] != NULL) depth++;

    const TrieNode *next_terminal = NULL;

    // Special case: Check if empty string is in trie
    // Then, do depth first search from root
    if (depth == 0)
    {
        depth = 1;
        ti->nodes[0] = ti->trie->first_node;
        next_terminal = find_terminal(ti, 1, 0, true);
        if (next_terminal != NULL) return (void*)next_terminal->data;
        return NULL;
    }

    // First: Carry on depth search
    next_terminal = find_terminal(ti, depth, 0, false);
    if (next_terminal != NULL) return (void*)next_terminal->data;

    // Switch character
    while (depth > 1)
    {
        depth--;
        ti->nodes[depth] = NULL;
        int start = ti->curr_str[depth] - TRIE_START_CHAR + 1;
        ti->curr_str[depth] = '\0';
        next_terminal = find_terminal(ti, depth, start, false);
        if (next_terminal != NULL) return (void*)next_terminal->data;
    }

    return NULL;
}

static void reset(Iterator *iterator)
{
    TrieIterator *ti = (TrieIterator*)iterator;
    *ti = trie_get_iterator(ti->trie);
}

TrieIterator trie_get_iterator(const Trie *trie)
{
    return (TrieIterator){
        .base = { .get_next = get_next, .reset = reset },
        .trie = trie,
        .curr_str = { '\0' },
        .nodes = { NULL },
    };
}

/*
Returns: String of last item that was returned by trie_get_next
*/
const char *trie_get_current_string(const TrieIterator *iterator)
{
    // +1 because first character is always \0
    // because the root node is not associated with a character
    return iterator->curr_str + 1;
}
