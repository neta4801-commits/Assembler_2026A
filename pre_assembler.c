/*
 * pre_assembler.c file, this file will be responsible for receiving the input file,
 * replacing all the macros that were written by the user, and creating
 * a new output file which will contain the macro commands with the source code.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pre_assembler.h"
#include "macro_table.h"
#include "parser.h"
#include "helpers.h"

boolean pre_assemble(FILE *source_file char *base_file_name, AssemblerContext *context) {
    
  char line[MAX_LINE_LENGTH + 2];
  char first_word[MAX_LINE_LENGTH + 2];
  char *line_ptr;
  char *am_file_name;
  FILE *am_file;
    
  boolean is_inside_macro = FALSE;.
  macro_ptr macro_head = NULL; 
  macro_ptr current_macro = NULL;
  macro_ptr found_macro = NULL;
  int c;
  
/* Start from line number 1 and set errors found to false.*/
  context->line_number = 1;
  context->error_found = FALSE;

/* Create a new output file (.am), then open it. */
  am_file_name = create_file_name(base_file_name, ".am");
  am_file = fopen(am_file_name, "w");
  if (am_file == NULL) {
      fprintf(stderr, "Error: Cannot create output file %s\n", am_file_name); /* Error text for not being able to open file*/
      free(am_file_name);
      return FALSE;
  }

/* Reads each line in the file */
  while (fgets(line, sizeof(line), source_file) != NULL) {
  
      /* Check if the line is too long*/
      if (strchr(line, '\n') == NULL && !feof(source_file)) {
          fprintf(stderr, "Error at line %d: Line exceeds max length.\n", context->line_number); /* Error text for exceeding max length*/
          context->error_found = TRUE;
          
          /* Clean the rest from buffer*/
          while ((c = fgetc(source_file)) != '\n' && c != EOF);
            context->line_number++;
            continue;
        }
  line_ptr = line;
  
  /* Using helper function to handle empty lines and comments*/
  if (is_empty_or_comment(line_ptr)) {
    if (is_inside_macro) {
        add_macro_line(current_macro, line);
        } else {
            fputs(line, am_file);
          }
          context->line_number++;
          continue;
        }
  
  /* first word of the line*/
  extract_word(&line_ptr, first_word);

  /* if currently saving a macro stop when you find 'mcroend', then check for extra text after 'mcroend'*/
  if (is_inside_macro) {
    if (strcmp(first_word, "mcroend") == 0) {
        skip_whitespaces(&line_ptr);
        if (*line_ptr != '\0' && *line_ptr != '\n') {
          fprintf(stderr, "Error at line %d: Extra text after 'mcroend'.\n", context->line_number); /* Error for extra text */
          context->error_found = TRUE;
          }
          is_inside_macro = FALSE;
          current_macro = NULL;
      } else {
          add_macro_line(current_macro, line);
      }
      
      /* Else - if we are not inside a macro (regular code) */
    } else {
        /* If first word starts with 'mcro' start a new macro*/
        if (strcmp(first_word, "mcro") == 0) {
          char macro_name[MAX_LABEL_LENGTH + 2] = {0};
          extract_word(&line_ptr, macro_name);
          
          /* Checks for errors in the macro name */
          if (macro_name[0] == '\0') {
            fprintf(stderr, "Error at line %d: Missing macro name.\n", context->line_number);
            context->error_found = TRUE;
          } else if (is_reserved_word(macro_name)) {
              fprintf(stderr, "Error at line %d: Macro name '%s' is reserved.\n", context->line_number, macro_name);
              context->error_found = TRUE;
          } else if (get_macro(macro_head, macro_name) != NULL) {
              fprintf(stderr, "Error at line %d: Macro '%s' already defined.\n", context->line_number, macro_name);
               context->error_found = TRUE;
          } else {
              /* Name is good, and needs saving */
              skip_whitespaces(&line_ptr);
              if (*line_ptr != '\0' && *line_ptr != '\n') {
                fprintf(stderr, "Error at line %d: Extraneous text.\n", context->line_number);
                context->error_found = TRUE;
              }
              current_macro = add_macro(&macro_head, macro_name);
              is_inside_macro = TRUE;
          }
      } 
      /* If we found a saved macro, write its lines to the file */
      else if ((found_macro = get_macro(macro_head, first_word)) != NULL) {
          macro_line_node *curr_line = found_macro->lines_head;
          while (curr_line != NULL) {
            fputs(curr_line->line, am_file);
            curr_line = curr_line->next;
            }
          }
          /* Not a macro, write the regular code to file */
          else {
            fputs(line, am_file);
          }
        }
        context->line_number++;
      }
    /* Close the file, and free memory*/
    fclose(am_file);
    free(am_file_name);
    free_macro_table(macro_head); 
    
    /* Return true if there were no errors*/
    return !context->error_found;
}
