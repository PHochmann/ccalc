#pragma once
#include <stdlib.h>
#include "table.h"

size_t get_text_width(char *str);
size_t console_strlen(char *str);
void print_repeated(char *string, size_t times);
void print_padded(char *string, int length, int total_length, int dot_padding, TextAlignment align);
size_t get_num_lines(char *string);
size_t get_line_of_string(char *string, size_t line_index, char **out_start);
