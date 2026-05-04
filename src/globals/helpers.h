/* Helpers header file */

#ifndef HELPERS_H
#define HELPERS_H

#include <stddef.h>
#include "constant.h"

/* checks if memory allocation was successful. */
void *check_malloc(size_t size);

/* skips spaces and '\t'. */
void skip_whitespaces(char **str);

/* returns true if line is empty, or is comment. */
boolean is_empty_or_comment(char *line);


/* combines file name with extension to create a full name for a file.
 * copies the original name of the file to file_name with \0 then concatenate the extension, once it finds \0
 * ('helpers' + '.h' = 'helpers.h') */
char *create_file_name(char *original_name, char *extension);

/* --- SECOND PASS HELPERS --- */
/* Instruction and validation helpers */
void initialize_instruction_info(instruction_line_info *info);

boolean extract_word_after_optional_label(char **line_ptr, char *first_word, int line_number);
boolean is_second_pass_ignored_directive(const char *word);
boolean validate_immediate_operand(const char *operand, int line_number, const char *operand_role);
boolean process_entry_directive(char *line_ptr, AssemblerState *state);
boolean parse_instruction_line(char *first_word, char *line_ptr, int line_number, instruction_line_info *info);

/* Extern list management */
boolean add_extern_usage(extern_ptr *extern_head, const char *symbol_name, int usage_address, int line_number);
void free_extern_list(extern_ptr head);

#endif

