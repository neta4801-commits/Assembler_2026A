
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "first_pass.h"
#include "parser.h"
#include "src/globals/helpers.h"
#include "assembler_tables.h"

#include <stdio.h>
#include <string.h>
#include "first_pass.h"
#include "parser.h"
#include "utils.h"
#include "tables.h"

boolean first_pass(FILE *am_file, AssemblerState *state) {
    char line[MAX_LINE_LENGTH + NUMBER_TWO];
    char first_word[MAX_LINE_LENGTH];
    char label[MAX_LABEL_LENGTH];
    char src[MAX_LINE_LENGTH];
    char dst[MAX_LINE_LENGTH];
    char *line_ptr;
    symbol_ptr curr;

    state->ic = INITIAL_IC;
    state->dc = NUMBER_ZERO;
    state->line_number = NUMBER_ONE;

    while (fgets(line, sizeof(line), am_file)) {
        boolean symbol_flag = FALSE;
        command_info *cmd;
        int src_m = -1, dst_m = -1, L = 1;

        line_ptr = line;
        if (is_empty_or_comment(line_ptr)) {
            state->line_number++;
            continue;
        }

        extract_word(&line_ptr, first_word);

        /* טיפול בתווית */
        if (strlen(first_word) > 0 && first_word[strlen(first_word) - 1] == ':') {
            if (!is_label_definition(first_word)) {
                fprintf(stderr, "Error line %d: Invalid label syntax '%s'\n", state->line_number, first_word);
                state->error_found = TRUE;
                state->line_number++;
                continue;
            }
            symbol_flag = TRUE;
            strncpy(label, first_word, strlen(first_word) - 1);
            label[strlen(first_word) - 1] = '\0';
            extract_word(&line_ptr, first_word);
        }

        /* הנחיות נתונים */
        if (strcmp(first_word, ".data") == 0) {
            if (symbol_flag) add_symbol(&state->symbol_head, label, state->dc, 0, 1, 0);
            extract_data(&line_ptr, state);
        } else if (strcmp(first_word, ".string") == 0) {
            if (symbol_flag) add_symbol(&state->symbol_head, label, state->dc, 0, 1, 0);
            extract_string_data(&line_ptr, state);
        } else if (strcmp(first_word, ".extern") == 0) {
            extract_word(&line_ptr, label);
            if (!get_symbol(state->symbol_head, label))
                add_symbol(&state->symbol_head, label, 0, 0, 0, 1);
        } else if (strcmp(first_word, ".entry") == 0) {
            /* מטופל במעבר שני */
        } else {
            /* פקודות קוד */
            cmd = get_command(first_word);
            if (!cmd) {
                fprintf(stderr, "Error line %d: Unknown command '%s'\n", state->line_number, first_word);
                state->error_found = TRUE;
            } else {
                if (symbol_flag) {
                    if (!add_symbol(&state->symbol_head, label, state->ic, 1, 0, 0)) {
                        fprintf(stderr, "Error line %d: Label '%s' redefined\n", state->line_number, label);
                        state->error_found = TRUE;
                    }
                }

                /* ניתוח אופרנדים באמצעות הפונקציה ב-parser.c */
                memset(src, 0, sizeof(src));
                memset(dst, 0, sizeof(dst));
                if (!extract_operands(&line_ptr, src, dst, cmd->expected_ops, state->line_number)) {
                    state->error_found = TRUE;
                } else {
                    if (cmd->expected_ops >= 1) dst_m = get_addressing_mode(dst);
                    if (cmd->expected_ops == 2) src_m = get_addressing_mode(src);

                    /* חישוב אורך פקודה: מילה אחת לפקודה + מילה לכל אופרנד */
                    L = 1 + cmd->expected_ops;

                    /* קידוד ראשוני (Opcode) */
                    state->code_image[state->ic - INITIAL_IC].value = (cmd->opcode << 8) | (cmd->funct << 4) |
                                                                          ((src_m != -1 ? src_m : 0) << 2) | (dst_m != -1 ? dst_m : 0);
                    state->code_image[state->ic - INITIAL_IC].are = ARE_ABSOLUTE;

                    /* קידוד ערכים מיידיים/אוגרים אם ידועים */
                    if (src_m == 0 || src_m == 3) {
                        int val = (src_m == 0) ? atoi(src + 1) : (1 << (src[1] - '0'));
                        state->code_image[state->ic - INITIAL_IC + 1].value = val & 0xFFF;
                        state->code_image[state->ic - INITIAL_IC + 1].are = ARE_ABSOLUTE;
                    }
                    if (dst_m == 0 || dst_m == 3) {
                        int val = (dst_m == 0) ? atoi(dst + 1) : (1 << (dst[1] - '0'));
                        int offset = (cmd->expected_ops == 2) ? 2 : 1;
                        state->code_image[state->ic - INITIAL_IC + offset].value = val & 0xFFF;
                        state->code_image[state->ic - INITIAL_IC + offset].are = ARE_ABSOLUTE;
                    }
                    state->ic += L; /* קידום פעם אחת בלבד! */
                }
            }
        }
        state->line_number++;
    }

    /* עדכון כתובות נתונים */
    curr = state->symbol_head;
    while (curr) {
        if (curr->is_data) curr->value += state->ic;
        curr = curr->next;
    }
    return !state->error_found;
}

