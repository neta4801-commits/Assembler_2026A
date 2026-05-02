/*
 * קובץ: pre_assembler.c
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pre_assembler.h"
#include "src/macros/macro_table.h"
#include "src/macros/parser.h"
#include "src/globals/helpers.h"
#include "src/globals/constant.h"


boolean pre_assemble(FILE *source_file, const char *base_file_name, AssemblerContext *context) {
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
    int c;

    context->line_number = NUMBER_ONE;
    context->error_found = FALSE;

    label_ptr label_head = NULL;
    label_ptr current_label = NULL;
    label_ptr new_label = NULL; /* To create new label nodes */
    label_ptr next_label = NULL; /* To free label nodes at the end */
    char temp_label[MAX_LABEL_LENGTH + NUMBER_ONE]; /* To check duplicate labels  */
    boolean is_label_conflict; /* To check conflicts with label names like duplicates */

    
    am_file_name = create_file_name(base_file_name, ".am");
    am_file = fopen(am_file_name, "w");
    if (am_file == NULL) {
        fprintf(stderr, "Error: Cannot create output file %s\n", am_file_name);
        free(am_file_name);
        return FALSE;
    }

    while (fgets(line, sizeof(line), source_file) != NULL) {
        /* Check for line length (the maximum allowed is 80 characters) and avoid false error at the end of file */
        if (strchr(line, '\n') == NULL && !feof(source_file)) {
            fprintf(stderr, "Error at line %d: Line exceeds max length.\n", context->line_number);
            context->error_found = TRUE;
            /* Skip the rest of the line if it exceeds the maximum length until newline or EOF is found */
            while ((c = fgetc(source_file)) != '\n' && c != EOF);
            context->line_number++;
            continue;
        }

        line_ptr = line;

        /* Check if the line is empty or a comment */
        if (is_empty_or_comment(line_ptr)) {
            //  לפי הההערות לא נדרש כי זו שורת הערה- לוודא כי יש מצב אפשר למחוק את תהנאי ולא להוסיף את השורה למאקרו.
            if (is_inside_macro) {
                if (current_macro != NULL) add_macro_line(current_macro, line);
            }
            /* Skip the line if it is empty or a comment */
            context->line_number++;
            continue;
        }
        extract_word(&line_ptr, first_word);

        /* check for macroend */
        if (is_inside_macro) {
            if (strcmp(first_word, "macroend") == NUMBER_ZERO) {
                skip_whitespaces(&line_ptr);
                /*check for extraneous text after macroend */
                if (*line_ptr != '\0' && *line_ptr != '\n') {
                    fprintf(stderr, "Error at line %d: Extraneous text after 'mcroend'.\n", context->line_number);
                    context->error_found = TRUE;
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
            memset(second_word, NUMBER_ZERO, sizeof(second_word));
            /*
            //אפשר לכתוב במקטם memset את הלולאה הזו לאתחול המערך הזה.
             //רק צריך לשים לב שהint i מחוץ לבלוק ובתחילת התוכנית איפשה שמכריזים על משתנים.
             * int i;
               for (i = 0; i < sizeof(second_word); i++) {
                    second_word[i] = '\0';
}
             */
            // הוצאת המילה השנייה- אחרי שתיגמר ישארו '/0'
            extract_word(&temp_ptr, second_word);

            /*indetify unnecessary text if the second word is "macro"*/
            if (strcmp(second_word, "mcro") == NUMBER_ZERO) {
                fprintf(stderr, "Error at line %d: Extraneous text or label ('%s') before 'mcro'.\n", context->line_number, first_word);
                context->error_found = TRUE;
                is_inside_macro = TRUE; /*until we find a valid macroend, keep the macro flag on, skipping uneccessary lines*/
                current_macro = NULL;
                context->line_number++;
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
                    fprintf(stderr, "Error at line %d: Missing macro name.\n", context->line_number);
                    context->error_found = TRUE;
                    is_inside_macro = TRUE;
                    current_macro = NULL; 
                }

                /* check if the macro name is a reserved word */
                else if (is_reserved_word(macro_name)) {
                    fprintf(stderr, "Error at line %d: macro name '%s' is reserved.\n", context->line_number, macro_name);
                    context->error_found = TRUE;
                    is_inside_macro = TRUE;
                    current_macro = NULL;  
                }

                /* check if the macro name is already defined as a macro */
                else if (get_macro(macro_head, macro_name) != NULL) {
                    fprintf(stderr, "Error at line %d: Macro '%s' already defined.\n", context->line_number, macro_name);
                    context->error_found = TRUE;
                    is_inside_macro = TRUE;
                    current_macro = NULL; 
                 }

                 /* check if the macro name is already defined as a label */
                 else if (is_label_conflict) {
                    fprintf(stderr, "Error at line %d: Macro name '%s' cannot be identical to an already defined label.\n", context->line_number, macro_name);
                    context->error_found = TRUE;
                    is_inside_macro = TRUE;
                    current_macro = NULL; /* התאוששות */ }

                    /*macro is valid*/
                    else {
                    /* check for extraneous text after the macro name */
                    skip_whitespaces(&line_ptr);
                    if (*line_ptr != '\0' && *line_ptr != '\n') {
                        fprintf(stderr, "Error at line %d: Extraneous text after macro name.\n", context->line_number);
                        context->error_found = TRUE;
                    }
                    current_macro = add_macro(&macro_head, macro_name);
                    is_inside_macro = TRUE;
                }
            }

            /* check for macroend without a matching macro */
            else if (strcmp(first_word, "mcroend") == NUMBER_ZERO) {
                fprintf(stderr, "Error at line %d: 'mcroend' encountered without matching 'mcro'.\n", context->line_number);
                context->error_found = TRUE;
            }

            /* not a macro */
            else {
                /* Check if the first word is a label (ends with ':') and add it to the labels linked list */
                if (strlen(first_word) > NUMBER_ZERO && first_word[strlen(first_word) - NUMBER_ONE] == ':') {
                    strncpy(temp_label, first_word, strlen(first_word) - NUMBER_ONE);
                    temp_label[strlen(first_word) - NUMBER_ONE] = '\0';

                    /* Validate the label name */
                    if (get_macro(macro_head, temp_label) != NULL) {
                        fprintf(stderr, "Error at line %d: Label name '%s' cannot be identical to a defined macro name.\n", context->line_number, temp_label);
                        context->error_found = TRUE;
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
        context->line_number++;
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
    
    return !context->error_found;
}

