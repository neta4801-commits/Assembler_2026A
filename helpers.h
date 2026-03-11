/* Helpers header file */

#define UTILS_H

#include <stddef.h>
#include "globals.h"


void *check_malloc(size_t size);
/*checks if memory allocation was successful. */


void skip_spaces(char **str);
/*skips spaces and '\t'. */


boolean is_empty_or_comment(char *line);
/*returns true if line is empty, or is comment. */


char *create_file_name(const char *original_name, char *extension);
/*combine file name with extension and create a file with full name */


#endif
