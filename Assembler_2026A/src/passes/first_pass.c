/* This file is for the first pass: adds symbols for symbol table for second pass,
 * checks and encodes data and string instructions on the file,
 * encodes commands words and check addressing mode for operand's command.
 * we check the user's am.file and print him an error alerts if there are mistakes in his file.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "first_pass.h"
#include "../globals/parser.h"
#include "../globals/helpers.h"
#include "../tables/assembler_tables.h"
#include "../tables/symbol_table.h"

/*
 * This function encodes the operands into the code image.
 * we calculate the address we need to put the word.
 * for the source operand, we add 1 for the index in code_image.
 * for the dest operand, we need to calculate the offset between it and the source operand to know the index in code_image.
 * the operands with IMMEDIATE_MODE or REGISTER_DIRECT_MODE gets an 'A' type for command.
 * for REGISTER_DIRECT_MODE we need to turn on a bit in the register number positon- 1 << num_for_reg.
 */
static void build_operands_words
        (AssemblerState *state, char *src, char *dst, int src_addressing_mode, int dst_addressing_mode, int expected_ops) {
    int offset;
    int num_for_reg;

    if (expected_ops == NUMBER_TWO) {
        if (src_addressing_mode == IMMEDIATE_MODE) {
            state->code_image[state->ic - IC_START + NUMBER_ONE].value = atoi(src + NUMBER_ONE) & 0xFFF;
        }
        else if (src_addressing_mode == REGISTER_DIRECT_MODE) {
            num_for_reg = atoi(src + NUMBER_ONE);
            state->code_image[state->ic - IC_START + NUMBER_ONE].value = (NUMBER_ONE << num_for_reg);
        }
        else {
            state->code_image[state->ic - IC_START + NUMBER_ONE].value = NUMBER_ZERO;
        }
        state->code_image[state->ic - IC_START + NUMBER_ONE].are =
                /* if src_addressing_mode has immediate mode or register direct mode this command is 'A' type. otherwise,
                 * we don't know yet the type of the command, and we need to check in second pass. */
                (src_addressing_mode == IMMEDIATE_MODE || src_addressing_mode == REGISTER_DIRECT_MODE) ? ARE_ABSOLUTE : ARE_NOT_DEFINE_YET;
    }

    if (expected_ops >= NUMBER_ONE) {
        offset = (expected_ops == NUMBER_TWO) ? NUMBER_TWO : NUMBER_ONE;

        if (dst_addressing_mode == IMMEDIATE_MODE) {
            state->code_image[state->ic - IC_START + offset].value = atoi(dst + NUMBER_ONE) & 0xFFF;
        }
        else if (dst_addressing_mode == REGISTER_DIRECT_MODE) {
            num_for_reg = atoi(dst + NUMBER_ONE);
            state->code_image[state->ic - IC_START + offset].value = (NUMBER_ONE << num_for_reg);
        }
        else {
            state->code_image[state->ic - IC_START + offset].value = NUMBER_ZERO;
        }
        state->code_image[state->ic - IC_START + offset].are =
                (dst_addressing_mode == IMMEDIATE_MODE || dst_addressing_mode == REGISTER_DIRECT_MODE) ? ARE_ABSOLUTE : ARE_NOT_DEFINE_YET;
    }
}


/*
 * This function gets the am_file and the assembler's state.
 * it executes the first pass algorithm according to the project instructions:
 * builds us the symbol table, updates the counters ic (for instruction lines) and dc (for datas).
 * adds the code words and the data words to tables.
 * If the first pass is successful, returns true - no errors were found. Otherwise, false and error alerts to the user.
 */
boolean first_pass(FILE *am_file, AssemblerState *state) {
    char line[MAX_LINE_LENGTH + NUMBER_TWO];
    char first_word[MAX_LINE_LENGTH];
    char label[MAX_LABEL_LENGTH];
    char clean_label[MAX_LABEL_LENGTH];
    char src[MAX_LINE_LENGTH];
    char dst[MAX_LINE_LENGTH];
    char *line_ptr;
    symbol_ptr current_symbol;
    symbol_ptr label_existing;
    boolean label_found;
    const command_info *cmd;
    int src_addressing_mode,dst_addressing_mode ,L, index ;

    if (am_file == NULL || state == NULL) {
        printf("Error: first pass received invalid arguments.\n");
        return FALSE;
    }

    state->ic = IC_START;
    state->dc = NUMBER_ZERO;
    state->line_number = NUMBER_ONE;
    state->error_found = FALSE;


    /* we check line by line for errors in data, command and labels in the file.*/
    while (fgets(line, sizeof(line), am_file)) {
        label_found = FALSE;
        src_addressing_mode = NUMBER_MINUS_ONE;
        dst_addressing_mode = NUMBER_MINUS_ONE;
        /*L= the numbers of words for command lines. */
        L = NUMBER_ONE;

        cmd= NULL;
        line_ptr = line;

        /* comment or empty line - we need to continue to the next line.
         * although we handle empty lines and comments on pre assembler file,
         * when we open the macros, empty lines and comments can pass so we need to ignore them.*/
        if (is_empty_or_comment(line_ptr)) {
            state->line_number++;
            continue;
        }

        extract_word(&line_ptr, first_word);

        /* checks if the word is a label.
         * we need to check the label length, if it's too long, we print an error alert to the user.
         * if the word end with ':' and it isn't a legal label, we need to print an error alert to the user. */
        if (is_label(first_word)) {

            label_found= TRUE;
            /* save the label name without ':' */
            strncpy(label, first_word, strlen(first_word) - NUMBER_ONE);
            label[strlen(first_word) - NUMBER_ONE] = '\0';
            /* We check if the word after label is a command or directive. if not, we need to print an error to the user */
            extract_word(&line_ptr, first_word);
            if (first_word[NUMBER_ZERO] == '\0') {
                fprintf(stdout, "Error in line %d: Label without command or directive.\n", state->line_number);
                state->error_found = TRUE;
                state->line_number++;
                continue;
            }
        }
        else if (strlen(first_word) > NUMBER_ZERO && first_word[strlen(first_word) - NUMBER_ONE] == ':') {
            if (strlen(first_word) - NUMBER_ONE > MAX_LABEL_LENGTH) {
                fprintf(stdout, "Error in line %d: The label name '%s' is too long.\n", state->line_number, first_word);
            }
            else {
                strncpy(clean_label, first_word, strlen(first_word) - NUMBER_ONE);
                clean_label[strlen(first_word) - NUMBER_ONE] = '\0';

                if (is_forbidden_word(clean_label)) {
                    fprintf(stdout, "Error in line %d: '%s' is a reserved word and cannot be used as a label name.\n", state->line_number, clean_label);
                }
                else {
                    fprintf(stdout, "Error in line %d: Invalid label name '%s'.\n", state->line_number, first_word);
                }
            }
            state->error_found = TRUE;
            state->line_number++;
            continue;
        }

        /* checks if the word is ".data" or ".string". and update dc by using functions from parser file. */
        if (strcmp(first_word, ".data") == NUMBER_ZERO || strcmp(first_word, ".string") == NUMBER_ZERO) {
            if (label_found) {
                /* if we don't have this label yet on the list, we add it with data type.
                 * Otherwise, we have this label name already, and we need to print an error alert for the user. */
                if (!add_symbol(&state->symbol_head, label, state->dc, FALSE, TRUE, FALSE)) {
                    fprintf(stdout, "Error in line %d: The label '%s' redefined.\n", state->line_number, label);
                    state->error_found = TRUE;
                }
            }
            if (strcmp(first_word, ".data") == NUMBER_ZERO) {
                /* We check if the words after .data are legal data with the function extract_data in parser. */
                if (!extract_data(&line_ptr, state)) {
                    state->error_found = TRUE;
                }
            }
            else {
                /* We check if the words after .string are legal string with the function extract_string_data in parser. */
                if (!extract_string_data(&line_ptr, state)) {
                    state->error_found = TRUE;
                }
            }
        }
        /*
         * We need to check if there is a label after .entry or .extern directive,
         * that the label is legal and in the legal length for label.
         * because the label is without ":" in .entry and .extern,  we need to use the function is_legal_name(label)).
         * and also, that there isn't an extra text after the directive.
         */
        else if (strcmp(first_word, ".extern") == NUMBER_ZERO || strcmp(first_word, ".entry") == NUMBER_ZERO) {
            /* if there is a label before .extern or .entry,
             * we print a warning to the user according to the project instructions. */
            if (label_found) {
                printf("Warning in line %d: Label before %s directive is meaningless and ignored.\n", state->line_number, first_word);
            }
            extract_word(&line_ptr, label);

            if (label[NUMBER_ZERO] == '\0') {
                fprintf(stdout, "Error in line %d: Missing label after %s directive.\n", state->line_number, first_word);
                state->error_found = TRUE;
                state->line_number++;
                continue;
            }

            if (strlen(label) > MAX_LABEL_LENGTH) {
                fprintf(stdout, "Error in line %d: Label '%s' is too long.\n", state->line_number, label);
                state->error_found = TRUE;
            }
            else if (!is_legal_name(label)) {
                fprintf(stdout, "Error in line %d: Invalid label name '%s' for directive .\n", state->line_number, label);
                state->error_found = TRUE;
            }

            /* Check for extraneous text after the label */
            skip_whitespaces(&line_ptr);
            if (*line_ptr != '\0' && *line_ptr != '\n') {
                fprintf(stdout, "Error in line %d: Extra text after %s directive.\n", state->line_number, first_word);
                state->error_found = TRUE;
            }


            /* we check on the label in .entry in the second_pass. */
            if (strcmp(first_word, ".entry") == NUMBER_ZERO) {
                state->line_number++;
                continue;
            }


            /* we get here if we have an .extern label:
             * we also need to check if the label for .extern is already exists in the symbol table.
             * if it isn't, and we didn't find errors in the label line we add it.
             * if it is and its type isn't in .extern, we need to print an error alert to the user.
            */
            if (!state->error_found) {
                label_existing = get_symbol(state->symbol_head, label);
                if (label_existing && !label_existing->is_extern) {
                    fprintf(stdout, "Error in line %d: The symbol '%s' defined locally, cannot be .extern\n", state->line_number, label);
                    state->error_found = TRUE;
                }
                else if (!label_existing) {
                    add_symbol(&state->symbol_head, label, NUMBER_ZERO, FALSE, FALSE, TRUE);
                }
            }
        }

        /* we found a label in instruction line, we need to add it to the symbol table with code type.
        * if it already exists, we print an error alert to the user. */
        else {
            if (label_found) {
                if (!add_symbol(&state->symbol_head, label, state->ic, TRUE, FALSE, FALSE)) {
                    fprintf(stdout, "Error in line %d: The label '%s' redefined.\n", state->line_number, label);
                    state->error_found = TRUE;
                }
            }

            /* we chack the command line :
             * the command name- if it isn't in the command list, we print an error alert to the user.
             * the excepted operands-  if we don't get the numbers of the operands we excepted, we print an error alert to the user.
             * the valid modes to the addressing mode for source and dest operands for the command. if we don't get them, we print an error alert to the user.
             * */
            cmd = get_command(first_word);
            if (!cmd) {
                fprintf(stdout, "Error in line %d: Unknown command '%s'\n", state->line_number, first_word);
                state->error_found = TRUE;
            }
            else {
                for(index =NUMBER_ZERO; index<MAX_LINE_LENGTH; index++){
                    src[index]='\0';
                    dst[index]='\0';
                }

                if (!extract_operands(&line_ptr, src, dst, cmd->expected_ops, state->line_number)) {
                    state->error_found = TRUE;
                }
                else {
                    /* We need to check the addressing mode for the source operand and dest operand in the command.
                     * if this addressing mode isn't one of the modes for this command, we print an error alert to the user.
                     * if the operand has immediate mode, we need to check if we have a legal number after '#'.
                     * also, the number has to be in range from -2048 until 2047 because we have only 12 bits.
                     * if it isn't we print an error alert to the user.
                     * */
                    if (cmd->expected_ops == NUMBER_TWO) {
                        src_addressing_mode = get_addressing_mode(src);
                        dst_addressing_mode = get_addressing_mode(dst);

                        if (!is_valid_addressing_mode(cmd->valid_src_operand_types, src_addressing_mode)) {
                            fprintf(stdout, "Error in line %d: Invalid source addressing mode for this command.\n", state->line_number);
                            state->error_found = TRUE;
                        }
                        if (!is_valid_addressing_mode(cmd->valid_dest_operand_types, dst_addressing_mode)) {
                            fprintf(stdout, "Error in line %d: Invalid destination addressing mode for this command.\n", state->line_number);
                            state->error_found = TRUE;
                        }
                        if (src_addressing_mode == IMMEDIATE_MODE) {
                            /* we send the src str from index 1 because we need to remove '#'. */
                            if (!is_legal_number(src + NUMBER_ONE)) {
                                fprintf(stdout, "Error in line %d: '%s' is an illegal number\n", state->line_number, src);
                                state->error_found = TRUE;
                            }
                            else if (!is_number_range(atoi(src + NUMBER_ONE))) {
                                fprintf(stdout, "Error in line %d: The number '%s' is out of the 12 bit range for numbers\n", state->line_number, src);
                                state->error_found = TRUE;
                            }
                        }

                    }
                    else if (cmd->expected_ops == NUMBER_ONE) {
                        dst_addressing_mode = get_addressing_mode(dst);
                        if (!is_valid_addressing_mode(cmd->valid_dest_operand_types, dst_addressing_mode)) {
                            fprintf(stdout, "Error in line %d: Invalid destination addressing mode for this command.\n", state->line_number);
                            state->error_found = TRUE;
                        }
                    }

                    if (cmd->expected_ops >= NUMBER_ONE) {
                        if (dst_addressing_mode == IMMEDIATE_MODE) {
                            if (!is_legal_number(dst + NUMBER_ONE)) {
                                fprintf(stdout, "Error in line %d: '%s' is an illegal number\n", state->line_number, dst);
                                state->error_found = TRUE;
                            }
                            else if (!is_number_range(atoi(dst + NUMBER_ONE))) {
                                fprintf(stdout, "Error in line %d: The number '%s' out of 12-bit range\n", state->line_number, dst);
                                state->error_found = TRUE;
                            }
                        }
                    }

                    /* we calculate the count of words we need for this command with L. */
                    L = NUMBER_ONE + cmd->expected_ops;

                    /* if the ic is become bigger than MEMORY_SIZE in the file,
                     * we print an error that the program is too large.   */
                    if ((state->ic + L - IC_START) >= MEMORY_SIZE) {
                        fprintf(stdout, "Error: The program is too large for the memory size %d.\n", MEMORY_SIZE);
                        state->error_found = TRUE;
                        return FALSE;
                    }

                    if (state->error_found) {
                        state->line_number++;
                        continue;
                    }

                    /* we encode the first word in the command line (12 Bits) and the command type (2 bits).
                     * for each command we have at least one word.
                     * for operand's commands that haven't an addressing mode, we put the bit 0. */
                    state->code_image[state->ic - IC_START].value =
                            (cmd->opcode << OPCODE_PLACE) |
                            (cmd->funct << FUNCT_PLACE) |
                            ((src_addressing_mode != MISSING_OPERAND ? src_addressing_mode : NUMBER_ZERO) << SRC_ADDRESSING_MODE_PLACE) |
                            (dst_addressing_mode != MISSING_OPERAND ? dst_addressing_mode : NUMBER_ZERO);
                    state->code_image[state->ic - IC_START].are = ARE_ABSOLUTE;

                    /* we use this function in order to encode more words for operand's command (if it has more words). */
                    build_operands_words(state, src, dst, src_addressing_mode, dst_addressing_mode, cmd->expected_ops);

                    /* updating ic counter and adding L. */
                    state->ic += L;
                }
            }
        }
        state->line_number++;
    }

    /* We need to move dc to the start after the end of ic. */
    current_symbol = state->symbol_head;
    while (current_symbol) {
        if (current_symbol->is_data) {
            current_symbol->label_address += state->ic;
        }
        current_symbol = current_symbol->next;
    }

    /* we chack the final memory file after summing IC and DC */
    if ((state->ic - IC_START) + state->dc >= MEMORY_SIZE) {
        fprintf(stdout, "Error: The program is too large for the memory size %d.\n", MEMORY_SIZE);
        state->error_found = TRUE;
        return FALSE;
    }
    return !state->error_found;
}
