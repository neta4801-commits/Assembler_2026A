/* This file handles the second pass:
 * resolve symbol-based operands, update .entry labels and collect extern usages.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "second_pass.h"
#include "assembler_tables.h"
#include "src/macros/parser.h"
#include "src/globals/helpers.h"
#include "src/globals/symbol_table.h"
#include "build_output_file.h"

typedef struct {
    const command_info *command;
    char operands[NUMBER_TWO][MAX_LINE_LENGTH + NUMBER_TWO];
    int operand_modes[NUMBER_TWO];
    int operand_count;
    int instruction_length;
} instruction_line_info;

/* Clears per-line parsed instruction data before parsing a new source line. */
static void initialize_instruction_info(instruction_line_info *info) {
    int i;
    int j;

    info->command = NULL;
    info->operand_count = NUMBER_ZERO;
    info->instruction_length = NUMBER_ONE;

    for (i = NUMBER_ZERO; i < NUMBER_TWO; i++) {
        info->operand_modes[i] = MISSING_OPERAND;
        for (j = NUMBER_ZERO; j < MAX_LINE_LENGTH + NUMBER_TWO; j++) {
            info->operands[i][j] = '\0';
        }
    }
}

/* Must stay identical to first pass L calculation so both passes advance IC the same way. */
static int calculate_instruction_length(const command_info *command) {
    return NUMBER_ONE + command->expected_ops;
}

/* Reads first field and skips an optional leading label. */
static boolean extract_word_after_optional_label(char **line_ptr, char *first_word, int line_number) {
    extract_word(line_ptr, first_word);

    if (is_label(first_word)) {
        extract_word(line_ptr, first_word);
        if (first_word[NUMBER_ZERO] == '\0') {
            fprintf(stderr, "Error line %d: Label without command or directive.\n", line_number);
            return FALSE;
        }
    }

    return TRUE;
}

/* Directives handled entirely in first pass and ignored in second pass. */
static boolean is_second_pass_ignored_directive(const char *word) {
    return strcmp(word, ".data") == NUMBER_ZERO ||
           strcmp(word, ".string") == NUMBER_ZERO ||
           strcmp(word, ".extern") == NUMBER_ZERO;
}

/* Adds one extern usage entry, so .ext can print every reference address. */
static boolean add_extern_usage(extern_ptr *extern_head, const char *symbol_name, int usage_address, int line_number) {
    extern_ptr current_node;
    extern_ptr new_node;

    new_node = (extern_ptr)malloc(sizeof(extern_node));
    if (new_node == NULL) {
        fprintf(stderr, "Error line %d: Memory allocation failed while saving extern usage of '%s'.\n",
                line_number, symbol_name);
        return FALSE;
    }

    strcpy(new_node->name, symbol_name);
    new_node->address = usage_address;
    new_node->next = NULL;

    if (*extern_head == NULL) {
        *extern_head = new_node;
        return TRUE;
    }

    current_node = *extern_head;
    while (current_node->next != NULL) {
        current_node = current_node->next;
    }
    current_node->next = new_node;
    return TRUE;
}

static boolean validate_immediate_operand(const char *operand, int line_number, const char *operand_role) {
    int immediate_value;
    char number_text[MAX_LINE_LENGTH + NUMBER_TWO];

    if (operand[NUMBER_ZERO] != '#') {
        fprintf(stderr, "Error line %d: Immediate %s operand must start with '#'.\n", line_number, operand_role);
        return FALSE;
    }
    if (strlen(operand) <= NUMBER_ONE || strlen(operand) >= MAX_LINE_LENGTH + NUMBER_TWO) {
        fprintf(stderr, "Error line %d: Missing or too long immediate value in %s operand '%s'.\n",
                line_number, operand_role, operand);
        return FALSE;
    }

    strcpy(number_text, operand + NUMBER_ONE);
    if (!is_legal_number(number_text)) {
        fprintf(stderr, "Error line %d: Invalid immediate number '%s' in %s operand.\n",
                line_number, operand, operand_role);
        return FALSE;
    }

    immediate_value = atoi(number_text);
    if (!is_number_range(immediate_value)) {
        fprintf(stderr, "Error line %d: Immediate number '%s' in %s operand is out of 12-bit range.\n",
                line_number, operand, operand_role);
        return FALSE;
    }

    return TRUE;
}

/* .entry is validated in second pass because symbol table is complete only after first pass. */
static boolean process_entry_directive(char *line_ptr, AssemblerState *state) {
    symbol_ptr symbol;
    char entry_label[MAX_LINE_LENGTH + NUMBER_TWO];

    extract_word(&line_ptr, entry_label);
    if (entry_label[NUMBER_ZERO] == '\0') {
        fprintf(stderr, "Error line %d: Missing label after .entry directive.\n", state->line_number);
        return FALSE;
    }
    if (strlen(entry_label) > MAX_LABEL_LENGTH) {
        fprintf(stderr, "Error line %d: Label '%s' is too long for .entry directive.\n",
                state->line_number, entry_label);
        return FALSE;
    }
    if (!is_legal_name(entry_label)) {
        fprintf(stderr, "Error line %d: Illegal label name '%s' in .entry directive.\n",
                state->line_number, entry_label);
        return FALSE;
    }

    skip_whitespaces(&line_ptr);
    if (*line_ptr != '\0' && *line_ptr != '\n') {
        fprintf(stderr, "Error line %d: Extraneous text after .entry label '%s'.\n",
                state->line_number, entry_label);
        return FALSE;
    }

    symbol = get_symbol(state->symbol_head, entry_label);
    if (symbol == NULL) {
        fprintf(stderr, "Error line %d: .entry label '%s' was not found in symbol table.\n",
                state->line_number, entry_label);
        return FALSE;
    }
    if (symbol->is_extern) {
        fprintf(stderr, "Error line %d: External symbol '%s' cannot be marked as .entry.\n",
                state->line_number, entry_label);
        return FALSE;
    }

    symbol->is_entry = TRUE;
    return TRUE;
}

/* Extracts raw operand strings from parser output into a unified operands array. */
static boolean set_instruction_operands(char *line_ptr, int line_number, instruction_line_info *info) {
    char src_operand[MAX_LINE_LENGTH + NUMBER_TWO];
    char dst_operand[MAX_LINE_LENGTH + NUMBER_TWO];
    char *cursor = line_ptr;

    src_operand[NUMBER_ZERO] = '\0';
    dst_operand[NUMBER_ZERO] = '\0';

    if (!extract_operands(&cursor, src_operand, dst_operand, info->command->expected_ops, line_number)) {
        return FALSE;
    }

    if (info->command->expected_ops == NUMBER_TWO) {
        strcpy(info->operands[NUMBER_ZERO], src_operand);
        strcpy(info->operands[NUMBER_ONE], dst_operand);
        info->operand_count = NUMBER_TWO;
    } else if (info->command->expected_ops == NUMBER_ONE) {
        strcpy(info->operands[NUMBER_ZERO], dst_operand);
        info->operand_count = NUMBER_ONE;
    }

    return TRUE;
}

/* Checks each operand mode and immediate values with one loop for both one/two-operand commands. */
static boolean validate_instruction_operands(const char *command_name, int line_number, instruction_line_info *info) {
    int operand_index;

    for (operand_index = NUMBER_ZERO; operand_index < info->operand_count; operand_index++) {
        const int *valid_modes;
        const char *operand_role;

        if (info->operand_count == NUMBER_TWO && operand_index == NUMBER_ZERO) {
            valid_modes = info->command->valid_src_operand_types;
            operand_role = "source";
        } else {
            valid_modes = info->command->valid_dest_operand_types;
            operand_role = "destination";
        }

        info->operand_modes[operand_index] = get_addressing_mode(info->operands[operand_index]);
        if (!is_valid_addressing_mode(valid_modes, info->operand_modes[operand_index])) {
            fprintf(stderr, "Error line %d: Invalid %s addressing mode for '%s'.\n",
                    line_number, operand_role, command_name);
            return FALSE;
        }

        if (info->operand_modes[operand_index] == IMMEDIATE_MODE &&
            !validate_immediate_operand(info->operands[operand_index], line_number, operand_role)) {
            return FALSE;
        }
    }

    return TRUE;
}

/* Parses one instruction line into reusable structure and validates all operands. */
static boolean parse_instruction_line(char *first_word, char *line_ptr, int line_number, instruction_line_info *info) {
    info->command = get_command(first_word);
    if (info->command == NULL) {
        fprintf(stderr, "Error line %d: Unknown command '%s'.\n", line_number, first_word);
        return FALSE;
    }

    if (!set_instruction_operands(line_ptr, line_number, info)) {
        return FALSE;
    }
    if (!validate_instruction_operands(first_word, line_number, info)) {
        return FALSE;
    }

    info->instruction_length = calculate_instruction_length(info->command);
    return TRUE;
}

/* Pulls symbol text from direct or relative operands and validates its length constraints. */
static boolean extract_symbol_name(const char *operand_text, int addressing_mode, int line_number, char *symbol_name) {
    const char *symbol_start = operand_text;
    size_t symbol_name_length;

    if (addressing_mode == RELATIVE_MODE) {
        if (operand_text[NUMBER_ZERO] != '%') {
            fprintf(stderr, "Error line %d: Relative operand '%s' must begin with '%%'.\n",
                    line_number, operand_text);
            return FALSE;
        }
        symbol_start = operand_text + NUMBER_ONE;
    }

    symbol_name_length = strlen(symbol_start);
    if (symbol_name_length == NUMBER_ZERO) {
        fprintf(stderr, "Error line %d: Missing symbol name in operand '%s'.\n", line_number, operand_text);
        return FALSE;
    }
    if (symbol_name_length > MAX_LABEL_LENGTH) {
        fprintf(stderr, "Error line %d: Symbol '%s' is too long.\n", line_number, symbol_start);
        return FALSE;
    }

    strncpy(symbol_name, symbol_start, MAX_LABEL_LENGTH);
    symbol_name[symbol_name_length] = '\0';
    return TRUE;
}

static boolean resolve_direct_operand(AssemblerState *state,
                                      symbol_ptr symbol,
                                      const char *symbol_name,
                                      int word_index,
                                      int operand_word_address,
                                      extern_ptr *extern_head,
                                      int line_number) {
    if (symbol->is_extern) {
        state->code_image[word_index].value = NUMBER_ZERO;
        state->code_image[word_index].are = ARE_EXTERNAL;
        return add_extern_usage(extern_head, symbol_name, operand_word_address, line_number);
    }

    state->code_image[word_index].value = symbol->label_address & 0xFFF;
    state->code_image[word_index].are = ARE_RELOCATABLE;
    return TRUE;
}

static boolean resolve_relative_operand(AssemblerState *state, symbol_ptr symbol, int word_index, int operand_word_address, int line_number) {
    int relative_value;

    if (symbol->is_extern) {
        fprintf(stderr, "Error line %d: Relative addressing cannot target external symbol '%s'.\n",
                line_number, symbol->label_name);
        return FALSE;
    }

    relative_value = symbol->label_address - operand_word_address;
    state->code_image[word_index].value = relative_value & 0xFFF;
    state->code_image[word_index].are = ARE_ABSOLUTE;
    return TRUE;
}

/* Resolves one direct/relative operand to final value + ARE in code image. */
static boolean resolve_symbol_operand(AssemblerState *state,
                                      const char *operand_text,
                                      int addressing_mode,
                                      int operand_word_address,
                                      extern_ptr *extern_head,
                                      int line_number) {
    int word_index;
    symbol_ptr symbol;
    char symbol_name[MAX_LABEL_LENGTH + NUMBER_ONE];

    if (addressing_mode != DIRECT_MODE && addressing_mode != RELATIVE_MODE) {
        return TRUE;
    }

    if (!extract_symbol_name(operand_text, addressing_mode, line_number, symbol_name)) {
        return FALSE;
    }
    if (!is_legal_name(symbol_name)) {
        fprintf(stderr, "Error line %d: Illegal symbol operand '%s'.\n", line_number, symbol_name);
        return FALSE;
    }

    symbol = get_symbol(state->symbol_head, symbol_name);
    if (symbol == NULL) {
        fprintf(stderr, "Error line %d: Undefined symbol '%s'.\n", line_number, symbol_name);
        return FALSE;
    }

    word_index = operand_word_address - IC_START;
    if (word_index < NUMBER_ZERO || word_index >= MEMORY_SIZE) {
        fprintf(stderr, "Error line %d: Operand address %d is outside code image bounds.\n",
                line_number, operand_word_address);
        return FALSE;
    }

    if (addressing_mode == DIRECT_MODE) {
        return resolve_direct_operand(state, symbol, symbol_name, word_index, operand_word_address, extern_head, line_number);
    }
    return resolve_relative_operand(state, symbol, word_index, operand_word_address, line_number);
}

/* Resolves all symbol-based operands for a parsed instruction line. */
static boolean resolve_instruction_operands(AssemblerState *state,
                                            const instruction_line_info *info,
                                            int instruction_address,
                                            extern_ptr *extern_head,
                                            int line_number) {
    int operand_index;
    boolean success = TRUE;

    for (operand_index = NUMBER_ZERO; operand_index < info->operand_count; operand_index++) {
        int operand_word_address;

        if (info->operand_count == NUMBER_TWO) {
            operand_word_address = instruction_address + NUMBER_ONE + operand_index;
        } else {
            operand_word_address = instruction_address + NUMBER_ONE;
        }

        if (!resolve_symbol_operand(state,
                                    info->operands[operand_index],
                                    info->operand_modes[operand_index],
                                    operand_word_address,
                                    extern_head,
                                    line_number)) {
            success = FALSE;
        }
    }

    return success;
}

/* Handles one instruction line end-to-end: parse, resolve symbols, and update IC. */
static boolean process_instruction_line(char *first_word, char *line_ptr, AssemblerState *state, extern_ptr *extern_head) {
    boolean success = TRUE;
    int instruction_address = state->ic;
    instruction_line_info instruction_info;

    initialize_instruction_info(&instruction_info);

    if (!parse_instruction_line(first_word, line_ptr, state->line_number, &instruction_info)) {
        if (instruction_info.command != NULL) {
            state->ic += calculate_instruction_length(instruction_info.command);
        }
        return FALSE;
    }

    if (!resolve_instruction_operands(state, &instruction_info, instruction_address, extern_head, state->line_number)) {
        success = FALSE;
    }

    state->ic += instruction_info.instruction_length;
    return success;
}

/* Handles one source line and delegates to the appropriate second-pass action. */
static boolean process_source_line(char *line, AssemblerState *state, extern_ptr *extern_head) {
    char first_word[MAX_LINE_LENGTH + NUMBER_TWO];
    char *line_ptr = line;

    if (is_empty_or_comment(line_ptr)) {
        return TRUE;
    }

    if (!extract_word_after_optional_label(&line_ptr, first_word, state->line_number)) {
        return FALSE;
    }
    if (is_second_pass_ignored_directive(first_word)) {
        return TRUE;
    }
    if (strcmp(first_word, ".entry") == NUMBER_ZERO) {
        return process_entry_directive(line_ptr, state);
    }

    return process_instruction_line(first_word, line_ptr, state, extern_head);
}

boolean second_pass(FILE *am_file, AssemblerState *state, extern_ptr *extern_head) {
    return run_second_pass(am_file, NULL, state, extern_head);
}

/* Executes second pass and, if no errors, builds output files when original_name is provided. */
boolean run_second_pass(FILE *am_file, char *original_name, AssemblerState *state, extern_ptr *extern_head) {
    char line[MAX_LINE_LENGTH + NUMBER_TWO];
    boolean second_pass_errors = FALSE;

    if (am_file == NULL || state == NULL || extern_head == NULL) {
        fprintf(stderr, "Error: second pass received invalid arguments.\n");
        return FALSE;
    }

    if (*extern_head != NULL) {
        free_extern_list(*extern_head);
        *extern_head = NULL;
    }

    rewind(am_file);
    state->ic = IC_START;
    state->line_number = NUMBER_ONE;

    while (fgets(line, sizeof(line), am_file) != NULL) {
        if (!process_source_line(line, state, extern_head)) {
            second_pass_errors = TRUE;
        }
        state->line_number++;
    }

    if (second_pass_errors) {
        state->error_found = TRUE;
        return FALSE;
    }

    if (original_name != NULL && original_name[NUMBER_ZERO] != '\0') {
        build_output_files(original_name, state, *extern_head);
    }

    return TRUE;
}

void free_extern_list(extern_ptr head) {
    extern_ptr temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}
