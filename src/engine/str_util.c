#include <string.h>

#include "str_util.h"

bool begins_with(char *prefix, char *string)
{
	size_t prefix_length = strlen(prefix);
	size_t string_length = strlen(string);
	if (prefix_length > string_length) return false;
	return strncmp(prefix, string, prefix_length) == 0;
}