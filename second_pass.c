/* 
 * second_pass.c
 * Core algorithm for the second pass: resolves symbol operands and builds final structures.
 */

#include <stdio.h>
#include <string.h>
#include "second_pass.h"
#include "assembler_tables.h"
#include "src/macros/parser.h"
#include "src/globals/constant.h"
#include "src/globals/helpers.h"
#include "src/globals/symbol_table.h"
#include "build_output_file.h"

/* Resolves symbols to their final values and ARE bits in code_image (Steps 5 & 6). */
static boolean resolve_symbol_operand(AssemblerState *state, const char *operand_text, int addressing_mode, int operand_word_address, extern_ptr *extern_head, int line_number) {
    int word_index, relative_value;
    size_t symbol_name_length;
    const char *symbol_start;
    symbol_ptr symbol;
    char symbol_name[MAX_LABEL_LENGTH + NUMBER_ONE];

    if (addressing_mode != DIRECT_MODE && addressing_mode != RELATIVE_MODE) return TRUE;

    symbol_start = (addressing_mode == RELATIVE_MODE) ? operand_text + NUMBER_ONE : operand_text;
    symbol_name_length = strlen(symbol_start);
    
    if (symbol_name_length == NUMBER_ZERO || symbol_name_length > MAX_LABEL_LENGTH) return FALSE;
    strncpy(symbol_name, symbol_start, MAX_LABEL_LENGTH);
    symbol_name[symbol_name_length] = '\0';

    symbol = get_symbol(state->symbol_head, symbol_name);
    if (symbol == NULL) {
        fprintf(stderr, "Error line %d: Undefined symbol '%s'.\n", line_number, symbol_name);
        return FALSE;
    }

    word_index = operand_word_address - IC_START;
    
    if (addressing_mode == DIRECT_MODE) {
        if (symbol->is_extern) {
            state->code_image[word_index].value = NUMBER_ZERO;
            state->code_image[word_index].are = ARE_EXTERNAL;
            return add_extern_usage(extern_head, symbol_name, operand_word_address, line_number);
        }
        state->code_image[word_index].value = symbol->label_address & 0xFFF;
        state->code_image[word_index].are = ARE_RELOCATABLE;
        return TRUE;
    }

    if (symbol->is_extern) {
        fprintf(stderr, "Error line %d: Relative addressing cannot target external symbol.\n", line_number);
        return FALSE;
    }

    relative_value = symbol->label_address - operand_word_address;
    state->code_image[word_index].value = relative_value & 0xFFF;
    state->code_image[word_index].are = ARE_ABSOLUTE;
    return TRUE;
}

/* Maps parsed operands to the exact extra words created in first pass (Step 6). */
static boolean encode_instruction_operands(AssemblerState *state, const instruction_line_info *info, int instruction_address, extern_ptr *extern_head, int line_number) {
    boolean success = TRUE;
    if (info->command->expected_ops == NUMBER_TWO) {
        if (!resolve_symbol_operand(state, info->src_operand, info->src_addressing_mode, instruction_address + NUMBER_ONE, extern_head, line_number)) success = FALSE;
        if (!resolve_symbol_operand(state, info->dst_operand, info->dst_addressing_mode, instruction_address + NUMBER_TWO, extern_head, line_number)) success = FALSE;
    } else if (info->command->expected_ops == NUMBER_ONE) {
        if (!resolve_symbol_operand(state, info->dst_operand, info->dst_addressing_mode, instruction_address + NUMBER_ONE, extern_head, line_number)) success = FALSE;
    }
    return success;
}

/* High-level processing for a single line of assembly code. */
static boolean process_single_line(char *line, AssemblerState *state, extern_ptr *extern_head) {
    char first_word[MAX_LINE_LENGTH + NUMBER_TWO];
    char *line_ptr = line;
    int instruction_address;
    boolean no_errors = TRUE;
    const command_info *command_for_length;
    instruction_line_info instruction_info;

    initialize_instruction_info(&instruction_info);

    if (is_empty_or_comment(line_ptr)) return TRUE; /* Step 1 */

    if (!extract_word_after_optional_label(&line_ptr, first_word, state->line_number)) return FALSE; /* Step 2 */

    if (is_second_pass_ignored_directive(first_word)) return TRUE; /* Step 3 */

    /* Step 4 & 5 */
    if (strcmp(first_word, ".entry") == NUMBER_ZERO) {
        return process_entry_directive(line_ptr, state);
    }

    /* Step 6 */
    instruction_address = state->ic;
    command_for_length = get_command(first_word);

    if (!parse_instruction_line(first_word, line_ptr, state->line_number, &instruction_info)) {
        if (command_for_length != NULL) state->ic += (NUMBER_ONE + command_for_length->expected_ops);
        return FALSE;
    }

    if (!encode_instruction_operands(state, &instruction_info, instruction_address, extern_head, state->line_number)) {
        no_errors = FALSE;
    }

    state->ic += instruction_info.instruction_length;
    return no_errors;
}

boolean second_pass(FILE *am_file, AssemblerState *state, extern_ptr *extern_head) {
    return run_second_pass(am_file, NULL, state, extern_head);
}

/* Step 1 through 8 Orchestrator */
boolean run_second_pass(FILE *am_file, char *original_name, AssemblerState *state, extern_ptr *extern_head) {
    char line[MAX_LINE_LENGTH + NUMBER_TWO];
    boolean second_pass_errors = FALSE;

    if (am_file == NULL || state == NULL || extern_head == NULL) return FALSE;

    if (*extern_head != NULL) {
        free_extern_list(*extern_head);
        *extern_head = NULL;
    }

    rewind(am_file);
    state->ic = IC_START;
    state->line_number = NUMBER_ONE;

    /* Step 1 Loop */
    while (fgets(line, sizeof(line), am_file) != NULL) {
        if (!process_single_line(line, state, extern_head)) {
            second_pass_errors = TRUE;
        }
        state->line_number++;
    }

    /* Step 7 */
    if (second_pass_errors) {
        state->error_found = TRUE;
        return FALSE;
    }

    /* Step 8 */
    if (original_name != NULL && original_name[NUMBER_ZERO] != '\0') {
        build_output_files(original_name, state, *extern_head);
    }

    return TRUE;
}