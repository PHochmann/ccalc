#include "constraint.h"
#include "text.h"

struct Constraint
{
    size_t from_index;
    size_t to_index;
    size_t min;
};

size_t needed_to_satisfy(struct Constraint *constr, size_t *vars)
{
    size_t sum = 0;
    for (size_t i = constr->from_index; i < constr->to_index; i++)
    {
        sum += vars[i];
    }
    return sum < constr->min ? constr->min - sum : 0;
}

// Make sure to zero out result before
void satisfy_constraints(size_t num_constrs, struct Constraint *constrs, size_t *result)
{
    // Current heuristic: Do simple cells before, then split remaining amount evenly

    for (size_t i = 0; i < num_constrs; i++)
    {
        if (constrs[i].to_index - constrs[i].from_index == 1)
        {
            result[constrs[i].from_index] = constrs[i].min;
        }
    }

    for (size_t i = 0; i < num_constrs; i++)
    {
        size_t needed = needed_to_satisfy(&constrs[i], result);
        if (needed > 0)
        {
            size_t length = constrs[i].to_index - constrs[i].from_index;
            size_t adjustment = needed / length;
            for (size_t j = 0; j < length; j++)
            {
                result[constrs[i].from_index + j] += adjustment;
                if (needed % length > j) result[constrs[i].from_index + j]++;
            }
        }
    }
}

void get_dimensions(Table *table, size_t *out_col_widths, size_t *out_row_heights)
{
    size_t num_cells_upper = table->num_cols * table->num_rows;
    struct Constraint constrs[num_cells_upper];

    // Satisfy constraints of width
    struct Row *curr_row = table->first_row;
    size_t index = 0;
    while (curr_row != NULL)
    {
        for (size_t i = 0; i < MAX_COLS; i++)
        {
            // Build constraints for set parent cells
            if (curr_row->cells[i].is_set && curr_row->cells[i].parent == NULL)
            {
                size_t min = get_text_width(curr_row->cells[i].text);
                // Constraint can be weakened when vlines are in between
                for (size_t j = i + 1; j < i + curr_row->cells[i].span_x; j++)
                {
                    if (min == 0) break;
                    if (table->vlines[j] != BORDER_NONE) min--;
                }

                constrs[index] = (struct Constraint){
                    .min = min,
                    .from_index = i,
                    .to_index = i + curr_row->cells[i].span_x
                };
                index++;
            }
        }
        curr_row = curr_row->next_row;
    }
    for (size_t i = 0; i < table->num_cols; i++) out_col_widths[i] = 0;
    satisfy_constraints(index, constrs, out_col_widths);

    // Satisfy constraints of height
    curr_row = table->first_row;
    index = 0;
    size_t row_index = 0;
    while (curr_row != NULL)
    {
        for (size_t i = 0; i < MAX_COLS; i++)
        {
            if (curr_row->cells[i].is_set && curr_row->cells[i].parent == NULL)
            {
                size_t min = get_num_lines(curr_row->cells[i].text);
                struct Row *row_checked_for_hline = curr_row->next_row;
                // Constraint can be weakened when hlines are in between
                for (size_t j = i + 1; j < i + curr_row->cells[i].span_y; j++)
                {
                    if (row_checked_for_hline == NULL || min == 0) break;
                    if (row_checked_for_hline->hline_above != BORDER_NONE) min--;
                    row_checked_for_hline = row_checked_for_hline->next_row;
                }

                constrs[index] = (struct Constraint){
                    .min = min,
                    .from_index = row_index,
                    .to_index = row_index + curr_row->cells[i].span_y
                };
                index++;
            }
        }
        curr_row = curr_row->next_row;
        row_index++;
    }
    for (size_t i = 0; i < table->num_rows; i++) out_row_heights[i] = 0;
    satisfy_constraints(index, constrs, out_row_heights);
}
