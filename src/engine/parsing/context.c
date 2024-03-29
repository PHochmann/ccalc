#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "context.h"

/*
Summary: This method is used to create a new ParsingContext without glue-op and operators
    Use ctx_add_op and ctx_add_glue_op to add them to new context
*/
ParsingContext ctx_create()
{
    ParsingContext res = {
        .op_list       = list_create(sizeof(Operator)),
        .keywords_trie = trie_create(0),
        .glue_op       = NULL
    };

    for (size_t i = 0; i < OP_NUM_PLACEMENTS; i++)
    {
        // Payload of trie nodes is pointer to list node in op_list
        res.op_tries[i] = trie_create(sizeof(ListNode*));
    }

    return res;
}

void ctx_destroy(ParsingContext *ctx)
{
    list_destroy(&ctx->op_list);
    trie_destroy(&ctx->keywords_trie);
    for (size_t i = 0; i < OP_NUM_PLACEMENTS; i++)
    {
        trie_destroy(&ctx->op_tries[i]);
    }
}

/*
Summary: Adds given operators to context
Returns: True if all operators were successfully added
    False if inconsistency occurred or invalid arguments
*/
bool ctx_add_ops(ParsingContext *ctx, size_t count, ...)
{
    va_list args;
    va_start(args, count);
    for (size_t i = 0; i < count; i++)
    {
        if (ctx_add_op(ctx, va_arg(args, Operator)) == NULL)
        {
            va_end(args);
            return false;
        }
    }
    va_end(args);
    return true;
}

/*
Summary: Adds operator to context
    To associate every operand with exactly one operator in a unique manner,
    infix operators with the same precedence must have the same associativity.
Returns: Pointer to operator within context, NULL if one of the following:
    * ctx is NULL
    * Infix operator with inconsistent associativity is given
          (another infix operator with same precedence has different associativity)
    * Function of same name and arity is present in context
*/
const Operator *ctx_add_op(ParsingContext *ctx, Operator op)
{
    if (ctx == NULL) return NULL;
    
    // Check for name clash
    if (ctx_lookup_op(ctx, op.name, op.placement) != NULL) return NULL;

    // Consistency checks
    if (op.placement == OP_PLACE_INFIX)
    {
        ListNode *curr = ctx->op_list.first;
        while (curr != NULL)
        {
            // Go through all operators and check if same precedence entails same associativity
            // If not, new operator would make set inconsistent
            Operator *opB = (Operator*)listnode_get_data(curr);
            if (opB->placement == OP_PLACE_INFIX && opB->precedence == op.precedence)
            {
                if (opB->assoc != op.assoc)
                {
                    return NULL;
                }
            }
            curr = listnode_get_next(curr);
        }
    }
    
    // Successfully passed the checks
    op.id = list_count(&ctx->op_list);
    ListNode *list_node = list_append(&ctx->op_list, &op);
    TRIE_ADD_ELEM(&ctx->op_tries[op.placement], op.name, ListNode*, list_node);
    trie_add_str(&ctx->keywords_trie, op.name);
    return (const Operator*)listnode_get_data(list_node);
}

/*
Summary: Deletes operator from context
Returns: True if operator existed before and was deleted, False otherwise
*/
bool ctx_delete_op(ParsingContext *ctx, const char *name, OpPlacement placement)
{
    ListNode **node = NULL;
    if (trie_contains(&ctx->op_tries[placement], name, (void**)&node))
    {
        list_delete_node(&ctx->op_list, *node);
        trie_remove_str(&ctx->keywords_trie, name);
        trie_remove_str(&ctx->op_tries[placement], name);
        return true;
    }
    else
    {
        return false;
    }    
}

/*
Summary: Sets glue-op, which is inserted between two subexpressions (such as 2a -> 2*a)
Returns: False, if ctx is NULL or operator with arity not equal to 2 or DYNAMIC_ARITY given
*/
bool ctx_set_glue_op(ParsingContext *ctx, const Operator *op)
{
    if (ctx == NULL
        || (op != NULL
            && op->arity != 2
            && op->arity != OP_DYNAMIC_ARITY))
    {
        return false;
    }
    ctx->glue_op = op;
    return true;
}

/*
Summmary: Searches for operator of given name and placement
Returns: NULL if no operator has been found or invalid arguments given, otherwise pointer to operator in ctx->operators
*/
const Operator *ctx_lookup_op(const ParsingContext *ctx, const char *name, OpPlacement placement)
{
    if (ctx == NULL || name == NULL) return NULL;

    // Operators are stored in a linked list, a trie is used to lookup the correct listnode
    ListNode **node = NULL;
    if (trie_contains(&ctx->op_tries[placement], name, (void**)&node))
    {
        // Return pointer to payload of listnode
        return (const Operator*)listnode_get_data(*node);
    }
    else
    {
        return NULL;
    }
}
