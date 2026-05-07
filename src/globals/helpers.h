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

#endif


