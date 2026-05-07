/*
 * pre_assembler.c file, this file will be responsible for receiving the input file,
 * replacing all the macros that were written by the user, print an error macro alerts to the user
 * and creating a new output file which will contain the macro commands with the source code.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pre_assembler.h"
#include "../tables/macro_table.h"
#include "../globals/parser.h"
#include "../globals/helpers.h"

/* We pass the function a pointer to the file, the original name (no extension),
 * and a pointer to struct from "AssemblerState" type.
 * The purpose of this function is to read the original file context (.as),
 * and find macros, in order to create a new output file (.am), in the
 * output file, the macros will be switched and written as commands.
 * In addition, we are adding labels to our label list,
 * and check if there are conflicts between label names and marco names.
 */
boolean pre_assemble(FILE *source_file, char *original_name, AssemblerState *state) {

     /* +2 for newline and null terminator */
    char line[MAX_LINE_LENGTH + NUMBER_TWO];
    char first_word[MAX_LINE_LENGTH + NUMBER_TWO];
    char *line_ptr;
    char *am_file_name;
    FILE *am_file;
    char *temp_ptr;
    char second_word[MAX_LINE_LENGTH];
    char macro_name[MAX_LABEL_LENGTH + NUMBER_TWO];
    macro_line_node *curr_line;

    boolean is_inside_macro = FALSE;
    macro_ptr macro_head = NULL;
    macro_ptr current_macro = NULL;
    macro_ptr found_macro = NULL;
    int current_char, index;

    label_ptr label_head = NULL;
    label_ptr current_label = NULL;
    label_ptr new_label = NULL; /* To create new label nodes */
    label_ptr next_label = NULL; /* To free label nodes at the end */
    char temp_label[MAX_LABEL_LENGTH + NUMBER_ONE]; /* To check duplicate labels  */
    boolean is_label_conflict; /* To check conflicts with label names like duplicates */

    state->line_number = NUMBER_ONE;
    state->error_found = FALSE;

    /* create the am file name and open it for writing */
    am_file_name = create_file_name(original_name, ".am");
    am_file = fopen(am_file_name, "w");
    if (am_file == NULL) {
        fprintf(stderr, "Error: Cannot create output file %s\n", am_file_name);
        free(am_file_name);
        return FALSE;
    }

    while (fgets(line, sizeof(line), source_file) != NULL) {
        /* Check for line length (the maximum allowed is 80 characters) and avoid false error at the end of file */
        if (strchr(line, '\n') == NULL && !feof(source_file)) {
            fprintf(stdout, "Error in line %d: Line exceeds max length.\n", state->line_number);
            state->error_found = TRUE;
            /* Skip the rest of the line if it exceeds the maximum length until newline or EOF is found */
            while ((current_char = fgetc(source_file)) != '\n' && current_char != EOF);
            state->line_number++;
            continue;
        }

        line_ptr = line;

        /* Check if the line is empty or a comment */
        if (is_empty_or_comment(line_ptr)) {
            if (is_inside_macro) {
                if (current_macro != NULL) {
                    add_macro_line(current_macro, line);
                }
            }
            /* Skip the line if it is empty or a comment */
            state->line_number++;
            continue;
        }

        extract_word(&line_ptr, first_word);

        /* check for mcroend */
        if (is_inside_macro) {
            if (strcmp(first_word, "mcroend") == NUMBER_ZERO) {
                skip_whitespaces(&line_ptr);
                /*check for extraneous text after mcroend */
                if (*line_ptr != '\0' && *line_ptr != '\n') {
                    fprintf(stdout, "Error in line %d: Extraneous text after 'mcroend'.\n", state->line_number);
                    state->error_found = TRUE;
                }
                is_inside_macro = FALSE;
                current_macro = NULL;

            }
            /* no mcroend found, we are still inside a macro, save the line so we can use it later*/
            else {
                if (current_macro != NULL) {
                    add_macro_line(current_macro, line);
                }
            }
        }
        /* not inside macro, check for macro definition or regular line of code */
        else {
            temp_ptr = line_ptr;
            for (index = NUMBER_ZERO; index < sizeof(second_word); index++) {
                second_word[index] = '\0';
            }

            extract_word(&temp_ptr, second_word);

            /*indetify unnecessary text if the second word is "macro"*/
            if (strcmp(second_word, "mcro") == NUMBER_ZERO) {
                fprintf(stdout, "Error in line %d: Extraneous text or label ('%s') before 'mcro'.\n", state->line_number, first_word);
                state->error_found = TRUE;
                is_inside_macro = TRUE; /*until we find a valid macroend, keep the macro flag on, skipping uneccessary lines*/
                current_macro = NULL;
                state->line_number++;
                continue;
            }

            /*check for macro definition if it starts with mcro and clear the macro name with zeros*/
            if (strcmp(first_word, "mcro") == NUMBER_ZERO) {
                memset(macro_name, NUMBER_ZERO, sizeof(macro_name));
                is_label_conflict = FALSE;
                /*extract the macro name from the current line */
                extract_word(&line_ptr, macro_name);

              /* check for label conflicts */
              current_label = label_head;
              while (current_label != NULL) {
                  if (strcmp(current_label->label_name, macro_name) == NUMBER_ZERO) {
                      is_label_conflict = TRUE;
                      break;
                  }
                  current_label = current_label->next;
                }
                


                /* validate the macro name */
                if (macro_name[NUMBER_ZERO] == '\0') {
                    fprintf(stdout, "Error in line %d: Missing macro name.\n", state->line_number);
                    state->error_found = TRUE;
                    is_inside_macro = TRUE;
                    current_macro = NULL;
                }

                /* check if the macro name is a reserved word */
                else if (is_forbidden_word(macro_name)) {
                    fprintf(stdout, "Error in line %d: macro name '%s' is reserved.\n", state->line_number, macro_name);
                    state->error_found = TRUE;
                    is_inside_macro = TRUE;
                    current_macro = NULL;
                }

                /* check if the macro name is already defined as a macro */
                else if (get_macro(macro_head, macro_name) != NULL) {
                    fprintf(stdout, "Error in line %d: Macro '%s' already defined.\n", state->line_number, macro_name);
                    state->error_found = TRUE;
                    is_inside_macro = TRUE;
                    current_macro = NULL;
                 }

                 /* check if the macro name is already defined as a label */
                 else if (is_label_conflict) {
                    fprintf(stdout, "Error in line %d: Macro name '%s' cannot be identical to an already defined label.\n", state->line_number, macro_name);
                    state->error_found = TRUE;
                    is_inside_macro = TRUE;
                    current_macro = NULL;
                 }

                    /*macro is valid*/
                    else {
                    /* check for extraneous text after the macro name */
                    skip_whitespaces(&line_ptr);
                    if (*line_ptr != '\0' && *line_ptr != '\n') {
                        fprintf(stdout, "Error in line %d: Extraneous text after macro name.\n", state->line_number);
                        state->error_found = TRUE;
                    }
                    current_macro = add_macro(&macro_head, macro_name);
                    is_inside_macro = TRUE;
                }
            }

            /* check for mcroend without a matching macro */
            else if (strcmp(first_word, "mcroend") == NUMBER_ZERO) {
                fprintf(stdout, "Error in line %d: 'mcroend' encountered without matching 'mcro'.\n", state->line_number);
                state->error_found = TRUE;
            }

            /* not a macro */
            else {
                /* Check if the first word is a label (ends with ':') and add it to the labels linked list */
                if (strlen(first_word) > NUMBER_ZERO && first_word[strlen(first_word) - NUMBER_ONE] == ':') {
                    strncpy(temp_label, first_word, strlen(first_word) - NUMBER_ONE);
                    temp_label[strlen(first_word) - NUMBER_ONE] = '\0';

                    /* Validate the label name */
                    if (get_macro(macro_head, temp_label) != NULL) {
                        fprintf(stdout, "Error in line %d: Label name '%s' cannot be identical to a defined macro name.\n", state->line_number, temp_label);
                        state->error_found = TRUE;
                    }
                    /* Memory allocation for the new label */
                    else {
                        new_label = check_malloc(sizeof(label_node));
                        strcpy(new_label->label_name, temp_label);
                        new_label->next = label_head;
                        label_head = new_label;
                    }

                }

                /* Check if the first word is a macro name and expand it if found, otherwise write the line as is to the .am file. */
                if ((found_macro = get_macro(macro_head, first_word)) != NULL) {
                    curr_line = found_macro->lines_head;
                    while (curr_line != NULL) {
                        fputs(curr_line->line, am_file);
                        curr_line = curr_line->next;
                    }
                }
                /* Regular code line and not a macro, so write it to the .am file. */
                else {
                    fputs(line, am_file);
                }
            }
        }
        state->line_number++;
    }

    fclose(am_file);
    free(am_file_name);
    free_macro_table(macro_head);

   /* Free the labels linked list */
    current_label = label_head;
    while (current_label != NULL) {
        next_label = current_label->next;
        free(current_label);
        current_label = next_label;
    }
    
    return !state->error_found;
}


