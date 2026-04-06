/* Helpers header file */

#ifndef HELPERS_H
#define HELPERS_H

#include <stddef.h>
#include "constant.h"

/*checks if memory allocation was successful. */
void *check_malloc(size_t size);

/*skips spaces and '\t'. */
void skip_whitespaces(char **str);

/*returns true if line is empty, or is comment. */
boolean is_empty_or_comment(char *line);


/* combines file name with extension to create a full name for a file.
 * copies the original name of the file to file_name with \0 then concatenate the extension, once it finds \0
 * ('helpers' + '.h' = 'helpers.h') */
char *create_file_name(char *original_name, char *extension);


/* Functions for symbol table */

/* This function gets a pointer to the head of the symbol list, and symbol details (name,address and the label type-code,data or extern).
 * The function adds the new symbol to the symbol list and returns true if it success, otherwise false. */
boolean add_symbol(symbol_ptr *head, char *label_name, int label_address, int is_code, int is_data, int is_extern);


/* This function gets a pointer to the head of the symbol list and a symbol name.
 * The function searches symbol on the list by his name.
 * Returns a pointer to this symbol with his details. */
symbol_ptr get_symbol(symbol_ptr head, char *label_name);

/* This function gets a pointer to the head of the symbol list and safely free all allocated memory */
void free_symbols(symbol_ptr head);

#endif

