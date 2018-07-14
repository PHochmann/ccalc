#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "prop.h"

#define PROP_NUM_OPS 9

ParsingContext prop_ctx;

bool prop_eval(Node *node)
{
	switch (node->type)
	{
		case NTYPE_CONSTANT:
			return *(bool*)(node->const_value);
		case NTYPE_OPERATOR:
			switch ((int)(node->op - prop_ctx.operators))
			{
				case 0:
					return prop_eval(node->children[0]) == prop_eval(node->children[1]);
				case 1:
					return !prop_eval(node->children[0]) || prop_eval(node->children[1]);
				case 2:
					return !prop_eval(node->children[1]) || prop_eval(node->children[0]);
				case 3:
					return prop_eval(node->children[0]) || prop_eval(node->children[1]);
				case 4:
					return prop_eval(node->children[0]) && prop_eval(node->children[1]);
				case 5:
					return prop_eval(node->children[0]) != prop_eval(node->children[1]);
				case 6:
					return !prop_eval(node->children[0]);
				case 7:
					for (int i = 0; i < node->num_children; i++) if (prop_eval(node->children[i])) return true;
					return false;
				case 8:
					for (int i = 0; i < node->num_children; i++) if (!prop_eval(node->children[i])) return false;
					return true;
			}
		default:
			return 0;
	}
}

bool _prop_try_parse(char *in, void *out)
{
	if (strcmp(in, "false") == 0 || strcmp(in, "0") == 0)
	{
		*((bool*)out) = false;
		return true;
	}
	else
	{
		if (strcmp(in, "true") == 0 || strcmp(in, "1") == 0)
		{
			*((bool*)out) = true;
			return true;
		}
	}
	return false;
}

void _prop_to_string(void *in, char *out, size_t buff_size)
{
	if (buff_size < 6) return;
	
	if (*(bool*)in == true)
	{
		strcpy(out, "true");
	}
	else
	{
		strcpy(out, "false");
	}
}

ParsingContext prop_get_ctx()
{
	prop_ctx = get_context(sizeof(bool), 6, PROP_NUM_OPS, _prop_try_parse, _prop_to_string);

	add_op(&prop_ctx, op_get_infix("<->", 1, OP_ASSOC_BOTH));
	add_op(&prop_ctx, op_get_infix("->", 1, OP_ASSOC_LEFT));
	add_op(&prop_ctx, op_get_infix("<-", 1, OP_ASSOC_LEFT));
	add_op(&prop_ctx, op_get_infix("|", 2, OP_ASSOC_BOTH));
	add_op(&prop_ctx, op_get_infix("&", 3, OP_ASSOC_BOTH));
	add_op(&prop_ctx, op_get_infix("xor", 3, OP_ASSOC_BOTH));
	
	add_op(&prop_ctx, op_get_prefix("!", 2));
	
	add_op(&prop_ctx, op_get_function("any", DYNAMIC_ARITY));
	add_op(&prop_ctx, op_get_function("all", DYNAMIC_ARITY));
	
	set_glue_op(&prop_ctx, &prop_ctx.operators[4]);
	
	return prop_ctx;
}
