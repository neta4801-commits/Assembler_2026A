/* This file handles the second pass:
 * resolve symbol-based operands, update .entry labels and collect extern usages.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "second_pass.h"
#include "../tables/assembler_tables.h"
#include "../globals/parser.h"
#include "../globals/helpers.h"
#include "../tables/symbol_table.h"
#include "../../outputs/build_output_file.h"

typedef struct {
    const command_info *command;
    char operands[NUMBER_TWO][MAX_LINE_LENGTH + NUMBER_TWO];
    int operand_modes[NUMBER_TWO];
    int operand_count;
    int instruction_length;
} instruction_line_info;

/* this function resets an instruction structure to the default empty state,
   clears command pointer and set operand count to zero,
   fill string buffers with null terminate */
static void reset_instruction(instruction_line_info *info) {
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

/* identical to first pass L calculation so both passes advance IC the same way. */
static int calculate_instruction_length(const command_info *command) {
    return NUMBER_ONE + command->expected_ops;
}

/* Reads first field and skips an optional leading label. */
static void skip_optional_label(char **line_ptr, char *first_word) {
    extract_word(line_ptr, first_word);

    if (is_label(first_word)) {
        extract_word(line_ptr, first_word);
    }
}


/* Directives handled entirely in first pass and ignored in second pass. */
static boolean is_second_pass_ignored_directive(char *word) {
    return strcmp(word, ".data") == NUMBER_ZERO ||
           strcmp(word, ".string") == NUMBER_ZERO ||
           strcmp(word, ".extern") == NUMBER_ZERO;
}

/* Adds one extern usage entry, so .ext can print every reference address. */
static void add_extern_usage(extern_ptr *extern_head, char *symbol_name, int usage_address) {
    extern_ptr current_node;
    extern_ptr new_node;

    new_node = (extern_ptr)check_malloc(sizeof(extern_node));
    strcpy(new_node->name, symbol_name);
    new_node->address = usage_address;
    new_node->next = NULL;

    if (*extern_head == NULL) {
        *extern_head = new_node;
        return;
    }

    current_node = *extern_head;
    while (current_node->next != NULL) {
        current_node = current_node->next;
    }
    current_node->next = new_node;
    return;
}


/* .entry is validated in second pass because symbol table is complete only after first pass. */
static boolean handle_entry_directive(char *line_ptr, AssemblerState *state) {
    symbol_ptr symbol;
    char entry_label[MAX_LINE_LENGTH + NUMBER_TWO];
    
    extract_word(&line_ptr, entry_label);
    if (entry_label[NUMBER_ZERO] == '\0') {
        fprintf(stdout, "Error in line %d: Missing a label after .entry directive.\n", state->line_number);
        return FALSE;
    }

    /* Check if label name length is too long. */
    if (strlen(entry_label) > MAX_LABEL_LENGTH ) {
        fprintf(stdout, "Error in line %d: Label '%s' is too long for .entry directive.\n",
                state->line_number, entry_label);
        return FALSE;
    }

    /* Check if label name is legal (not starting with a digit, not containing invalid characters, etc.). */
    if (!is_legal_name(entry_label)) {
        fprintf(stdout, "Error in line %d: Illegal label name '%s' in .entry directive.\n",
                state->line_number, entry_label);
        return FALSE;
    }

    skip_whitespaces(&line_ptr);

    /* Check for extraneous text after the label name. */
    if (*line_ptr != '\0' && *line_ptr != '\n') {
        fprintf(stdout, "Error in line %d: Extra text after .entry label '%s'.\n",
                state->line_number, entry_label);
        return FALSE;
    }

    symbol = get_symbol(state->symbol_head, entry_label);
    if (symbol == NULL) {
        fprintf(stdout, "Error in line %d: Label '%s' is declared as .entry but is not defined in this file.\n",
                state->line_number, entry_label);
        return FALSE;
    }
    if (symbol->is_extern) {
        fprintf(stdout, "Error in line %d: External symbol '%s' cannot be marked as .entry.\n",
                state->line_number, entry_label);
        return FALSE;
    }

    symbol->is_entry = TRUE;
    return TRUE;
}

/* Extracts raw operand strings from parser output into a unified operands array. */
static boolean get_instruction_operands(char *line_ptr, int line_number, instruction_line_info *info) {
    char src_operand[MAX_LINE_LENGTH + NUMBER_TWO];
    char dst_operand[MAX_LINE_LENGTH + NUMBER_TWO];
    char *cursor = line_ptr;

    src_operand[NUMBER_ZERO] = '\0';
    dst_operand[NUMBER_ZERO] = '\0';

    /* Return false if extraction fails. */
    if (!extract_operands(&cursor, src_operand, dst_operand, info->command->expected_ops, line_number)) {
        return FALSE;
    }

    /* Copy operands in the instruction info structure for validation. */
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


/* Read an instruction line and extracts its operands and addressing modes. */
static boolean get_instruction_info(char *first_word, char *line_ptr, int line_number, instruction_line_info *info) {
    int i;
    /* Find the command by its name, this command is already legal, we checked it in the first pass. */
    info->command = get_command(first_word);

    /* Extract the operands from the line. Stop if there is a syntax error. */
    if (!get_instruction_operands(line_ptr, line_number, info)) {
        return FALSE;
    }

    /* We check the addressing mode for the operand in the command,
     * in order to encode the words we didn't encode in first pass.*/
    for (i = NUMBER_ZERO; i < info->operand_count; i++) {
              info->operand_modes[i] = get_addressing_mode(info->operands[i]);
    }

    /* Calculate how much memory this instruction will take. */
    info->instruction_length = calculate_instruction_length(info->command);
    return TRUE;
}

/*Takes an operand that is a symbol and extracts its name.*/
static void extract_symbol_name(char *operand_text, int addressing_mode, char *symbol_name) {
    char *symbol_start = operand_text;

    /* if relative mode the name starts after the '%' , we need to check if the '%' is there. */
    if (addressing_mode == RELATIVE_MODE) {
        symbol_start = operand_text + NUMBER_ONE;
    }
    strcpy(symbol_name, symbol_start);
}


/* Resolves a direct operand to its final value and ARE. */
static void resolve_direct_operand(AssemblerState *state, symbol_ptr symbol, char *symbol_name,int word_index,
                                            int operand_word_address,extern_ptr *extern_head) {
    
    /*External symbols have no local address, we mark them as external 'E'. */
    if (symbol->is_extern) {
        state->code_image[word_index].value = NUMBER_ZERO;
        state->code_image[word_index].are = ARE_EXTERNAL;

        add_extern_usage(extern_head, symbol_name, operand_word_address);
        return;
    }

    state->code_image[word_index].value = symbol->label_address & 0xFFF;
    state->code_image[word_index].are = ARE_RELOCATABLE;
    return;
}

static boolean resolve_relative_operand
(AssemblerState *state, symbol_ptr symbol, int word_index, int operand_word_address, int line_number) {
    int relative_value;

    /* We cannot calculate a jump distance to an external symbol because its final memory location is unknown. */
    if (symbol->is_extern) {
        fprintf(stdout, "Error in line %d: Cannot calculate final address for '%s'.\n",
                line_number, symbol->label_name);
        return FALSE;
    }

    relative_value = symbol->label_address - operand_word_address;
    state->code_image[word_index].value = relative_value & 0xFFF;
    state->code_image[word_index].are = ARE_ABSOLUTE;
    return TRUE;
}

/* Resolves one direct/relative operand to final value + ARE in code image. */
static boolean resolve_symbol_operand
(AssemblerState *state,  char *operand_text, int addressing_mode,
 int operand_word_address, extern_ptr *extern_head, int line_number) {
    int word_index;
    symbol_ptr symbol;
    char symbol_name[MAX_LABEL_LENGTH + NUMBER_ONE];

    if (addressing_mode != DIRECT_MODE && addressing_mode != RELATIVE_MODE) {
        return TRUE;
    }

    extract_symbol_name(operand_text, addressing_mode, symbol_name);

    symbol = get_symbol(state->symbol_head, symbol_name);
    if (symbol == NULL) {
        fprintf(stdout, "Error in line %d: Undefined symbol '%s'.\n", line_number, symbol_name);
        return FALSE;
    }

    /*  subtract IC_START (100) because code_image array starts from index 0. */
    word_index = operand_word_address - IC_START;
    if (word_index < NUMBER_ZERO || word_index >= MEMORY_SIZE) {
        fprintf(stdout, "Error in line %d: Operand address %d is outside code image bounds.\n",
                line_number, operand_word_address);
        return FALSE;
    }

    if (addressing_mode == DIRECT_MODE) {
        resolve_direct_operand(state, symbol, symbol_name, word_index, operand_word_address, extern_head);
        return TRUE;
    }
    return resolve_relative_operand(state, symbol, word_index, operand_word_address, line_number);
}

/* Resolves all symbol-based operands for a parsed instruction line. */
static boolean resolve_instruction_operands
(AssemblerState *state, instruction_line_info *info,int instruction_address,extern_ptr *extern_head,int line_number) {
    int operand_index;
    boolean success = TRUE;

    for (operand_index = NUMBER_ZERO; operand_index < info->operand_count; operand_index++) {
        int operand_word_address;

        if (info->operand_count == NUMBER_TWO) {
            operand_word_address = instruction_address + NUMBER_ONE + operand_index;
        } else {
            operand_word_address = instruction_address + NUMBER_ONE;
        }

        if (!resolve_symbol_operand
        (state,info->operands[operand_index],info->operand_modes[operand_index],operand_word_address,extern_head,line_number)) {
            success = FALSE;
        }
    }

    return success;
}

/* Handles one instruction line end-to-end: parse, resolve symbols, and update IC. */
static boolean handle_instruction_line(char *first_word, char *line_ptr, AssemblerState *state, extern_ptr *extern_head) {
    boolean success = TRUE;
    int instruction_address = state->ic;
    instruction_line_info instruction_info;

    reset_instruction(&instruction_info);

    if (!get_instruction_info(first_word, line_ptr, state->line_number, &instruction_info)) {
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

/* Handles one line and forwards to the appropriate second-pass action. */
static boolean handle_line(char *line, AssemblerState *state, extern_ptr *extern_head) {
    char first_word[MAX_LINE_LENGTH + NUMBER_TWO];
    char *line_ptr = line;

    /*although we handle empty lines and comments on pre assembler file,
    * when we open the macros, empty lines and comments can pass so we need to ignore them. */
    if (is_empty_or_comment(line_ptr)) {
        return TRUE;
    }

    skip_optional_label(&line_ptr, first_word);

    if (is_second_pass_ignored_directive(first_word)) {
        return TRUE;
    }
    if (strcmp(first_word, ".entry") == NUMBER_ZERO) {
        return handle_entry_directive(line_ptr, state);
    }

    return handle_instruction_line(first_word, line_ptr, state, extern_head);
}

boolean second_pass(FILE *am_file, AssemblerState *state, extern_ptr *extern_head) {
    return run_second_pass(am_file, NULL, state, extern_head);
}

/* Executes second pass and, if no errors, builds output files when original_name is provided. */
boolean run_second_pass(FILE *am_file, char *original_name, AssemblerState *state, extern_ptr *extern_head) {
    char line[MAX_LINE_LENGTH + NUMBER_TWO];
    boolean second_pass_errors = FALSE;

    if (am_file == NULL || state == NULL || extern_head == NULL) {
        fprintf(stdout, "Error: second pass received invalid arguments.\n");
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
        if (!handle_line(line, state, extern_head)) {
            second_pass_errors = TRUE;
        }
        state->line_number++;
    }

    if (second_pass_errors) {
        state->error_found = TRUE;
        return FALSE;
    }

    if (original_name != NULL && original_name[NUMBER_ZERO] != '\0') {
        /* Checks if there is line code or data from the user on file.am-
         * if the file is empty, we need to print an alert to the user.  */
        if (state->ic > IC_START || state->dc > NUMBER_ZERO) {
            build_output_files(original_name, state, *extern_head);
        }
        else {
            printf("There isn't code or data created for file %s, so the output files will not be created.\n", original_name);
        }
    }
    return TRUE;
}


/* Free safely the memory for the list. */
void free_extern_list(extern_ptr head) {
    extern_ptr temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}
