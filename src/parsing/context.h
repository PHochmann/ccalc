#pragma once
#include <stdbool.h>
#include "../tree/operator.h"
#include "../util/list.h"
#include "../util/trie.h"

#define OP_NUM_PLACEMENTS 4

typedef struct
{
    Operator *glue_op;    // Points to op in operators
    LinkedList op_list; // Buffer of operators
    Trie op_tries[OP_NUM_PLACEMENTS];     // Tries for fast operator lookup
    Trie keywords_trie;   // Contains all operators for keyword lookup in tokenizer
} ParsingContext;

ParsingContext context_create();
void context_destroy(ParsingContext *ctx);
bool ctx_add_ops(ParsingContext *ctx, size_t count, ...);
Operator *ctx_add_op(ParsingContext *ctx, Operator op);
bool ctx_remove_op(ParsingContext *ctx, char *name, OpPlacement placement);
bool ctx_set_glue_op(ParsingContext *ctx, Operator *op);
Operator *ctx_lookup_op(ParsingContext *ctx, char *name, OpPlacement placement);
