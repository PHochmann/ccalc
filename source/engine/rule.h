#include "context.h"
#include "node.h"

typedef struct {
	
	ParsingContext context;
	Node before;
	Node after;
	
} RewriteRule;

typedef struct {
	
	Node *matched_tree;

	char **mapped_vars;
	Node **mapped_nodes;
	size_t num_mapped;
	
} Matching;

bool get_matching(ParsingContext *ctx, Node *tree, Node *pattern, Matching *result);
bool find_matching(ParsingContext *ctx, Node *tree, Node *pattern, Matching *result);