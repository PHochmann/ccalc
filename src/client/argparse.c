#include "../engine/util/trie.h"

Trie long_switches; // Payload: bool
Trie shorthand_switches;

void init_argparse()
{
    long_switches = trie_create(sizeof(bool*));
    shorthand_switches = trie_create(sizeof(bool*));
}

void unload_argparse()
{
    trie_destroy(&long_switches);
    trie_destroy(&shorthand_switches);
}

void add_switch(char *long_str, char *alt_str, bool *detection)
{
    if (long_str != NULL) TRIE_ADD_ELEM(&long_switches, long_str, bool*, detection);
    if (alt_str != NULL) TRIE_ADD_ELEM(&shorthand_switches, alt_str, bool*, detection);
    *detection = false;
}

bool *parse_arg(char *arg)
{
    bool **detection = NULL;
    if (trie_contains(&shorthand_switches, arg, (void**)&detection)
        || trie_contains(&long_switches, arg, (void**)&detection))
    {
        **detection = true;
        return *detection;
    }
    else
    {
        return NULL;
    }
}
